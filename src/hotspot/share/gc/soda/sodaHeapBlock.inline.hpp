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

#ifndef SHARE_GC_SODA_SODAHEAPBLOCK_INLINE_HPP
#define SHARE_GC_SODA_SODAHEAPBLOCK_INLINE_HPP

#include "gc/soda/sodaHeapBlock.hpp"
#include "gc/soda/sodaHeapBlockSet.hpp"

inline SodaHeapBlock* SodaHeapBlock::cont_next() {
  auto idx = index() + _blocks;
  return idx == SodaHeapBlocks::size() ?
                nullptr :
                SodaHeapBlocks::at(idx);
}

inline SodaHeapBlock* SodaHeapBlock::cont_prev() {
  auto idx = index();
  return idx == 0 ? nullptr : SodaHeapBlocks::at(idx - 1)->_header;
}

inline SodaHeapBlock* SodaHeapBlock::last() {
  return SodaHeapBlocks::at(index() + _blocks - 1);
}

inline uintx SodaHeapBlock::index() {
  return ((intptr_t)this - (intptr_t)SodaHeapBlocks::_blocks) /
         sizeof(SodaHeapBlock);
}

#endif // SHARE_GC_SODA_SODAHEAPBLOCK_INLINE_HPP
