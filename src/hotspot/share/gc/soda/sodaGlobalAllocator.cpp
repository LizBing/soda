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

#include "gc/soda/sodaGenEnum.hpp"
#include "gc/soda/sodaGlobalAllocator.hpp"
#include "runtime/mutexLocker.hpp"

uintx SodaGlobalAllocator::_num_free_blocks;
SodaGlobalAllocator::AVL SodaGlobalAllocator::_avl;
SodaHeapBlockStack SodaGlobalAllocator::_stack;
SodaHeapBlockLFStack SodaGlobalAllocator::_lfs[SodaGenEnum::num_gens];
uintx SodaGlobalAllocator::_active_blocks[SodaGenEnum::num_gens];

void SodaGlobalAllocator::initialize() {
  _num_free_blocks = SodaHeap::heap()->capacity_in_blocks();

  auto hb = SodaHeapBlocks::at(0);
  hb->_blocks = _num_free_blocks;

  hb->_header = hb;
  hb->last()->_header = hb;

  hb->_free = true;
  _reclaim(hb);
}

SodaHeapBlock* SodaGlobalAllocator::allocate(int num_blocks, int gen) {
  SodaHeapBlock* src = nullptr;
  SodaHeapBlock* hb = nullptr;
  AVL::Node* n = nullptr;

  MutexLocker ml(Heap_lock);

  if (num_blocks == 1) {
    hb = _stack.pop();
    if (hb != nullptr)
      goto ret;
  }

  for (;;) {
    n = _avl.find_equal_or_successor(num_blocks);
    if (n == nullptr)
      return nullptr;

    auto ssg = n->get_value();
    src = ssg->pop();
    if (ssg->size() == 0) {
      _avl.erase(n->key());
      delete ssg;
    }

    if (src != nullptr)
      break;;
  }

  if (num_blocks == src->blocks())
    hb = src;
  else {
    hb = src->partition(num_blocks);
    _reclaim(src);
  }

ret:
  hb->claim_occupied();
  hb->_gen = gen;

  _num_free_blocks -= num_blocks;
  _active_blocks[gen] += num_blocks;

  return hb;
}

void SodaGlobalAllocator::reclaim(SodaHeapBlock *hb) {
  assert(!hb->is_free(), "should not be reclaimed twice");

  hb->reset();

  MutexLocker ml(Heap_lock);
  _num_free_blocks += hb->blocks();
  _active_blocks[hb->gen()] -= hb->blocks();

  auto cp = hb->cont_prev();
  auto cn = hb->cont_next();

  if (cp != nullptr && cp->is_free()) {
    cp->_same_sized_group->erase(cp);
    cp->merge(hb);
    hb = cp;
  } else
    hb->_free = true;

  if (cn != nullptr && cn->is_free()) {
    cn->_same_sized_group->erase(cn);
    hb->merge(cn);
  }

  _reclaim(hb);
}

void SodaGlobalAllocator::_reclaim(SodaHeapBlock* hb) {
  assert(hb->is_free(), "should be free");

  SodaHeapBlockStack* ssg = nullptr;

  if (hb->blocks() == 1)
    ssg = &_stack;
  else {
    auto n = _avl.find(hb->blocks());
    if (n == nullptr) {
      ssg = new SodaHeapBlockStack();
      _avl.insert(hb->blocks(), ssg);
    } else
      ssg = n->get_value();
  }

  ssg->push(hb);
}
