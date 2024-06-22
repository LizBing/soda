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

#ifndef SHARE_GC_SODA_SODATHREADLOCALDATA_HPP
#define SHARE_GC_SODA_SODATHREADLOCALDATA_HPP

#include "gc/soda/sodaTLAB.hpp"
#include "gc/shared/gc_globals.hpp"
#include "runtime/javaThread.hpp"
#include "utilities/debug.hpp"

class SodaThreadLocalData {
private:
  SodaTLAB _tlab;

private:
  SodaThreadLocalData() = default;

  static SodaThreadLocalData* data(Thread* thread) {
    assert(UseSodaGC, "Sanity");
    return thread->gc_data<SodaThreadLocalData>();
  }

public:
  static void create(Thread* thread) {
    new (data(thread)) SodaThreadLocalData();
  }

  static void destroy(Thread* thread) {
    data(thread)->~SodaThreadLocalData();
  }

  static SodaTLAB* tlab(Thread* thread) { return &data(thread)->_tlab; }
};

#endif // SHARE_GC_SODA_SODATHREADLOCALDATA_HPP
