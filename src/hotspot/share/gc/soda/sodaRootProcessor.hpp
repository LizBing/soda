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

#ifndef SHARE_GC_SODA_SODAROOTPROCESSOR_HPP
#define SHARE_GC_SODA_SODAROOTPROCESSOR_HPP

#include "gc/shared/workerThread.hpp"
#include "memory/allStatic.hpp"
#include "memory/iterator.inline.hpp"

struct SodaRootClosure {
  // Closures to process raw oops in the root set.
  virtual OopClosure* strong_oops() = 0;

  // Closures to process CLDs in the root set.
  virtual CLDClosure* weak_clds() = 0;
  virtual CLDClosure* strong_clds() = 0;

  // Applied to code blobs reachable as strong roots.
  virtual CodeBlobClosure* strong_codeblobs() = 0;
};

class SodaRootProcessor : AllStatic {
public:
  void process(WorkerThreads*, SodaRootClosure*);

private:
};


#endif // SHARE_GC_SODA_SODAROOTPROCESSOR_HPP
