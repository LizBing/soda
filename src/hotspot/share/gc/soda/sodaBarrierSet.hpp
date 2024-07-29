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

#ifndef SHARE_GC_SODA_SODABARRIERSET_HPP
#define SHARE_GC_SODA_SODABARRIERSET_HPP

#include "gc/shared/barrierSet.hpp"
#include "gc/shared/cardTableBarrierSet.inline.hpp"

class SodaBarrierSet: public CardTableBarrierSet {
  friend class VMStructs;

public:
  SodaBarrierSet(CardTable*);

  static SodaBarrierSet* barrier_set() {
    return barrier_set_cast<SodaBarrierSet>(BarrierSet::barrier_set());
  }


  virtual void on_thread_create(Thread* thread);
  virtual void on_thread_destroy(Thread* thread);

  template <DecoratorSet decorators, typename BarrierSetT = SodaBarrierSet>
  class AccessBarrier: public BarrierSet::AccessBarrier<decorators, BarrierSetT> {};
};

template<>
struct BarrierSet::GetName<SodaBarrierSet> {
  static const BarrierSet::Name value = BarrierSet::SodaBarrierSet;
};

template<>
struct BarrierSet::GetType<BarrierSet::SodaBarrierSet> {
  typedef ::SodaBarrierSet type;
};

#endif // SHARE_GC_SODA_SODABARRIERSET_HPP
