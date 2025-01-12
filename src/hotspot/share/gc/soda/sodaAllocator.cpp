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
#include "precompiled.hpp"

#include "gc/soda/sodaAllocator.hpp"
#include "gc/soda/sodaContainerOf.hpp"
#include "gc/soda/sodaHeapBlock.hpp"
#include "memory/allocation.hpp"
#include "runtime/mutexLocker.hpp"

SodaLinkedList SodaHBAllocator::_freeList;
SodaLockFreeStack SodaHBAllocator::_separateds;
SodaLockFreeStack SodaHBAllocator::_reusables;
SodaHBNode* SodaHBAllocator::_cache = nullptr;

SodaHeapBlock* SodaHBAllocator::alloc_reusable() {
  return unwrap(_reusables.pop());
}

SodaHeapBlock* SodaHBAllocator::cache_path(size_t n) {
  assert(Heap_lock->is_locked(), "should be protected by Heap Lock.");

  SodaHeapBlock* res = nullptr;

  if (_cache != nullptr && _cache->blocks() >= n) {
    if (_cache->blocks() == n) {
      _cache->_node.erase();
      res = container_of(_cache, SodaHeapBlock, _manager_set);
      clear_cache();
    } else {
      res = container_of(_cache->partition(n), SodaHeapBlock, _manager_set);

      if (_cache->blocks() == 1) {
        _cache->_node.erase();
        _separateds.push(_cache->_node);
        clear_cache();
      }
    }
  }

  return res;
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

SodaHeapBlock* SodaHBAllocator::allocate(size_t n) {
  SodaHeapBlock* res = nullptr;

  if (n == 1) {
    res = unwrap(_separateds.pop());
    if (res != nullptr) return res;
  }

  MutexLocker ml(Heap_lock);

  res = cache_path(n);
  if (res != nullptr) return res;

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
