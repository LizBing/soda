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

#include "gc/shared/taskTerminator.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaSharedMarker.hpp"
#include "memory/allocation.hpp"
#include "memory/memRegion.hpp"

MarkBitMap SodaSharedMarker::_mbm;
SodaHeap* SodaSharedMarker::_heap;

void SodaSharedMarker::initialize() {
  _heap = SodaHeap::heap();

  size_t word_size = MarkBitMap::compute_size(_heap->max_capacity()) /
                     HeapWordSize;
  auto mem = NEW_C_HEAP_ARRAY(HeapWord, word_size, mtGC);
  MemRegion storage(mem, word_size);

  _mbm.initialize(_heap->reserved_region(), storage);
}

void SodaSharedMarker::prepare_for_marking(bool compact_gc) {
  assert_at_safepoint();

  _heap->ensure_parsability(compact_gc);
}
