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

#include "precompiled.hpp"
#include "gc/soda/sodaAllocator.hpp"
#include "gc/soda/sodaHeapBlock.hpp"

class RecycleClosure : public SodaFreeLineDiscoverer::Closure {
public:
  bool do_free_line(MemRegion mr) override {
    _mr = mr;
    return false;
  }

  MemRegion discovery() { return _mr; }

private:
  MemRegion _mr;
};

SodaHeapBlock::SodaHeapBlock() {
  auto heap = SodaHeap::heap();

  _idx = (ptrdiff_t)((intptr_t)this - (intptr_t)SodaHeapBlocks::_blocks) /
         sizeof(SodaHeapBlock);
  _start = intptr_t(heap->heap_start()) + heap->block_size() * _idx;

  _discoverer.initialize(_start);
  reset(true);

  _cont_next = nullptr;
  _cont_prev = nullptr;
}

intptr_t SodaHeapBlock::alloc_rec(size_t s) {
  auto n = alloc_seq(s);
  if (n != 0) return n;

  RecycleClosure cl;
  if (_discoverer.discover(&cl))
    return 0;

  auto mr = cl.discovery();
  _bumper.fill((intptr_t)mr.start(), (intptr_t)mr.end());

  return alloc_seq(s);
}

SodaHeapBlock* SodaHeapBlock::partition(int n) {
  assert(n < _blocks, "target block size should be less than source block.");
  assert(n > 0, "0 sized block is unavailable.");

  _blocks -= n;
  auto res = SodaHeapBlocks::at(_idx + _blocks);
  res->_blocks = n;

  res->_cont_next = _cont_next;
  res->_cont_prev = this;
  if (_cont_next)
    _cont_next->_cont_prev = res;
  _cont_next = res;

  return res;
}

void SodaHeapBlock::merge(SodaHeapBlock *cn) {
  assert(_idx + _blocks == cn->_idx,
         "should be an upper and contiguous region");

  cn->_same_sized_group->erase(cn);

  _blocks += cn->_blocks;

  auto next = cn->_cont_next;
  if (next != nullptr)
    next->_cont_prev = this;
  _cont_next = next;
}
