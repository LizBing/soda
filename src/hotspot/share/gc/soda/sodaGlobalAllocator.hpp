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

#ifndef SHARE_GC_SODA_SODAGLOBALALLOCATOR_HPP
#define SHARE_GC_SODA_SODAGLOBALALLOCATOR_HPP

#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaHeapBlock.inline.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"
#include "gc/shared/gc_globals.hpp"
#include "memory/allStatic.hpp"
#include "memory/virtualspace.hpp"
#include "runtime/atomic.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/lockFreeStack.hpp"


class SodaGlobalAllocator : AllStatic {
public:
  static uintx num_free_blocks() { return _num_free_blocks; }

  static uintx active_blocks() { return _active_blocks; }

public:
  static void initialize(size_t num_active_blocks);

  static SodaHeapBlock* allocate(uint num_blocks);

  // the reclamation phase of an immix
  static void reclaim();

  static SodaHeapBlock* alloc_reusing() {
    return _recycle.pop();
  }
  static void reclaim_for_reusing(SodaHeapBlock* hb) {
    _recycle.push(*hb);
  }

  static void clear_lfs() {
    _single.pop_all();
    _recycle.pop_all();
  }

  static void clear();

private:
  static uintx _num_free_blocks;
  static uintx _active_blocks;

  static SodaHeapBlockLFStack _single;   // for one-free-block allocation
  static SodaHeapBlockLFStack _recycle;
};

#endif // SHARE_GC_SODA_SODAGLOBALALLOCATOR_HPP
