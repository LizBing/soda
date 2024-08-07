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

#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/soda/sodaGlobalAllocator.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaHeapBlock.inline.hpp"
#include "gc/soda/sodaObjAllocator.hpp"
#include "gc/shared/tlab_globals.hpp"
#include "memory/allocation.hpp"
#include "runtime/atomic.hpp"
#include "runtime/os.hpp"
#include "utilities/globalDefinitions.hpp"

uintx SodaObjAllocator::_undone_hb = 0;
SodaHeapBlock** SodaObjAllocator::_shared_blocks;

void SodaObjAllocator::initialize() {
  _shared_blocks = new(NEW_C_HEAP_ARRAY(SodaHeapBlock*,
                       os::processor_count(), mtGC))
                     SodaHeapBlock*[os::processor_count()]();
}

intptr_t SodaObjAllocator::alloc(size_t size) {
  intptr_t mem = 0;

  auto shared = _shared_blocks + os::processor_id();
  auto hb = Atomic::load(shared);
  if (hb != nullptr) {
    mem = hb->par_alloc(size);
    if (mem != 0) return mem;
  }

  auto new_hb = SodaGlobalAllocator::allocate(1);
  if (new_hb == nullptr) return 0;

  new_hb->fill_bumper();
  mem = new_hb->alloc(size);

  // try to install the new heap block
  SodaHeapBlock* prev_hb = nullptr;
  intptr_t tmp = 0;
retry:
  prev_hb = Atomic::cmpxchg(shared, hb, new_hb);
  if (prev_hb == hb) {  // success, return
    if (hb != nullptr)
      SodaBlockArchive::record_normal(hb);
    return mem;
  }

  tmp = prev_hb->par_alloc(size);
  if (tmp == 0) {
    hb = prev_hb;
    goto retry;
  }

  SodaGlobalAllocator::reclaim(new_hb);
  Atomic::inc(&_undone_hb);

  return tmp;
}

inline size_t SodaObjAllocator::undone() {
  return Atomic::load(&_undone_hb) * SodaHeap::heap()->block_size();
}

size_t SodaObjAllocator::unsafe_max_tlab_alloc() {
  auto hb = Atomic::load(_shared_blocks + os::processor_id());

  return hb == nullptr ? MinTLABSize :
                         MIN2(hb->_bumper.remaining(),
                              SodaHeap::heap()->max_tlab_size() * HeapWordSize);
}

void SodaObjAllocator::retire_blocks() {
  for (int i = 0; i < os::processor_count(); ++i) {
    auto hb = _shared_blocks[i];
    if (hb != nullptr)
      SodaBlockArchive::record_normal(hb);
  }
}
