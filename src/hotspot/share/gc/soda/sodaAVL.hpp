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

#ifndef SHARE_GC_SODA_SODAAVL_HPP
#define SHARE_GC_SODA_SODAAVL_HPP

#include "memory/allocation.inline.hpp"

// 'cmp(k1, k2)' returns 0 if k1 == k2,
// returns 1 if k1 > k2, returns -1 if k1 < k2
template<class Key, class Value, int(*cmp)(Key, Key)>
class SodaAVL {
public:
  class Node;

  class Closure {
  public:
    virtual void do_node(Node*) = 0;
  };

  class Node : CHeapObj<mtGC> {
    friend class SodaAVL;

  private:
    Node(Key k, Value v)
    : _key(k),
      _value(v),
      _bf(0),
      _left(nullptr),
      _right(nullptr),
      _parent(nullptr) {}

  public:
    Key key() { return _key; }

    Value get_value() { return _value; }
    void set_value(Value v) { _value = v; }

  public:
    Node* predecessor() {
      auto iter = _left;
      if (iter == nullptr && _parent != nullptr) {
        if (isLeftChild()) {
          for (iter = _parent; iter != nullptr; iter = iter->_parent) {
            if (iter->_parent != nullptr &&
                iter->isRightChild()) { return iter->_parent; }
          }
        } else { return _parent; }
      }

      while (iter != nullptr) {
        auto n = iter->_right;
        if (n == nullptr) { return iter; }
        iter = n;
      }

      return nullptr;
    }

    Node* successor() {
      auto iter = _right;
      if (iter == nullptr && _parent != nullptr) {
        if (isRightChild()) {
          for (iter = _parent; iter != nullptr; iter = iter->_parent) {
            if (iter->_parent != nullptr &&
                iter->isLeftChild()) { return iter->_parent; }
          }
        } else { return _parent; }
      }

      while (iter != nullptr) {
        auto n = iter->_left;
        if (n == nullptr) { return iter; }
        iter = n;
      }

      return nullptr;
    }

  private:
    void go_left() {
      assert(_right != nullptr, "should have right child");

      auto np = _right;
      _right = _right->_left;
      np->_left = this;

      if (_parent != nullptr){
        if (isLeftChild())
          _parent->_left = np;
        else
          _parent->_right = np;
      }
      np->_parent = _parent;
      _parent = np;

      if (_right != nullptr) {
        _right->_parent = this;

        np->_bf = -1;
        _bf = 1;
      } else {
        np->_bf = 0;
        _bf = 0;
      }
    }

    void go_right() {
      assert(_left != nullptr, "should have left child");

      auto np = _left;
      _left = _left->_right;
      np->_right = this;

      if (_parent != nullptr) {
        if (isLeftChild())
          _parent->_left = np;
        else
          _parent->_right = np;
      }
      np->_parent = _parent;
      _parent = np;

      if (_left != nullptr) {
        _left->_parent = this;

        np->_bf = 1;
        _bf = -1;
      } else {
        np->_bf = 0;
        _bf = 0;
      }
    }

    Node* find_ins_pos(Key k) {
      auto iter = this;
      for (;;) {
        int res = cmp(iter->_key, k);
        Node* n = nullptr;
        if (res > 0) {
          n = iter->_left;
        } else if (res < 0) {
          n = iter->_right;
        } else { return nullptr; }

        if (n == nullptr) { return iter; }
        iter = n;
      }

      ShouldNotReachHere();
      return nullptr;
    }

    Node* find_del_pos() {
      Node* n = nullptr;

      Node* iter = nullptr;
      // try to find an equity leaf node starting from left child
      if (_left != nullptr) {
        iter = _left;
        for (;;) {
          if (iter->_right == nullptr) {
            if (iter->_left == nullptr)
              n = iter;
            break;
          }
          iter = iter->_right;
        }
      }
      // from right child
      if (n == nullptr && _right != nullptr) {
        iter = _right;
        for (;;) {
          if (iter->_left == nullptr) {
            if (iter->_right == nullptr)
              n = iter;
            break;
          }
          iter = iter->_right;
        }
      }

      return n;
    }

    bool isLeftChild() {
      assert(_parent != nullptr, "should have parent");

      return this == _parent->_left ? true : false;
    }

    bool isRightChild() { return !isLeftChild(); }

    static void balance(Node* begin, int delta_bf) {
      auto iter = begin;
      auto p = iter->_parent;

      bool trigger = false;
      while (p != nullptr) {
        if (iter->isLeftChild()) {
          p->_bf -= delta_bf;
        } else {
          p->_bf += delta_bf;
        }

        if (p->_bf == -2 || p->_bf == 2) {
          trigger = true;
          break;
        }

        iter = p;
        p = p->_parent;
      }

      if (trigger) {
        if (p->_bf == -2) {
          if (iter->_bf == 1) {
            iter->go_left();
          }
          p->go_right();
        } else {
          if (iter->_bf == -1) {
            iter->go_right();
          }
          p->go_left();
        }
      }
    }

  private:
    void erase() {
      auto n = find_del_pos();

      if (n == nullptr)
        n = this;
      else {
        this->_key = n->_key;
        this->_value = n->_value;
      }

      balance(n, -1);

      if (n->isLeftChild())
        n->_parent->_left = nullptr;
      else
        n->_parent->_right = nullptr;
    }

    Node* find(Key k) {
      auto iter = this;
      while (iter != nullptr) {
        int res = cmp(iter->_key, k);
        if (res > 0) {
          iter = iter->_left;
        } else if (res < 0) {
          iter = iter->_right;
        } else { return iter; }
      }

      return nullptr;
    }

    Node* find_equal_or_successor(Key k) {
      auto iter = this;
      Node* gte = nullptr;

      while (iter != nullptr) {
        int res = cmp(iter->_key, k);
        if (res > 0) {
          gte = iter;
          iter = iter->_left;
        } else if (res < 0) {
          iter = iter->_right;
        } else { return iter; }
      }

      return gte;
    }

  private:
    void iterate_preorder(Closure* cl) {
      cl->do_node(this);
      if (_left != nullptr) { _left->iterate_preorder(cl); }
      if (_right != nullptr) { _right->iterate_preorder(cl); }
    }

    void iterate_inorder(Closure* cl) {
      if (_left != nullptr) { _left->iterate_inorder(cl); }
      cl->do_node(this);
      if (_right != nullptr) { _right->iterate_inorder(cl); }
    }

    void iterate_postorder(Closure* cl) {
      if (_left != nullptr) { _left->iterate_postorder(cl); }
      if (_right != nullptr) { _right->iterate_postorder(cl); }
      cl->do_node(this);
    }

  private:
    Key _key;
    Value _value;

  private:
    int _bf;

    Node* _left;
    Node* _right;
    Node* _parent;
  };

public:
  SodaAVL()
  : _root(nullptr),
    _size(0),
    _cache(nullptr) {}

public:
  size_t size() { return _size; }

  void insert(Key k, Value v) {
    if (_root == nullptr)
      _root = new Node(k, v);
    else {
      auto pos = _root->find_ins_pos(k);
      if (pos == nullptr)
        return;

      auto n = new Node(k, v);
      n->_parent = pos;
      if (cmp(pos->_key, k) > 0)
        pos->_left = n;
      else
        pos->_right = n;

      Node::balance(n, 1);

      if (_root->_parent != nullptr)
        _root = _root->_parent;

      _cache = n;
    }

    _size += 1;
  }

  void erase(Key k) {
    if (_size == 1 && cmp(_root->_key, k) == 0) {
      delete _root;
      _root = nullptr;
      _cache = nullptr;

      _size = 0;
      return;
    }

    auto n = find(k);
    if (n != nullptr) {
      _cache = nullptr;

      n->erase();
      if (_root->_parent != nullptr)
        _root = _root->_parent;

      _size -= 1;
    }
  }

  Node* find(Key k) {
    if (_root == nullptr) { return nullptr; }
    if (check_cache(k)) { return _cache; }

    auto res = _root->find(k);
    if (res != nullptr) { _cache = res; }

    return res;
  }

  Node* find_equal_or_successor(Key k) {
    if (_root == nullptr) { return nullptr; }
    if (check_cache(k)) { return _cache; }

    auto res = _root->find_equal_or_successor(k);
    if (res != nullptr) { _cache = res; }

    return res;
  }

  void iterate_preorder(Closure* cl) {
    if (_root != nullptr) { _root->iterate_preorder(cl); }
  }

  void iterate_inorder(Closure* cl) {
    if (_root != nullptr) { _root->iterate_inorder(cl); }
  }

  void iterate_postorder(Closure* cl) {
    if (_root != nullptr) { _root->iterate_postorder(cl); }
  }

  // before deletion, iterate all entries in post-order and do cl to them
  void clear(Closure* cl) {
    if (_root == nullptr) return;
    _cache = nullptr;

    struct DelClosure : public Closure {
      void do_node(Node* n) override {
        cl->do_node(n);
        delete n;
      }
    } _cl;

    iterate_postorder(&_cl);

    _root = nullptr;
    _size = 0;
  }

private:
  bool check_cache(Key k) {
    if (_cache == nullptr) { return false; }
    return cmp(_cache->key(), k) == 0;
  }

private:
  Node* _root;
  size_t _size;

  Node* _cache;
};

#endif // SHARE_GC_SODA_SODAAVL_HPP
