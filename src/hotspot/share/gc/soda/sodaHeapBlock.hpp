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
#include "gc/soda/sodaGlobals.hpp"
#include "gc/soda/sodaFlexibleList.hpp"
#include "memory/allocation.hpp"
#include "memory/allStatic.hpp"
#include "utilities/align.hpp"

class SodaHeapBlock;
class SodaHBABuffer;

class SodaHBTable: AllStatic {
  friend class SodaHeapBlock;

public:
  static void initialize(uintptr_t heap_base, size_t heap_size);

  static uintx ptr_to_index(uintptr_t);
  static SodaHeapBlock* ptr_to_block(uintptr_t ptr);
  static SodaHeapBlock* get(uintx index);

private:
  static size_t calc_capacity(size_t heap_size) {
    assert(is_aligned(heap_size, SodaGlobals::block_size), "should be aligned.");

    return heap_size >> SodaGlobals::log_block_size;
  }

private:
  static uintptr_t _heap_base;
  static size_t _blocks;
  static SodaHeapBlock* _array;
};

class SodaHBNode: StackObj {
  friend class FirstFitClosure;
  friend class SodaHBAllocator;

public:
  SodaHeapBlock* unwrap();
  uintx index();  // forwarding

  size_t blocks() { return _blocks; }
  SodaHBNode* header() { return _node_header; }
  SodaHBNode* tail();

  bool is_free();
  void set_free() { _free_tag = true; }
  void set_occupied() { _free_tag = false; }

public:
  SodaHBNode* partition(size_t n);
  void set_up(size_t blocks);
  bool merge();   // attempt to merge with the next following block

private:
  uintx calc_partition_res_index(size_t n) {
    assert(n > 0 && n < blocks(), "fail to partition");
    return index() + blocks() - n;
  }

private:
  SodaFlexibleListNode _node;

private:
  bool _free_tag;

  SodaHBNode* _node_header;
  size_t _blocks;
};

class SodaHeapBlock: public CHeapObj<mtGC> {
  // friend class SodaHBABuffer;
  friend class SodaHBAllocator;
  friend class SodaHBNode;

public:
  uintx index() { return this - SodaHBTable::_array; }
  SodaHBNode* node() { return &_node; }

public:
  bool is_reused() { return _is_reused; }

  uintptr_t alloc(size_t s);
  void undo_allocation(uintptr_t ptr, size_t s);

private:
  void ensure_parsability() {
    Universe::heap()->fill_with_dummy_object((HeapWord*)_top, (HeapWord*)_end, false);
  }

private:
  bool _is_reused;
  SodaHBNode _node;

  volatile uintptr_t _top;
  uintptr_t _start;
  uintptr_t _end;
};



#endif // SHARE_GC_SODA_SODAHEAPBLOCK_HPP
