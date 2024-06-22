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
#include "memory/allocation.inline.hpp"
#include "memory/allStatic.hpp"

class SodaHeapBlock;

using CardValue = uint8_t;

class SodaFreeLineTable : AllStatic {
public:
  static void initialize(intptr_t heap_start) {
    _heap_start = heap_start;
    _cards = NEW_C_HEAP_ARRAY(CardValue, size(), mtGC);
    memset(_cards, clean_card_value(), size());
  }

public:
  static CardValue clean_card_value() { return 0; }
  static CardValue dirty_card_value() { return 1; }

public:
  static CardValue* card_for(intptr_t p) {
    auto ls = SodaHeap::heap()->line_size();
    p = align_down(p, ls);
    return _cards + (p - _heap_start) / ls;
  }

  static intptr_t addr_for(CardValue* p) {
    return _heap_start + (p - _cards) * SodaHeap::heap()->line_size();
  }

private:
  static size_t size() { return SodaHeap::heap()->capacity_in_lines(); }

public:
  // no need to provide a parallel version
  static void mark(oop p) {
    auto start = card_for((intptr_t)p.obj());
    auto cards = cards_to_mark(p);

    for (int i = 0; i < cards; ++i)
      start[i] = dirty_card_value();
  }

private:
  static int cards_to_mark(oop p) {
    auto ls = SodaHeap::heap()->line_size();
    auto addr = (intptr_t)p.obj();
    auto len = (p->size() - 1) * HeapWordSize;

    return (align_down(addr + len, ls) - align_down(addr, ls)) / ls + 1;
  }

private:
  static intptr_t _heap_start;
  static CardValue* _cards;
};

class SodaFreeLineDiscoverer : public StackObj {
  friend class SodaHeapBlock;

public:
  struct Closure {
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
    auto s = _end - _begin;
    memset(_begin, SodaFreeLineTable::clean_card_value(), _end - _begin);
  }

private:
  static size_t cards_per_block() { return SodaLinesPerHeapBlock; }

private:
  CardValue* _begin;
  CardValue* _iter;
  CardValue* _end;
};

#endif // SHARE_GC_SODA_SODAFREELINETABLE_HPP
