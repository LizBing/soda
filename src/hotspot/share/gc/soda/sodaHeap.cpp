/*
 * Copyright (c) 2024, Lei Zaakjyu. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Lei Zaakjyu(E-mail: leizaakjyu@163.com) if you need additional
 * information or have any questions.
 *
 */

#include "precompiled.hpp"

#include "gc/shared/gcThreadLocalData.hpp"
#include "gc/shared/gc_globals.hpp"
#include "gc/shared/genArguments.hpp"
#include "gc/shared/workerThread.hpp"
#include "gc/soda/sodaObjAllocator.hpp"
#include "gc/shared/gcArguments.hpp"
#include "gc/shared/gcLocker.inline.hpp"
#include "gc/shared/locationPrinter.inline.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaInitLogger.hpp"
#include "gc/soda/sodaMemoryPool.hpp"
#include "gc/soda/sodaThreadLocalData.hpp"
#include "gc/soda/sodaGlobalAllocator.hpp"
#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"
#include "logging/log.hpp"
#include "memory/allocation.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/metaspaceUtils.hpp"
#include "memory/resourceArea.hpp"
#include "memory/universe.hpp"
#include "runtime/atomic.hpp"
#include "runtime/globals.hpp"
#include "runtime/threadSMR.inline.hpp"

SodaHeap::SodaHeap() {
  // assert(sizeof(GCThreadLocalData) >= sizeof(SodaThreadLocalData), "Sanity.");
}

jint SodaHeap::initialize() {
  ParallelScavengeHeap::initialize();

  _line_size = DEFAULT_CACHE_LINE_SIZE * SodaCacheLinesPerBlockLine;
  _block_size = _line_size * SodaLinesPerHeapBlock;
  _capacity_in_blocks = align_down(MaxOldSize, _block_size) / _block_size;
  _heap_start = (intptr_t)_reserved.start();

  // Initialize facilities
  SodaObjAllocator::initialize();
  SodaFreeLineTable::initialize();
  SodaHeapBlocks::initialize();
  SodaGlobalAllocator::initialize();

  _conc_workers = new WorkerThreads("Soda Concurrent Worker", ConcGCThreads);

  // All done, print out the configuration
  SodaInitLogger::print();

  return JNI_OK;
}

void SodaHeap::post_initialize() {
  ParallelScavengeHeap::post_initialize();
}

void SodaHeap::initialize_serviceability() {
  ParallelScavengeHeap::initialize_serviceability();
}

SodaHeap* SodaHeap::heap() {
  return named_heap<SodaHeap>(CollectedHeap::Soda);
}

HeapWord* SodaHeap::mem_allocate(size_t size, bool *gc_overhead_limit_was_exceeded) {
  auto s = size * HeapWordSize;
  HeapWord* res = nullptr;
/*
  if (s >= min_humongous()) {
    res = alloc_humongous(s);
    if (res != nullptr) {
      *gc_overhead_limit_was_exceeded = false;
      return res;
    }
  }
*/
  return ParallelScavengeHeap::mem_allocate(size,
                                            gc_overhead_limit_was_exceeded);
}

HeapWord* SodaHeap::alloc_humongous(size_t size) {
  int num_blocks = align_up(size, block_size()) / block_size();

  auto hb = SodaGlobalAllocator::allocate(num_blocks);
  if (hb == nullptr) return nullptr;

  SodaBlockArchive::record_humongous(hb);

  auto start = hb->start();
  return (HeapWord*)start;
}

void SodaHeap::print_on(outputStream *st) const {
  st->print_cr("Soda Heap");
  ParallelScavengeHeap::print_on(st);
}

void SodaHeap::print_heap_info(size_t used) const {
  size_t reserved  = max_capacity();
  size_t committed = capacity();

  if (reserved != 0) {
    log_info(gc)("Heap: " SIZE_FORMAT "%s reserved, " SIZE_FORMAT "%s (%.2f%%) committed, "
                 SIZE_FORMAT "%s (%.2f%%) used",
            byte_size_in_proper_unit(reserved),  proper_unit_for_byte_size(reserved),
            byte_size_in_proper_unit(committed), proper_unit_for_byte_size(committed),
            committed * 100.0 / reserved,
            byte_size_in_proper_unit(used),      proper_unit_for_byte_size(used),
            used * 100.0 / reserved);
  } else {
    log_info(gc)("Heap: no reliable data");
  }
}

void SodaHeap::print_metaspace_info() const {
  MetaspaceCombinedStats stats = MetaspaceUtils::get_combined_statistics();
  size_t reserved  = stats.reserved();
  size_t committed = stats.committed();
  size_t used      = stats.used();

  if (reserved != 0) {
    log_info(gc, metaspace)("Metaspace: " SIZE_FORMAT "%s reserved, " SIZE_FORMAT "%s (%.2f%%) committed, "
                            SIZE_FORMAT "%s (%.2f%%) used",
            byte_size_in_proper_unit(reserved),  proper_unit_for_byte_size(reserved),
            byte_size_in_proper_unit(committed), proper_unit_for_byte_size(committed),
            committed * 100.0 / reserved,
            byte_size_in_proper_unit(used),      proper_unit_for_byte_size(used),
            used * 100.0 / reserved);
  } else {
    log_info(gc, metaspace)("Metaspace: no reliable data");
  }
}

void SodaHeap::stop() {
  SodaGlobalAllocator::clear_lfs();
  SodaBlockArchive::clear();
}

void SodaHeap::gc_threads_do(ThreadClosure *tc) const {
  ParallelScavengeHeap::gc_threads_do(tc);
  // _conc_workers->threads_do(tc);
}


void SodaHeap::ensure_parsability(bool retire_tlabs) {
  ParallelScavengeHeap::ensure_parsability(retire_tlabs);

  if (retire_tlabs) {
    for (JavaThreadIteratorWithHandle jtiwh; JavaThread *thread = jtiwh.next();) {
      SodaThreadLocalData::ra(thread)->retire();
    }

    SodaObjAllocator::retire_blocks();
  }
}
