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

#ifndef SHARE_GC_SODA_SODARECYCLINGALLOCATOR_HPP
#define SHARE_GC_SODA_SODARECYCLINGALLOCATOR_HPP

#include "gc/soda/sodaBumper.hpp"
#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/soda/sodaHeapBlock.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"
#include "memory/allocation.hpp"

class SodaRecyclingAllocator : StackObj {
public:
  SodaRecyclingAllocator():
    _hb(nullptr), _bumper() {}

public:
  void retire() {
    if (_hb != nullptr) {
      _discoverer.clear_cards();
      SodaBlockArchive::record_normal(_hb);
      _hb = nullptr;
    }
  }

  intptr_t allocate(size_t);

private:
  intptr_t alloc_slow(size_t);
  bool fill();

private:
  SodaHeapBlock* _hb;
  SodaFreeLineDiscoverer _discoverer;
  SodaBumper _bumper;
};


#endif // SHARE_GC_SODA_SODARECYCLINGALLOCATOR_HPP
