/*
 * Copyright (c) 2024, Lei Zaakjyu. All rights reserved.
 * Copyright (c) 2024, Oracle and/or its affiliates. All rights reserved.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_GC_SODA_SODATLAB_HPP
#define SHARE_GC_SODA_SODATLAB_HPP

#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaHeapBlock.hpp"

class SodaTLAB {
public:
  SodaTLAB()
  : _small(nullptr),
    _medium(nullptr) {}

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

  intptr_t alloc_small(size_t);
  intptr_t alloc_medium(size_t);

private:
  bool _reusing;
  SodaHeapBlock* _small;
  SodaHeapBlock* _medium;
};


#endif // SHARE_GC_SODA_SODATLAB_HPP
