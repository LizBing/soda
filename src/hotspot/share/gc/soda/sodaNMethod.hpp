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

#ifndef SHARE_GC_SODA_SODANMETHOD_HPP
#define SHARE_GC_SODA_SODANMETHOD_HPP

#include "code/nmethod.hpp"
#include "gc/shared/scavengableNMethods.hpp"
#include "memory/allStatic.hpp"
#include "runtime/mutexLocker.hpp"

// serial for now
struct SodaNMethodTable : AllStatic {
  static void initialize();

  static void register_nmethod(nmethod* nm) {
    ScavengableNMethods::register_nmethod(nm);
  }

  static void unregister_nmethod(nmethod* nm) {
    ScavengableNMethods::unregister_nmethod(nm);
  }

  static void iterate(CodeBlobToOopClosure* cl) {
    MutexLocker ml(CodeCache_lock);

    ScavengableNMethods::nmethods_do(cl);
  }
};

#endif // SHARE_GC_SODA_SODANMETHOD_HPP
