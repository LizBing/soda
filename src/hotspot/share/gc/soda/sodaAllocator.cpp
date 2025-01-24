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

#include "gc/soda/sodaFlexibleList.hpp"
#include "gc/soda/sodaHeapBlock.hpp"
#include "precompiled.hpp"

#include "gc/soda/sodaAllocator.hpp"
#include "gc/soda/sodaContainerOf.hpp"
#include "gc/soda/sodaHeapBlock.inline.hpp"
#include "memory/allocation.hpp"
#include "runtime/atomic.hpp"
#include "runtime/mutexLocker.hpp"

SodaHeapBlock* volatile SodaHBABuffer::_bumper = nullptr;
SodaHeapBlock* SodaHBABuffer::_end = nullptr;

SodaHeapBlock* SodaHBABuffer::allocate() {
  SodaHeapBlock* res = nullptr;
  SodaHeapBlock* new_top = nullptr;

  do {
    res = Atomic::load(&_bumper);
    if (res == nullptr)
      return nullptr;

    new_top = SodaHBTable::get(res->index() + 1);
    if (new_top == _end)
      return nullptr;
  } while(Atomic::cmpxchg(&_bumper, res, new_top) != res);

  // no need to set up the block
  // just make it occupied
  res->node()->set_occupied();

  return res;
}

SodaLinkedList SodaHBAllocator::_freeList;
SodaLockFreeStack SodaHBAllocator::_separateds;
SodaLockFreeStack SodaHBAllocator::_reusables;

SodaHeapBlock* SodaHBAllocator::alloc_reusable() {
  return unwrap(_reusables.pop());
}

class FirstFitClosure: public SodaLListClosure {
public:
  FirstFitClosure(size_t require):
  _req(require), _res(nullptr) {}

public:
  SodaHBNode* result() { return _res; }

public:
  bool do_node(SodaFlexibleListNode* n) override {
    auto sub_node = container_of(n, SodaHBNode, _node);

    if (_req >= sub_node->blocks()) {
      _res = sub_node;
      return false;
    }

    return true;
  }

private:
  size_t _req;
  SodaHBNode* _res;
};

SodaHeapBlock* SodaHBAllocator::slow_path(size_t n) {
  MutexLocker ml(Heap_lock);

  FirstFitClosure cl(n);
  _freeList.iterate(&cl);

  SodaHBNode* res = nullptr;
  auto node = cl.result();
  // out of memory
  if (node == nullptr) return nullptr;
  assert(node->blocks() >= n, "bad code");

  if (node->blocks() == n) {
    node->_node.erase();
    res = node;
  } else {
    res = node->partition(n);
    if (node->blocks() == 1) {
      node->_node.erase();
      _separateds.push(node->_node);
    }
  }

  res->set_occupied();
}

SodaHeapBlock* SodaHBAllocator::allocate(size_t n) {
  SodaHeapBlock* res = nullptr;

  if (n == 1) {
    res = unwrap(_separateds.pop());
    if (res == nullptr)
      res = SodaHBABuffer::allocate();

    if (res != nullptr) return res;
  }

  MutexLocker ml(Heap_lock);

  FirstFitClosure cl(n);
  _freeList.iterate(&cl);

  // out of memory
  if (cl.result() == nullptr) return nullptr;

  // updates the cache
  _cache = cl.result();
  res = cache_path(n);
  assert(res != nullptr, "bad code");

  return res;
}
