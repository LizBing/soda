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
#include "gc/soda/sodaArguments.hpp"
#include "gc/soda/sodaHeap.hpp"
#include "gc/shared/gcArguments.hpp"
#include "gc/shared/tlab_globals.hpp"
#include "logging/log.hpp"
#include "runtime/globals.hpp"
#include "runtime/globals_extension.hpp"

size_t SodaArguments::conservative_max_heap_alignment() {
  return UseLargePages ? os::large_page_size() : os::vm_page_size();
}

void SodaArguments::initialize() {
  GCArguments::initialize();

  assert(UseSodaGC, "Sanity");

  // Forcefully exit when OOME is detected. Nothing we can do at that point.
  if (FLAG_IS_DEFAULT(ExitOnOutOfMemoryError)) {
    FLAG_SET_DEFAULT(ExitOnOutOfMemoryError, true);
  }

  FLAG_SET_DEFAULT(UseTLAB, false);
}

void SodaArguments::initialize_alignments() {
  size_t block_size = DEFAULT_CACHE_LINE_SIZE *
                      SodaCacheLinesPerBlockLine *
                      SodaLinesPerHeapBlock;
  SpaceAlignment = MAX3(block_size, os::vm_page_size(), os::large_page_size());
  HeapAlignment  = SpaceAlignment;
}

CollectedHeap* SodaArguments::create_heap() {
  return new SodaHeap();
}
