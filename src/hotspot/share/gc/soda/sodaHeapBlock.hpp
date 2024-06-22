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
#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/shared/gc_globals.hpp"
#include "memory/memRegion.hpp"

class SodaHeapBlockStack;

class SodaHeapBlock : public CHeapObj<mtGC> {
  friend class SodaHeapBlocks;
  friend class SodaGlobalAllocator;

  SodaHeapBlock();

public:
  static SodaHeapBlock* volatile* next_ptr(SodaHeapBlock& n) {
    return &n._next;
  }

  void claim_occupied() { _free = false; }
  bool is_free() { return _free; }

  intptr_t start() { return _start; }
  int blocks() { return _blocks; }
  MemRegion mr() {
    return MemRegion((HeapWord*)_start, SodaHeap::heap()->block_size());
  }

  uintx index() { return _idx; }

  bool should_be_evacuate() { return _should_be_evacuate; }

  void set_same_sized_group(SodaHeapBlockStack* n) { _same_sized_group = n; }

  SodaHeapBlock* prev() { return _prev; }
  SodaHeapBlock* next() { return _next; }
  void set_prev(SodaHeapBlock* n) { _prev = n; }
  void set_next(SodaHeapBlock* n) { _next = n; }

  SodaHeapBlock* cont_prev() { return _cont_prev; }
  SodaHeapBlock* cont_next() { return _cont_next; }

  void reset(bool init = false) {
    _free = true;
    _should_be_evacuate = false;

    _bumper.fill(_start, _start + SodaHeap::heap()->block_size());
    if (!init)
      _discoverer.reset();

    _prev = nullptr;
    _next = nullptr;
  }

  // forward
  void clear_cards() { _discoverer.clear_cards(); }

private:
  SodaHeapBlock* partition(size_t n);
  // @cn: contiguously next block
  void merge(SodaHeapBlock* cn);

public:
  intptr_t alloc_seq(size_t s) { return _bumper.bump(s); }
  intptr_t alloc_rec(size_t);   // only used by small sized objects allocation

private:
  uintx _idx;

  intptr_t _start;
  size_t _blocks;

  bool _free;
  bool _should_be_evacuate;

  SodaBumper _bumper;
  SodaFreeLineDiscoverer _discoverer;

private:
  SodaHeapBlockStack* _same_sized_group;

  SodaHeapBlock* _prev;
  SodaHeapBlock* _next;

  SodaHeapBlock* _cont_prev;
  SodaHeapBlock* _cont_next;
};

#endif // SHARE_GC_SODA_SODAHEAPBLOCK_HPP
