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
#include "gc/soda/sodaGenEnum.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"
#include "gc/soda/sodaTLAB.hpp"

bool SodaTLAB::refill_small() {
  MemRegion mr;
  bool ok = false;
  bool trigger = false;

  if (_reusing)
retry:
    mr = _small->discover_one_recyclable(&ok);

  if (!ok) {
    trigger = true;

    _small = SodaGlobalAllocator::alloc_reusing(SodaGenEnum::young_gen);
    if (_small != nullptr) {
      _reusing = true;
      goto retry;
    }

    _small = SodaGlobalAllocator::allocate(1, SodaGenEnum::young_gen);
    if (_small == nullptr) return false;
    mr = _small->mr();
  }

  if (trigger)
    SodaBlockArchive::record_young(_small);

  _small_bumper.fill((intptr_t)mr.start(), (intptr_t)mr.end());

  return true;
}

bool SodaTLAB::refill_medium() {
  auto hb = SodaGlobalAllocator::allocate(1, SodaGenEnum::young_gen);
  if (hb == nullptr) return false;
  SodaBlockArchive::record_young(hb);

  _medium_bumper.fill(hb->start(), hb->start() + SodaHeap::heap()->block_size());

  return true;
}

intptr_t SodaTLAB::alloc_small(size_t s) {
  intptr_t res = 0;
  if (!_small_bumper.empty()) {
    res = _small_bumper.bump(s);
    if (res != 0) return res;
  }

  return refill_small() ? _small_bumper.bump(s) : 0;
}

intptr_t SodaTLAB::alloc_medium(size_t s) {
  intptr_t res = 0;
  if (!_medium_bumper.empty()) {
    res = _medium_bumper.bump(s);
    if (res != 0) return res;
  }
  return refill_medium() ? _medium_bumper.bump(s) : 0;
}

