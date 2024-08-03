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

#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/shared/gc_globals.hpp"
#include "memory/allocation.hpp"
#include "memory/memRegion.hpp"

class SodaHeapBlockStack;
class SodaObjAllocator;

class SodaHeapBlock {
  friend class SodaGlobalAllocator;
  friend class SodaHeapBlocks;
  friend class SodaHeapBlockStack;
  friend class SodaObjAllocator;

public:
  struct Type {
    enum Tag {
      Free,
      Normal,
      HumonguousStart,
      HumonguousCont,
      EndTag
    };
  };

private:
  SodaHeapBlock();

public:
  static SodaHeapBlock* volatile* next_ptr(SodaHeapBlock& n) {
    return &n._next;
  }

  Type::Tag tag() { return _tag; }
  bool well_tagged() { return _tag < Type::EndTag; }

public:
  void set_humonguous() {
    for (uint i = 1; i < _blocks; ++i) {
      this[i]._tag = Type::HumonguousCont;
    }
    _tag = Type::HumonguousStart;
  }

  void set_free() {
    for (uint i = 0; i < _blocks; ++i)
      this[i]._tag = Type::Free;
  }

  void set_normal() {
    _tag = Type::Normal;
  }

  intptr_t start() { return _start; }
  int blocks() { return _blocks; }
  void set_blocks(uint n) {
    _blocks = n;
    _header = this;
    last()->_header = this;
  }
  // in bytes
  size_t size() {
    return SodaHeap::heap()->block_size() * _blocks;
  }

  MemRegion mr() {
    return MemRegion((HeapWord*)_start, size() / HeapWordSize);
  }

  uintx index();

  void evaluate_fragmentation();
  bool should_be_evacuate() { return _should_be_evacuate; }

  SodaHeapBlock* next() { return _next; }
  void set_next(SodaHeapBlock* n) { _next = n; }

  SodaHeapBlock* cont_prev();
  SodaHeapBlock* cont_next();

  void fill_bumper() { _bumper.fill(mr()); }

public:
  intptr_t alloc(size_t s) { return _bumper.bump(s); }
  intptr_t par_alloc(size_t s) { return _bumper.par_bump(s); }

private:
  SodaHeapBlock* last() { return this + (_blocks - 1); }

private:
  const intptr_t _start;
  size_t _blocks;

  volatile Type::Tag _tag;
  bool _should_be_evacuate;

private:
  SodaHeapBlock* _next;
  SodaHeapBlock* _header;

private:
  SodaBumper _bumper;
};

#endif // SHARE_GC_SODA_SODAHEAPBLOCK_HPP
