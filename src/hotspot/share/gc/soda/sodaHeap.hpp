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

#ifndef SHARE_GC_SODA_SODAHEAP_HPP
#define SHARE_GC_SODA_SODAHEAP_HPP

#include "gc/parallel/parallelScavengeHeap.inline.hpp"
#include "gc/shared/gc_globals.hpp"
#include "gc/shared/collectedHeap.hpp"
#include "gc/shared/gcLocker.inline.hpp"
#include "gc/shared/softRefPolicy.hpp"
#include "gc/soda/sodaAVL.hpp"
#include "gc/soda/sodaBarrierSet.hpp"
#include "services/memoryManager.hpp"
#include "utilities/globalDefinitions.hpp"

class WorkerThreads;

// Used as Soda Heap
class SodaHeap : public ParallelScavengeHeap {
  friend class VMStructs;

public:
  static SodaHeap* heap();

  SodaHeap();

  Name kind() const override {
    return CollectedHeap::Soda;
  }

  const char* name() const override {
    return "Soda(Parallel Scavenge Agency)";
  }

  jint initialize() override;
  void post_initialize() override;
  void initialize_serviceability() override;

  // false for now
  // bool requires_barriers(stackChunkOop obj) const override { return false; }
  bool uses_stack_watermark_barrier()       const override { return false; }

  size_t max_tlab_size() const override {
    return align_down(min_humongous() / HeapWordSize, MinObjAlignment);
  }

  // No support for block parsing.
  HeapWord* block_start(const void* addr) const override { return nullptr; }
  bool block_is_obj(const HeapWord* addr) const override { return false; }

  WorkerThreads* safepoint_workers() override { return &_workers; }

  void ensure_parsability(bool retire_tlabs) override;

public:
  void print_on(outputStream *st) const override;
  void print_heap_info(size_t used) const;
  void print_metaspace_info() const;

public:
  HeapWord* mem_allocate(size_t word_size,
                         bool *gc_overhead_limit_was_exceeded) override;

  HeapWord* alloc_humongous(size_t byte_size);

public:
  void gc_threads_do(ThreadClosure *tc) const override;

private:
  // hot field
  size_t _line_size;
  size_t _block_size;

public:
  size_t line_size() const { return _line_size; }
  size_t block_size() const { return _block_size; }

  size_t min_humongous() const { return block_size() >> 1; }

  WorkerThreads* par_workers() { return &_workers; }
  WorkerThreads* conc_workers() { return _conc_workers; }

public:
  void stop() override;

private:
  WorkerThreads* _conc_workers;
};

#endif // SHARE_GC_SODA_SODAHEAP_HPP
