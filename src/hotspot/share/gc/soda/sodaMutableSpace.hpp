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

#ifndef SHARE_GC_SODA_SODAMUTABLESPACE_HPP
#define SHARE_GC_SODA_SODAMUTABLESPACE_HPP

#include "gc/parallel/mutableSpace.hpp"
#include "gc/shared/gc_globals.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "logging/log.hpp"

class SodaMutableSpace : public MutableSpace {
  static SodaMutableSpace* _immix_space;

public:
  // @size: in bytes
  static uintx calc_capable_block_size(size_t size) {
    size_t bs = SodaHeap::heap()->block_size();
    return align_up(size, bs) / bs;
  }

public:
  SodaMutableSpace(size_t page_size);

  static SodaMutableSpace* immix_space() { return _immix_space; }

// Accessors
public:
  size_t max_capacity_in_blocks() const { return _max_capacity_in_blocks; }
  size_t max_capacity_in_lines() const {
    return _max_capacity_in_blocks * SodaLinesPerHeapBlock;
  }

  size_t capacity_in_blocks() const {
    size_t byte_size = capacity_in_bytes();
    size_t block_size = _heap->block_size();
    assert(is_aligned(byte_size, block_size),
           "should be aligned");

    return byte_size / block_size;
  }

public:
  // mt-safe
  void initialize(MemRegion mr,
                  bool clear_space,
                  bool mangle_space,
                  bool setup_pages = SetupPages,
                  WorkerThreads* pretouch_workers = nullptr) override;

  void set_top(HeapWord *value) override;

  void set_top_for_allocations() override {
    MutableSpace::set_top_for_allocations();
    log_warning(gc, heap)("Immix heap structure doesn't have a real top.");
  }

  size_t used_in_words() const override;
  size_t free_in_words() const override;
  // unsupported
  size_t tlab_capacity(Thread* thr) const override {
    ShouldNotReachHere();
    return 0;
  }
  size_t tlab_used(Thread* thr) const override {
    ShouldNotReachHere();
    return 0;
  }
  size_t unsafe_max_tlab_alloc(Thread* thr) const override {
    ShouldNotReachHere();
    return 0;
  }

  HeapWord* cas_allocate(size_t word_size) override;

  void print_on(outputStream *st) const override;
  void print_short_on(outputStream *st) const override;

private:
  void ensure_constructable();

private:
  // hot field
  SodaHeap* _heap;
  size_t _max_capacity_in_blocks;

  // When initializing, set the watermark to the start of free regions,
  // in order to assign a type to the heap regions below.
  intptr_t _watermark;
};


#endif // SHARE_GC_SODA_SODAMUTABLESPACE_HPP
