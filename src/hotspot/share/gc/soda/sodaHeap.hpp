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

#ifndef SHARE_GC_SHARED_SODA_SODAHEAP_HPP
#define SHARE_GC_SHARED_SODA_SODAHEAP_HPP

#include "gc/parallel/parallelScavengeHeap.inline.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/memRegion.hpp"

// Soda manages the old generation of Parallel.
class SodaHeap : public CHeapObj<mtGC> {
public:
  static SodaHeap* heap() { return _heap; }

  static jint initialize();

private:
  static SodaHeap* _heap;
  static ParallelScavengeHeap* _psh;

public:
  MemRegion reserved() {
    return _psh->old_gen()->reserved();
  }

  MemRegion committed() {
    return _psh->old_gen()->committed();
  }
};


#endif // SHARE_GC_SHARED_SODA_SODAHEAP_HPP
