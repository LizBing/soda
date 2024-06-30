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
#include "runtime/mutexLocker.hpp"

int SodaGlobalAllocator::_num_free_blocks;
SodaGlobalAllocator::AVL SodaGlobalAllocator::_avl;
SodaHeapBlockStack SodaGlobalAllocator::_stack;
SodaHeapBlockLFStack SodaGlobalAllocator::_lfs[SodaGenEnum::num_gens];

SodaHeapBlock* SodaGlobalAllocator::allocate(int num_blocks, int gen) {
  SodaHeapBlock* src = nullptr;
  AVL::Node* n = nullptr;

  MutexLocker ml(Heap_lock);

  SodaHeapBlock* hb = nullptr;
  if (num_blocks == 1) {
    hb = _stack.pop();
    if (hb != nullptr) goto ret;
  }

  // get source block & prune
  {
    n = _avl.find_equal_or_successor(num_blocks);
    if (n == nullptr) return nullptr;

    auto ssg = n->get_value();
    src = ssg->pop();
    if (ssg->size() == 0)
      // abandoned 0 sized same sized group
      // maybe we should reserve some common SSGs for further maintainance
      _avl.erase(n->key());
  }

  // process source block
  if (num_blocks < n->key()) {
    hb = src->partition(num_blocks);
    _reclaim(src);
  } else
    hb = src;

  hb->claim_occupied();

ret: // record and quit
  hb->_gen = gen;
  _num_free_blocks -= num_blocks;

  return hb;
}

void SodaGlobalAllocator::reclaim(SodaHeapBlock *hb) {
  hb->reset();

  MutexLocker ml(Heap_lock);
  _num_free_blocks += hb->blocks();

  auto cp = hb->cont_prev();
  auto cn = hb->cont_next();

  if (cn != nullptr && cn->is_free())
    hb->merge(cn);

  if (cp != nullptr && cp->is_free()) {
    cp->merge(hb);
    hb = cp;
  }

  _reclaim(hb);
}

void SodaGlobalAllocator::_reclaim(SodaHeapBlock* hb) {
  if (hb->blocks() == 1) {
    _stack.push(hb);
    return;
  }

  auto n = _avl.find(hb->blocks());
  if (n != nullptr)
    n->get_value()->push(hb);
  else {
    auto ssg = new SodaHeapBlockStack;
    ssg->push(hb);
    _avl.insert(hb->blocks(), ssg);
  }
}
