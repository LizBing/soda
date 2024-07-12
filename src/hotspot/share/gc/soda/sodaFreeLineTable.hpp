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

#ifndef SHARE_GC_SODA_SODAFREELINETABLE_HPP
#define SHARE_GC_SODA_SODAFREELINETABLE_HPP

#include "gc/shared/gc_globals.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "memory/allocation.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/allStatic.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"

class SodaHeapBlock;

using CardValue = uint8_t;

class SodaFreeLineTable : AllStatic {
public:
  static void initialize() {
    _cards = NEW_C_HEAP_ARRAY(CardValue, size(), mtGC);
    memset(_cards, clean_card_value(), size());
  }

public:
  static CardValue clean_card_value() { return 0; }
  static CardValue dirty_card_value() { return 1; }

public:
  static CardValue* card_for(intptr_t p) {
    auto heap = SodaHeap::heap();
    auto ls = heap->line_size();

    p = align_down(p, ls);
    uintx idx = (p - heap->heap_start()) / ls;

    assert(idx >= 0 && idx < size(), "Invaild index.");

    return _cards + idx;
  }

  static intptr_t addr_for(CardValue* p) {
    auto heap = SodaHeap::heap();
    uintx idx = pointer_delta((HeapWord*)p, (HeapWord*)_cards);
    assert(idx >= 0 && idx < size(), "Invaild index.");

    return heap->heap_start() + idx * heap->line_size();
  }

private:
  static size_t size() { return SodaHeap::heap()->capacity_in_lines(); }

public:
  // no need to provide a parallel version
  static void mark(oop p) {
    auto start = card_for((intptr_t)(void*)p);
    size_t cards = cards_to_mark(p);

    for (uint i = 0; i < cards; ++i)
      start[i] = dirty_card_value();
  }

private:
  static int cards_to_mark(oop p) {
    auto ls = SodaHeap::heap()->line_size();
    auto addr = (intptr_t)(void*)p;
    auto len = (p->size() - 1) * HeapWordSize;

    return (align_down(addr + len, ls) - align_down(addr, ls)) / ls + 1;
  }

private:
  static CardValue* _cards;
};

class SodaFreeLineDiscoverer : StackObj {
  friend class SodaHeapBlock;

public:
  struct Closure : StackObj {
    // return false to stop iteration
    virtual bool do_free_line(MemRegion) = 0;
  };

private:
  void initialize(intptr_t block_start) {
    _begin = SodaFreeLineTable::card_for(block_start);
    _end = _begin + cards_per_block();

    reset();
  }

public:
  void reset() { _iter = _begin; }

  // returns the result of the closure
  bool discover(Closure*);

  void clear_cards() {
    auto heap = SodaHeap::heap();
    size_t s = _end - _begin;
    assert(s == cards_per_block(), "Should be block sized.");

    memset(_begin, SodaFreeLineTable::clean_card_value(), s);
  }

private:
  static size_t cards_per_block() { return SodaLinesPerHeapBlock; }

private:
  CardValue* _begin;
  CardValue* _iter;
  CardValue* _end;
};

#endif // SHARE_GC_SODA_SODAFREELINETABLE_HPP
