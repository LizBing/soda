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

#ifndef SHARE_GC_SODA_SODALINECARDTABLE_HPP
#define SHARE_GC_SODA_SODALINECARDTABLE_HPP

#include "gc/soda/sodaGlobals.hpp"
#include "gc/shared/cardTable.hpp"
#include "memory/allocation.hpp"
#include "memory/memRegion.hpp"

class SodaLineCardTable: public CHeapObj<mtGC> {
  using card_t = uint8_t;

private:
  size_t compute_card_size(size_t heap_size) {
    return heap_size >> SodaGlobals::log_line_size;
  }

public:
  SodaLineCardTable(MemRegion heap_mr): _heap_mr(heap_mr) {}

  void initialize();

private:
  enum CardValues {
    _clean_card = (card_t)-1,
    _dirty_card = 0,
  };

private:
  MemRegion _heap_mr;

  size_t _card_size;
  card_t* _cards;
};


#endif // SHARE_GC_SODA_SODALINECARDTABLE_HPP
