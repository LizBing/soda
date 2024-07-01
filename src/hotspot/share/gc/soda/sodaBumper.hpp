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

#ifndef SHARE_GC_SODA_SODABUMPER_HPP
#define SHARE_GC_SODA_SODABUMPER_HPP

#include "memory/memRegion.hpp"
#include "runtime/atomic.hpp"

// General sequential allocator
class SodaBumper {
public:
  SodaBumper() :
    _empty(true),
    _top(0), _end(0) {}

  void fill(intptr_t start, intptr_t end) {
    _top = start;
    _end = end;

    _empty = false;
  }

  void fill(MemRegion mr) {
    fill((intptr_t)mr.start(), (intptr_t)mr.end());
  }

public:
  bool empty() { return _empty; }
  void set_empty() { _empty = true; }

  intptr_t top() { return _top; }
  intptr_t end() { return _end; }
  size_t remaining() { return _end - _top; }

public:
  intptr_t bump(size_t size) {
    assert(!_empty, "The bumper should be filled before bumping.");

    auto res = _top;
    intptr_t new_top = _top + size;

    if (new_top > _end) return 0;
    _top = new_top;

    return res;
  }

 intptr_t par_bump(size_t size) {
    assert(!_empty, "The bumper should be filled before bumping.");
    intptr_t res = 0;
    intptr_t new_top = 0;

    do {
      res = Atomic::load(&_top);
      new_top = res + size;

      if (new_top > _end) return 0;
    } while (res != Atomic::cmpxchg(&_top, res, new_top));

    return res;
  }

 private:
  bool _empty;

  volatile intptr_t _top;
  intptr_t _end;
};


#endif // SHARE_GC_SODA_SODABUMPER_HPP
