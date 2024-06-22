/*
 * Copyright (c) 2024, Lei Zaakjyu. All rights reserved.
 * Copyright (c) 2024, Oracle and/or its affiliates. All rights reserved.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "gc/soda/sodaFreeLineTable.hpp"
#include "gc/soda/sodaHeapBlock.hpp"

intptr_t SodaFreeLineTable::_heap_start = 0;
CardValue* SodaFreeLineTable::_cards = nullptr;

bool SodaFreeLineDiscoverer::discover(Closure* cl) {
  auto ccv = SodaFreeLineTable::clean_card_value();

  for (; _iter != _end;) {
    if (*_iter == ccv) {
      auto tmp = _iter;
/*
      while (
        _iter < _end &&
        *(++_iter) == ccv
      );
*/
      for (;;) {
        ++_iter;
        if (_iter == _end) break;
        if (*_iter != ccv) break;
      }

      MemRegion mr(
        (HeapWord*)SodaFreeLineTable::addr_for(tmp),
        (HeapWord*)SodaFreeLineTable::addr_for(_iter)
      );

      if (!cl->do_free_line(mr))
        return false;
    } else ++_iter;
  }

  return true;
}

