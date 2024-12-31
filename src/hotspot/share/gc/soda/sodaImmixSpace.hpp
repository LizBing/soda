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

#ifndef SHARE_GC_SODA_SODAIMMIXSPACE_HPP
#define SHARE_GC_SODA_SODAIMMIXSPACE_HPP

#include "gc/parallel/mutableSpace.hpp"
#include "gc/soda/sodaGlobals.hpp"
#include "gc/soda/sodaLineCardTable.hpp"
#include "memory/allocation.inline.hpp"

// Note that the committed size of the old generation does not
// vary until a major gc or allocation with expansion occurs.

class SodaImmixSpace: public MutableSpace {
public:
  SodaImmixSpace(size_t page_size):
    MutableSpace(page_size),
    _constructed(false) {}

  void initialize(MemRegion mr,
                  bool clear_space,
                  bool mangle_space,
                  bool setup_pages = SetupPages,
                  WorkerThreads *pretouch_workers = nullptr) override;

public:
  void reset_constructed() { _constructed = false; }
  bool constructed() { return _constructed; }

public:
  void construct();

private:
  bool _constructed;

  SodaLineCardTable* _lct;
};


#endif // SHARE_GC_SODA_SODAIMMIXSPACE_HPP
