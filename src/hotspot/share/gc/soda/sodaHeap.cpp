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
#include "gc/shared/gcArguments.hpp"
#include "gc/shared/gcLocker.inline.hpp"
#include "gc/shared/locationPrinter.inline.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaInitLogger.hpp"
#include "gc/soda/sodaMemoryPool.hpp"
#include "gc/soda/sodaThreadLocalData.hpp"
#include "gc/soda/sodaAllocator.hpp"
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

jint SodaHeap::initialize() {
  size_t align = HeapAlignment;

  _line_size = DEFAULT_CACHE_LINE_SIZE * SodaCacheLinesPerBlockLine;
  _block_size = _line_size * SodaLinesPerHeapBlock;

  size_t hsize = align_up(MaxHeapSize, align);
  _capacity_in_blocks  = hsize / _block_size;

  // Initialize backing storage
  ReservedHeapSpace heap_rs = Universe::reserve_heap(hsize, align);
  _virtual_space.initialize(heap_rs, hsize);

  MemRegion committed_region((HeapWord*)_virtual_space.low(),
                             (HeapWord*)_virtual_space.high());

  initialize_reserved_region(heap_rs);
  _heap_start = (intptr_t)_reserved.start();

  // Initialize facilities
  SodaFreeLineTable::initialize();
  SodaHeapBlocks::initialize();
  SodaGlobalAllocator::initialize();

  // Enable monitoring
  _monitoring_support = new SodaMonitoringSupport(this);

  // Install barrier set
  BarrierSet::set_barrier_set(new SodaBarrierSet());

  // All done, print out the configuration
  SodaInitLogger::print();

  return JNI_OK;
}

struct SodaScavengable : public BoolObjectClosure {
  bool do_object_b(oop obj) override { return true; }
};

static SodaScavengable _is_scavengable;

void SodaHeap::post_initialize() {
  CollectedHeap::post_initialize();

  ScavengableNMethods::initialize(&_is_scavengable);
}

void SodaHeap::initialize_serviceability() {
  _pool = new SodaMemoryPool(this);
  _memory_manager.add_pool(_pool);
}

GrowableArray<GCMemoryManager*> SodaHeap::memory_managers() {
  GrowableArray<GCMemoryManager*> memory_managers(1);
  memory_managers.append(&_memory_manager);
  return memory_managers;
}

GrowableArray<MemoryPool*> SodaHeap::memory_pools() {
  GrowableArray<MemoryPool*> memory_pools(1);
  memory_pools.append(_pool);
  return memory_pools;
}

SodaHeap* SodaHeap::heap() {
  return named_heap<SodaHeap>(CollectedHeap::Soda);
}

HeapWord* SodaHeap::mem_allocate(size_t size, bool *gc_overhead_limit_was_exceeded) {
  *gc_overhead_limit_was_exceeded = false;

  auto s = size * HeapWordSize;
  if (s < min_humongous())
    return (HeapWord*)SodaThreadLocalData::tlab(Thread::current())->
           allocate(s);

  int num_blocks = align_up(s, block_size()) / block_size();
  auto hb = SodaGlobalAllocator::allocate(num_blocks, SodaGenEnum::young_gen);
  if (hb == nullptr) return nullptr;

  SodaBlockArchive::record_humongous(hb);
  return (HeapWord*)hb->start();
}

HeapWord* SodaHeap::allocate_loaded_archive_space(size_t size) {
  bool ignorance = false;
  return mem_allocate(size, &ignorance);
}

void SodaHeap::collect(GCCause::Cause cause) {
  switch (cause) {
    case GCCause::_metadata_GC_threshold:
    case GCCause::_metadata_GC_clear_soft_refs:
      // Receiving these causes means the VM itself entered the safepoint for metadata collection.

      assert(SafepointSynchronize::is_at_safepoint(), "Expected at safepoint");
      log_info(gc)("GC request for \"%s\" is handled", GCCause::to_string(cause));
      MetaspaceGC::compute_new_size();
      print_metaspace_info();
      break;
    default:
      log_info(gc)("GC request for \"%s\" is ignored", GCCause::to_string(cause));
  }
  _monitoring_support->update_counters();
}

void SodaHeap::do_full_collection(bool clear_all_soft_refs) {
  collect(gc_cause());
}

void SodaHeap::object_iterate(ObjectClosure *cl) {
  log_info(gc)("Heap object iteration is unavailable for now.");
}

void SodaHeap::print_on(outputStream *st) const {
  st->print_cr("Soda Heap");

  _virtual_space.print_on(st);

  MetaspaceUtils::print_on(st);
}

bool SodaHeap::print_location(outputStream* st, void* addr) const {
  return BlockLocationPrinter<SodaHeap>::print_location(st, addr);
}

void SodaHeap::print_tracing_info() const {
  print_heap_info(used());
  print_metaspace_info();
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

size_t SodaHeap::used() const {
  return block_size() *
         (capacity_in_blocks() - SodaGlobalAllocator::num_free_blocks());
}

void SodaHeap::stop() {
  SodaGlobalAllocator::clear_lfs();
  SodaBlockArchive::clear();
}
