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

#include "gc/shared/genArguments.hpp"
#include "gc/soda/sodaGlobalAllocator.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"
#include "gc/soda/sodaMutableSpace.hpp"

SodaMutableSpace::SodaMutableSpace(size_t page_size):
  MutableSpace(page_size), _heap(SodaHeap::heap())
{
  assert(_immix_space == nullptr, "only one instance of SodaMutableSpace available");
  _immix_space = this;

  assert(is_aligned(MaxOldSize, _heap->block_size()), "should be aligned");
  _max_capacity_in_blocks = MaxOldSize / _heap->block_size();

  set_bottom(_heap->reserved_region().start());
  SodaHeapBlocks::initialize();
}

void SodaMutableSpace::ensure_constructable() {
  SodaBlockArchive::clear();
  SodaGlobalAllocator::clear();
}

void SodaMutableSpace::initialize(MemRegion mr,
                                  bool clear_space,
                                  bool mangle_space,
                                  bool setup_pages,
                                  WorkerThreads* pretouch_workers) {
  MutableSpace::initialize(mr, clear_space, mangle_space, setup_pages,
                           pretouch_workers);

  // the top has been set
  ensure_constructable();

  if (clear_space) {
    clear(mangle_space);
  }

  ;
}

void SodaMutableSpace::set_top(HeapWord *value) {
  ;
}

