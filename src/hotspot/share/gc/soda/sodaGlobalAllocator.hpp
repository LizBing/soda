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

#ifndef SHARE_GC_SODA_SODAGLOBALALLOCATOR_HPP
#define SHARE_GC_SODA_SODAGLOBALALLOCATOR_HPP

#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaAVL.hpp"
#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaHeapBlock.inline.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"
#include "gc/shared/gc_globals.hpp"
#include "memory/allStatic.hpp"
#include "memory/virtualspace.hpp"
#include "runtime/atomic.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/lockFreeStack.hpp"

class SodaHeapBlockStack : public CHeapObj<mtGC> {
public:
  SodaHeapBlockStack()
  : _size(0), _head(nullptr) {}

  ~SodaHeapBlockStack() {
    assert(_size == 0, "should be 0");
  }

public:
  size_t size() { return _size; }

public:
  void push(SodaHeapBlock* hb) {
    assert(hb->_same_sized_group == nullptr, "should be null");

    assert(hb->prev() == nullptr, "should be null");
    assert(hb->next() == nullptr, "should be null");

    hb->set_next(_head);

    if (_head != nullptr)
      _head->set_prev(hb);

    _head = hb;

    _size += 1;
    hb->set_same_sized_group(this);
  }

  SodaHeapBlock* pop() {
    auto hb = _head;
    if (hb == nullptr)
      return nullptr;

    assert(_size > 0, "pop: should be greater than 0");

    auto next = _head->next();
    if (next != nullptr)
      next->set_prev(nullptr);

    _head = next;
    _size -= 1;

    hb->set_same_sized_group(nullptr);
    hb->set_prev(nullptr);
    hb->set_next(nullptr);

    return hb;
  }

  // call when about to merge
  void erase(SodaHeapBlock* hb) {
    assert(_size > 0, "erase: should be greater than 0");
    assert(hb->_same_sized_group == this, "different ssg");

    auto prev = hb->prev();
    auto next = hb->next();

    if (prev == nullptr) {
      pop();
      return;
    }

    _size -= 1;
    if (next != nullptr)
      next->set_prev(prev);
    prev->set_next(next);

    hb->set_same_sized_group(nullptr);
    hb->set_prev(nullptr);
    hb->set_next(nullptr);
  }

private:
  size_t _size;
  SodaHeapBlock* _head;
};

class SodaGlobalAllocator : AllStatic {
  static int cmp(int l, int r) {
    if (l > r) return 1;
    if (l < r) return -1;
    return 0;
  }

  using AVL = SodaAVL<int, SodaHeapBlockStack*, cmp>;

public:
  static uintx num_free_blocks() { return _num_free_blocks; }

  static uintx active_blocks() { return _active_blocks; }

public:
  static void initialize();

  static SodaHeapBlock* allocate(int num_blocks);
  static void reclaim(SodaHeapBlock* hb);

  static SodaHeapBlock* alloc_reusing() {
    return _lfs.pop();
  }
  static void reclaim_for_reusing(SodaHeapBlock* hb) {
    _lfs.push(*hb);
  }

  static void clear_lfs() {
    _lfs.pop_all();
  }

private:
  static void _reclaim(SodaHeapBlock* hb);

private:
  static uintx _num_free_blocks;
  static uintx _active_blocks;

  static AVL _avl;
  static SodaHeapBlockStack _stack;   // for one-free-block allocation
  static SodaHeapBlockLFStack _lfs;
};

#endif // SHARE_GC_SODA_SODAGLOBALALLOCATOR_HPP
