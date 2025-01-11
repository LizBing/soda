/*
 * Copyright (c) 2025, Lei Zaakjyu. All rights reserved.
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

#ifndef SHARE_SODA_SODAFLEXIBLELIST_HPP
#define SHARE_SODA_SODAFLEXIBLELIST_HPP

#include "gc/z/zAddress.hpp"
#include "memory/allocation.hpp"
#include "utilities/lockFreeStack.hpp"

class SodaFlexibleListNode: StackObj {
  friend class SodaLinkedList;

public:
  static SodaFlexibleListNode* volatile*
  next_ptr(SodaFlexibleListNode& n) { return &n._next_vol; }

public:
  void insert(SodaFlexibleListNode* n) {
    assert(n != nullptr, "unnullable");

    n->_next = _next;
    n->_prev = this;

    _next->_prev = n;
    _next = n;
  }

  void erase() {
    _prev->_next = _next;
    _next->_prev = _prev;
  }

private:
  union {
    SodaFlexibleListNode* volatile _next_vol;
    SodaFlexibleListNode* _next;
  };

  SodaFlexibleListNode* _prev;
};

struct SodaLListClosure: StackObj {
  // Return false to stop iteration.
  virtual bool do_node(SodaFlexibleListNode*) = 0;
};

class SodaLinkedList: SodaFlexibleListNode {
  using LFS = LockFreeStack<SodaFlexibleListNode, SodaFlexibleListNode::next_ptr>;

public:
  SodaLinkedList() { clear(); }

public:
  SodaFlexibleListNode* first() { return _next; }
  SodaFlexibleListNode* last()  { return _prev; }

  // After moving, this ll would be cleared.
  void move_to_lfs(LFS* lfs) {
    auto f = first();
    auto l = last();
    l->_next = nullptr;

    lfs->prepend(*f, *l);

    clear();
  }

  bool empty() { return first() == last(); }

  void append(SodaFlexibleListNode* n) { insert(n); }

  void push(SodaFlexibleListNode* n)  { last()->insert(n); }

  SodaFlexibleListNode* dequeue() {
    if (empty()) return nullptr;

    auto n = first();
    n->erase();

    return n;
  }

  SodaFlexibleListNode* pop() {
    if (empty()) return nullptr;

    auto n = last();
    n->erase();

    return n;
  }

  void clear() {
    _prev = this;
    _next = this;
  }

  bool iterate(SodaLListClosure* cl) {
    for (auto n = first(); n != this; n = n->_next)
      if (!cl->do_node(n)) return false;

    return true;
  }

  bool iterate_rev(SodaLListClosure* cl) {
    for (auto n = last(); n != this; n = n->_prev)
      if (!cl->do_node(n)) return false;

    return true;
  }
};


#endif // SHARE_SODA_SODAFLEXIBLELIST_HPP
