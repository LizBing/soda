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

#ifndef SHARE_GC_SODA_SODAHEAPBLOCKSET_HPP
#define SHARE_GC_SODA_SODAHEAPBLOCKSET_HPP

#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/soda/sodaHeapBlock.hpp"
#include "memory/allStatic.hpp"
#include "memory/allocation.hpp"
#include "utilities/lockFreeStack.hpp"

class SodaHeapBlocks : AllStatic {
  friend class SodaHeapBlock;

public:
  static void initialize() {
    _blocks = NEW_C_HEAP_ARRAY(SodaHeapBlock, size(), mtGC);
    new(_blocks) SodaHeapBlock[size()]();
  }

public:
  static size_t size() {
    return SodaHeap::heap()->capacity_in_blocks();
  }

  static SodaHeapBlock* at(uintx idx) {
    assert(idx < size(), "invaild block index");
    return _blocks + idx;
  }

  static SodaHeapBlock* block_for(intptr_t p) {
    auto heap = SodaHeap::heap();

    return at((align_down(p, heap->block_size()) - heap->heap_start()) /
              heap->block_size());
  }

  static SodaHeapBlock* block_for(oop p) {
    return block_for((intptr_t)(void*)p);
  }

private:
  static SodaHeapBlock* _blocks;
};

using SodaHeapBlockLFStack =
  LockFreeStack<SodaHeapBlock, SodaHeapBlock::next_ptr>;

// thread-safe
class SodaBlockArchive : AllStatic {
public:
  struct Closure {
    virtual void do_block(SodaHeapBlock*) = 0;
  };

public:
  static void record_young(SodaHeapBlock* hb) {
    record(&_young, hb);
  }
/*
  static void record_old(SodaHeapBlock* hb) {
    record(&_old, hb);
  }
*/
  static void record_humongous(SodaHeapBlock* hb) {
    record(&_humongous, hb);
  }

  static void pop_all_young(Closure* cl) { pop_all(&_young, cl); }
  // static void pop_all_old(Closure* cl) { pop_all(&_old, cl); }
  static void pop_all_humongous(Closure* cl) { pop_all(&_humongous, cl); }

  static void clear() {
    _young.pop_all();
    // _old.pop_all();
    _humongous.pop_all();
  }

private:
  static void record(SodaHeapBlockLFStack* s,
                     SodaHeapBlock* hb) {
    s->push(*hb);
  }

  static void pop_all(SodaHeapBlockLFStack* s, Closure* cl) {
    for (;;) {
      auto n = s->pop();
      if (n == nullptr) return;
      cl->do_block(n);
    }
  }

private:
  static SodaHeapBlockLFStack _young;
  // static SodaHeapBlockLFStack _old;
  static SodaHeapBlockLFStack _humongous;
};


#endif // SHARE_GC_SODA_SODAHEAPBLOCKSET_HPP
