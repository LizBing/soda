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

#include "gc/soda/sodaHeapBlockSet.hpp"
#include "precompiled.hpp"

#include "gc/soda/sodaGlobalAllocator.hpp"
#include "gc/soda/sodaRecyclingAllocator.hpp"

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

intptr_t SodaRecyclingAllocator::allocate(size_t s) {
  intptr_t mem = 0;

  if (_hb != nullptr) {
    mem = _bumper.bump(s);
    if (mem != 0) return mem;
  }

  return alloc_slow(s);
}

intptr_t SodaRecyclingAllocator::alloc_slow(size_t s) {
  if (_hb != nullptr) {
    if (fill()) return _bumper.bump(s);

    // about to get another recyclable heap block
    retire();
  }

  _hb = SodaGlobalAllocator::alloc_reusing();
  if (_hb == nullptr) return 0;

  _discoverer.initialize(_hb->start());
  assert(fill(), "should fill successfully");

  return _bumper.bump(s);
}

bool SodaRecyclingAllocator::fill() {
  assert(_hb != nullptr, "heap block should not be null");

  RecycleClosure cl;
  if (_discoverer.discover(&cl)) return false;

  _bumper.fill(cl.discovery());
  return true;
}
