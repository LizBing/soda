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

#ifndef SHARE_GC_SODA_SODAALLOCATOR_HPP
#define SHARE_GC_SODA_SODAALLOCATOR_HPP

#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaAVL.hpp"
#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaGenEnum.hpp"
#include "gc/soda/sodaHeapBlock.hpp"
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

public:
  size_t size() { return _size; }

public:
  void push(SodaHeapBlock* hb) {
    ++_size;

    hb->set_same_sized_group(this);

    hb->set_next(_head);
    if (_head != nullptr)
      _head->set_prev(hb);

    _head = hb;
  }

  SodaHeapBlock* pop() {
    auto hb = _head;
    if (hb == nullptr)
      return nullptr;

    --_size;

    auto next = hb->next();
    if (next != nullptr)
      next->set_prev(nullptr);

    _head = next;
    return hb;
  }

  // call when about to merge
  void erase(SodaHeapBlock* hb) {
    --_size;

    auto prev = hb->prev();
    auto next = hb->next();

    if (prev == nullptr) {
      pop();
      return;
    }

    if (next != nullptr)
      next->set_prev(prev);
    prev->set_next(next);
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
  static int num_free_blocks() { return _num_free_blocks; }

public:
  static void initialize() {
    _num_free_blocks = SodaHeap::heap()->capacity_in_blocks();
    auto hb = SodaHeapBlocks::at(0);
    hb->_blocks = _num_free_blocks;

    _reclaim(hb);
  }

  static SodaHeapBlock* allocate(int num_blocks, int gen);
  static void reclaim(SodaHeapBlock* hb);

  static SodaHeapBlock* alloc_reusing(int gen) {
    return _lfs[gen].pop();
  }
  static void reclaim_for_reusing(int gen, SodaHeapBlock* hb) {
    _lfs[gen].push(*hb);
  }

  static void clear_lfs() {
    for (int i = 0; i < SodaGenEnum::num_gens; ++i)
      _lfs->pop_all();
  }

private:
  static void _reclaim(SodaHeapBlock* hb);

private:
  static int _num_free_blocks;

  static AVL _avl;
  static SodaHeapBlockStack _stack;   // for one-free-block allocation
  static SodaHeapBlockLFStack _lfs[SodaGenEnum::num_gens];
};

#endif // SHARE_GC_SODA_SODAALLOCATOR_HPP
