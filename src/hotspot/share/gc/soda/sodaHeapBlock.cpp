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
#include "gc/soda/sodaHeapBlock.inline.hpp"

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

  _start = intptr_t(heap->heap_start()) + heap->block_size() * index();

  _discoverer.initialize(_start);

  reset(true);
}


SodaHeapBlock* SodaHeapBlock::partition(int n) {
  assert(n < _blocks, "target block size should be less than source block.");
  assert(n > 0, "0 sized block is unavailable.");

  auto idx = index();

  _blocks -= n;
  auto res = SodaHeapBlocks::at(idx + _blocks);
  res->_blocks = n;

  last()->_header = this;
  res->last()->_header = res;

  return res;
}

void SodaHeapBlock::merge(SodaHeapBlock *cn) {
  assert(index() + _blocks == cn->index(),
         "should be an upper and contiguous region");

  cn->_same_sized_group->erase(cn);

  _blocks += cn->_blocks;
  cn->last()->_header = this;
}

MemRegion SodaHeapBlock::discover_one_recyclable(bool* succeeded) {
  RecycleClosure cl;

  *succeeded = !_discoverer.discover(&cl);

  return cl.discovery();
}
