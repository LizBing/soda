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
#include "gc/soda/sodaHeap.hpp"
#include "gc/soda/sodaInitLogger.hpp"
#include "gc/shared/tlab_globals.hpp"
#include "logging/log.hpp"
#include "runtime/globals.hpp"
#include "runtime/globals_extension.hpp"
#include "utilities/globalDefinitions.hpp"

void SodaInitLogger::print_gc_specific() {
  auto heap = SodaHeap::heap();

  log_info(gc)(
    "Heap block size: " SIZE_FORMAT " bytes, "
    "Block line size: " SIZE_FORMAT " bytes.",
    heap->block_size(),
    heap->line_size()
  );

  log_info(gc)("Soda uses immix-constructed heap.");
}

void SodaInitLogger::print() {
  SodaInitLogger init_log;
  init_log.print_all();
}
