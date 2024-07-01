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

public:
  bool empty() { return _empty; }
  void set_empty() { _empty = true; }

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
    intptr_t des = 0;

    do {
      res = Atomic::load(&_top);
      des = res + size;

      if (des > _end) return 0;
    } while (!Atomic::cmpxchg(&_top, res, des));

    return res;
  }

 private:
  bool _empty;

  volatile intptr_t _top;
  intptr_t _end;
};


#endif // SHARE_GC_SODA_SODABUMPER_HPP
