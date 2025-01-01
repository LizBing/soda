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

#ifndef SHARE_GC_SODA_SODAHEAPBLOCK_HPP
#define SHARE_GC_SODA_SODAHEAPBLOCK_HPP

#include "gc/shared/collectedHeap.hpp"
#include "memory/allocation.hpp"
#include "memory/allStatic.hpp"
#include "utilities/align.hpp"

class SodaHBNode;

class SodaHeapBlock: public CHeapObj<mtGC> {
public:
  bool is_reused() { return _is_reused; }

  SodaHBNode* node() { return _node; }
  void set_node(SodaHBNode* n) { _node = n; }

  uintptr_t alloc(size_t s) {
    assert(is_aligned(s, HeapWordSize), "should be aligned");
    assert(vaild_top(), "block hasn't been initialized");

    if (ok_to_allocate(s)) {
      uintptr_t tmp = _top;
      _top += s;
      return tmp;
    }

    ensure_parsability();
    return false;
  }

  void undo_allocation(uintptr_t ptr, size_t s) {
    assert(ptr != 0, "should not be null");

    if (ptr == _top)
      _top -= s;

    assert(vaild_top(), "broken block");
  }

private:
  bool vaild_top() { return _top >= _start && _top < _end; }
  bool ok_to_allocate(size_t s) {
    return _top + s == _end ||
           _top + s + CollectedHeap::min_dummy_object_size() < _end;
  }

  void ensure_parsability() {
    Universe::heap()->fill_with_dummy_object((HeapWord*)_top, (HeapWord*)_end, false);
  }

private:
  bool _is_reused;
  SodaHBNode* _node;

  volatile uintptr_t _top;
  uintptr_t _start;
  uintptr_t _end;
};

class SodaHBTable: AllStatic {};


#endif // SHARE_GC_SODA_SODAHEAPBLOCK_HPP
