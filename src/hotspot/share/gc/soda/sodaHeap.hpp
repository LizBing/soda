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

#include "gc/shared/gc_globals.hpp"
#include "gc/shared/collectedHeap.hpp"
#include "gc/shared/gcLocker.inline.hpp"
#include "gc/shared/softRefPolicy.hpp"
#include "gc/soda/sodaAVL.hpp"
#include "gc/soda/sodaBarrierSet.hpp"
#include "gc/soda/sodaMonitoringSupport.hpp"
#include "gc/soda/sodaNMethod.hpp"
#include "memory/virtualspace.hpp"
#include "services/memoryManager.hpp"
#include "utilities/globalDefinitions.hpp"

class WorkerThreads;

// Used as Soda Heap
class SodaHeap : public CollectedHeap {
  friend class VMStructs;

private:
  SoftRefPolicy _soft_ref_policy;
  SodaMonitoringSupport* _monitoring_support;
  MemoryPool* _pool;
  GCMemoryManager _memory_manager;
  VirtualSpace _virtual_space;

public:
  static SodaHeap* heap();

  SodaHeap();

  Name kind() const override {
    return CollectedHeap::Soda;
  }

  const char* name() const override {
    return "Soda";
  }

  SoftRefPolicy* soft_ref_policy() override {
    return &_soft_ref_policy;
  }

  jint initialize() override;
  void post_initialize() override;
  void initialize_serviceability() override;

  GrowableArray<GCMemoryManager*> memory_managers() override;
  GrowableArray<MemoryPool*> memory_pools() override;

  size_t max_capacity() const override { return capacity(); }
  size_t capacity()     const override {
    return _capacity_in_blocks * block_size();
  }
  size_t used()         const override;

  bool is_in(const void* p) const override;

  // false for now
  bool requires_barriers(stackChunkOop obj) const override { return false; }
  bool uses_stack_watermark_barrier()       const override { return false; }

  bool is_maximal_no_gc() const override { return used() == max_capacity(); }

  // Allocation
  HeapWord* allocate_new_tlab(size_t ignore, size_t word_size,
                               size_t* actual_size) override;

  HeapWord* mem_allocate(size_t word_size,
                         bool* gc_overhead_limit_was_exceeded) override;

  // size in bytes
  HeapWord* alloc_humongous(size_t);

  // TLAB allocation
  size_t tlab_capacity(Thread* ignore) const override {
    // return capacity() / (NewRatio + 1); // generational
    return capacity();
  }
  size_t tlab_used(Thread* ignore) const override;

  size_t max_tlab_size() const override {
    return align_down(min_humongous() / HeapWordSize, MinObjAlignment);
  }

  size_t unsafe_max_tlab_alloc(Thread* ignore) const override;

  void collect(GCCause::Cause cause) override;
  void do_full_collection(bool clear_all_soft_refs) override;

  // Heap walking unsupport
  void object_iterate(ObjectClosure* cl) override {
    log_warning(gc)("Heap object iteration is unsupported.");
  }

  void pin_object(JavaThread* thread, oop obj) override {
    GCLocker::lock_critical(thread);
  }
  void unpin_object(JavaThread* thread, oop obj) override {
    GCLocker::unlock_critical(thread);
  }

  // No support for block parsing.
  HeapWord* block_start(const void* addr) const { return nullptr; }
  bool block_is_obj(const HeapWord* addr) const { return false; }

  void gc_threads_do(ThreadClosure* tc) const override;

  void register_nmethod(nmethod* nm) override {
    SodaNMethodTable::register_nmethod(nm);
  }
  void unregister_nmethod(nmethod* nm) override {
    SodaNMethodTable::unregister_nmethod(nm);
  }
  // unsupported
  void verify_nmethod(nmethod* nm) override {}

  // No heap verification
  void prepare_for_verify() override {}
  void verify(VerifyOption option) override {}

  MemRegion reserved_region() const { return _reserved; }
  bool is_in_reserved(const void* addr) const {
    return _reserved.contains(addr);
  }

  void print_on(outputStream* st) const override;
  void print_tracing_info() const override;
  bool print_location(outputStream* st, void* addr) const override;

  WorkerThreads* safepoint_workers() override { return _par_workers; }

  void ensure_parsability(bool retire_tlabs) override;

private:
  void print_heap_info(size_t used) const;
  void print_metaspace_info() const;

// Soda
private:
  // hot field
  size_t _capacity_in_blocks;
  size_t _line_size;
  size_t _block_size;
  intptr_t _heap_start;

public:
  size_t line_size() const { return _line_size; }
  size_t block_size() const { return _block_size; }

  intptr_t heap_start() { return _heap_start; }
  size_t capacity_in_lines() const { return SodaLinesPerHeapBlock * _capacity_in_blocks; }
  size_t capacity_in_blocks() const { return _capacity_in_blocks; }
  size_t min_humongous() const { return block_size() >> 1; }

  WorkerThreads* par_workers() { return _par_workers; }
  WorkerThreads* conc_workers() { return _conc_workers; }

public:
  void stop() override;

private:
  WorkerThreads* _par_workers;
  WorkerThreads* _conc_workers;

};

#endif // SHARE_GC_SODA_SODAHEAP_HPP
