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

#ifndef SHARE_GC_SODA_SODATLAB_HPP
#define SHARE_GC_SODA_SODATLAB_HPP

#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaHeapBlock.hpp"

class SodaTLAB {
public:
  SodaTLAB():
    _reusing(false),
    _small(nullptr),
    _small_bumper(),
    _medium_bumper() {}

public:
  intptr_t allocate(size_t s) {
    auto h = SodaHeap::heap();
    assert(s < h->min_humongous(),
           "humongous objects should not be allocated using TLAB.");

    if (s < h->line_size())
      return alloc_small(s);
    return alloc_medium(s);
  }

private:
  bool refill_small();
  bool refill_medium();
  void refill_helper(SodaBumper* b) {
    if (!b->empty() && b->remaining() > 0)
      SodaHeap::heap()->fill_with_dummy_object(
        (HeapWord*)b->top(),
        (HeapWord*)b->end(), false);
  }

  intptr_t alloc_small(size_t);
  intptr_t alloc_medium(size_t);

private:
  bool _reusing;
  SodaHeapBlock* _small;
  SodaBumper _small_bumper;

  SodaBumper _medium_bumper;

};


#endif // SHARE_GC_SODA_SODATLAB_HPP
