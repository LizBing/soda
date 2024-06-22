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
  if (_small != nullptr)
    SodaBlockArchive::record_young(_small);

  _small = SodaGlobalAllocator::alloc_reusing(SodaGenEnum::young_gen);
  if (_small == nullptr) {
    _small = SodaGlobalAllocator::allocate(1);
    if (_small == nullptr) return false;
    _reusing = false;
  } else _reusing = true;

  return true;
}

bool SodaTLAB::refill_medium() {
  if (_medium != nullptr)
    SodaBlockArchive::record_young(_medium);

  _medium = SodaGlobalAllocator::allocate(1);
  return _medium != nullptr ? true : false;
}

intptr_t SodaTLAB::alloc_small(size_t s) {
  intptr_t res = 0;

  if (_small != nullptr) {
    res = _reusing ? _small->alloc_rec(s) : _small->alloc_seq(s);
    if (res != 0) return res;
  }

  if (refill_small())
    return alloc_small(s);
  return 0;
}

intptr_t SodaTLAB::alloc_medium(size_t s) {
  intptr_t res = 0;

  if (_medium != nullptr) {
    res = _medium->alloc_seq(s);
    if (res != 0) return res;
  }

  if (refill_medium())
    return _medium->alloc_seq(s);
  return 0;
}

