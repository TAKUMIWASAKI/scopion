#ifndef SCOPION_AST_UTIL_H_
#define SCOPION_AST_UTIL_H_

#include "scopion/ast/attribute.hpp"
#include "scopion/ast/expr.hpp"
#include "scopion/ast/value_wrapper.hpp"

#include <boost/variant.hpp>

#include <string>

namespace scopion {
namespace ast {

struct set_lval_visitor : boost::static_visitor<expr> {
  const bool v;

  set_lval_visitor(bool v_) : v(v_) {}

  template <typename T> expr operator()(T val) const {
    attr(val).lval = v;
    return val;
  }

  ast::expr operator()(expr val) const {
    return boost::apply_visitor(*this, val);
  }

  ast::expr operator()(value val) const {
    return boost::apply_visitor(*this, val);
  }

  ast::expr operator()(operators val) const {
    return boost::apply_visitor(*this, val);
  }
};

struct set_to_call_visitor : boost::static_visitor<expr> {
  const bool v;

  set_to_call_visitor(bool v_) : v(v_) {}

  template <typename T> expr operator()(T val) const {
    attr(val).to_call = v;
    return val;
  }

  ast::expr operator()(expr val) const {
    return boost::apply_visitor(*this, val);
  }

  ast::expr operator()(value val) const {
    return boost::apply_visitor(*this, val);
  }

  ast::expr operator()(operators val) const {
    return boost::apply_visitor(*this, val);
  }
};

struct set_attr_visitor : boost::static_visitor<expr> {
  std::string const &key;
  std::string const &val;

  set_attr_visitor(std::string const &key_, std::string const &val_)
      : key(key_), val(val_) {}

  template <typename T> expr operator()(T value) const {
    attr(value).attributes[key] = val;
    return value;
  }

  ast::expr operator()(expr val) const {
    return boost::apply_visitor(*this, val);
  }

  ast::expr operator()(value val) const {
    return boost::apply_visitor(*this, val);
  }

  ast::expr operator()(operators val) const {
    return boost::apply_visitor(*this, val);
  }
};

template <typename T> T &val(value_wrapper<T> &w) { return w.value; }

template <typename T> const T &val(value_wrapper<T> const &w) {
  return w.value;
}

template <typename T> attribute &attr(T &w) { return w.attr; }
template <typename T> const attribute &attr(T const &w) { return w.attr; }

template <typename T> expr set_lval(T t, bool val) {
  return set_lval_visitor(val)(t);
}

template <typename T> expr set_to_call(T t, bool val) {
  return set_to_call_visitor(val)(t);
}

template <typename T>
expr set_attr(T t, std::string const &key, std::string const &val) {
  return set_attr_visitor(key, val)(t);
}

template <typename T, typename RangeT> T set_where(T val, RangeT range) {
  attr(val).where = range;
  return val;
}

}; // namespace ast
}; // namespace scopion

#endif
