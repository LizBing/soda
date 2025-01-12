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

#ifndef SHARE_GC_SODA_SODAALLOCATOR_HPP
#define SHARE_GC_SODA_SODAALLOCATOR_HPP

#include "gc/soda/sodaContainerOf.hpp"
#include "gc/soda/sodaFlexibleList.hpp"
#include "gc/soda/sodaHeapBlock.hpp"
#include "memory/allStatic.hpp"
#include "utilities/globalDefinitions.hpp"

class SodaHeapBlock;


class SodaHBAllocator: AllStatic {
public:
  static SodaHeapBlock* alloc_reusable();
  static SodaHeapBlock* allocate(size_t n);

private:
  static SodaHeapBlock* unwrap(SodaFlexibleListNode* n) {
    if (n == nullptr) return nullptr;

    auto sub_node = container_of(n, SodaHBNode, _node);
    return container_of(sub_node, SodaHeapBlock, _manager_set);
  }

  static SodaHeapBlock* cache_path(size_t);

  static void clear_cache() { _cache = nullptr; }

private:
  static SodaLockFreeStack _separateds;
  static SodaLinkedList _freeList;
  static SodaHBNode* _cache;

  static SodaLockFreeStack _reusables;
};

class SodaObjAllocator: AllStatic {
public:
  static uintptr_t allocate(size_t size);
  static uintptr_t alloc_humongous(size_t size);
};


#endif // SHARE_GC_SODA_SODAALLOCATOR_HPP
