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

#include "gc/soda/sodaGlobals.hpp"
#include "gc/soda/sodaImmixSpace.hpp"

void SodaImmixSpace::initialize(
  MemRegion mr,
  bool clear_space,
  bool mangle_space,
  bool setup_pages,
  WorkerThreads *pretouch_workers)
{
  MutableSpace::initialize(mr, clear_space, mangle_space, setup_pages, pretouch_workers);

  _lct = new SodaLineCardTable(mr);
}

void SodaImmixSpace::construct() {
  assert(!constructed(), "should not be constructed before reset");
  _constructed = true;

  // ...
}
