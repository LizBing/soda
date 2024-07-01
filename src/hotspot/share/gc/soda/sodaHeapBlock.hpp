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

#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/soda/sodaGenEnum.hpp"
#include "gc/shared/gc_globals.hpp"
#include "memory/memRegion.hpp"

class SodaHeapBlockStack;

class SodaHeapBlock {
  friend class SodaHeapBlocks;
  friend class SodaGlobalAllocator;

private:
  SodaHeapBlock();

public:
  static SodaHeapBlock* volatile* next_ptr(SodaHeapBlock& n) {
    return &n._next;
  }

  void claim_occupied() {
    for (uint i = 0; i < _blocks; ++i)
      this[i]._free = false;
  }
  bool is_free() { return _free; }

  intptr_t start() { return _start; }
  int blocks() { return _blocks; }
  MemRegion mr() {
    return MemRegion((HeapWord*)_start,
                     SodaHeap::heap()->block_size() / HeapWordSize);
  }

  uintx index();

  int gen() { return _gen; }

  bool should_be_evacuate() { return _should_be_evacuate; }

  void set_same_sized_group(SodaHeapBlockStack* n) { _same_sized_group = n; }

  SodaHeapBlock* prev() { return _prev; }
  SodaHeapBlock* next() { return _next; }
  void set_prev(SodaHeapBlock* n) { _prev = n; }
  void set_next(SodaHeapBlock* n) { _next = n; }

  SodaHeapBlock* cont_prev();
  SodaHeapBlock* cont_next();

  void reset(bool init = false) {
    if (init)
      _free = true;
    else
      for (uintx i = 0; i < _blocks; ++i)
        this[i]._free = true;

    _should_be_evacuate = false;

    if (!init)
      _discoverer.reset();

    _prev = nullptr;
    _next = nullptr;
  }

  // forward
  void clear_cards() { _discoverer.clear_cards(); }

private:
  SodaHeapBlock* partition(int n);
  // @cn: contiguously next block
  void merge(SodaHeapBlock* cn);

  SodaHeapBlock* last();

public:
  MemRegion discover_one_recyclable(bool* succeeded);

private:
  intptr_t _start;
  size_t _blocks;

  bool _free;
  bool _should_be_evacuate;
  int _gen;

  SodaFreeLineDiscoverer _discoverer;

private:
  SodaHeapBlockStack* _same_sized_group;

  SodaHeapBlock* _prev;
  SodaHeapBlock* _next;

  SodaHeapBlock* _header;
};

#endif // SHARE_GC_SODA_SODAHEAPBLOCK_HPP
