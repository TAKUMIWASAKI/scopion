#include "scopion/ast/ast.h"

namespace scopion {
namespace ast {

template <typename Tc> struct compare_expr : boost::static_visitor<bool> {
public:
  Tc with;
  compare_expr(Tc const &_with) : with(_with) {}

  bool operator()(ast::expr_base exp) const {
    return boost::apply_visitor(*this, exp);
  }

  template <typename Op> bool operator()(ast::binary_op<Op> op) const {
    if (with.type() != typeid(op))
      return false;
    compare_expr<ast::expr_base> nr(boost::get<ast::binary_op<Op>>(with).rhs);
    compare_expr<ast::expr_base> nl(boost::get<ast::binary_op<Op>>(with).lhs);
    return boost::apply_visitor(nr, op.rhs) && boost::apply_visitor(nl, op.lhs);
  }

  bool operator()(ast::value value) const {
    if (with.type() != typeid(value))
      return false;
    compare_expr<ast::value> n(boost::get<ast::value>(with));
    return boost::apply_visitor(n, value);
  }

  template <typename T> bool operator()(T value) const {
    if (with.type() != typeid(value))
      return false;
    return value == boost::get<T>(with);
  }
};

bool operator==(expr const &lhs, expr const &rhs) {
  return boost::apply_visitor(compare_expr<expr>(lhs), rhs);
}
bool operator==(variable const &lhs, variable const &rhs) {
  return (lhs.name == rhs.name) && (lhs.lval == rhs.lval) &&
         (lhs.isFunc == rhs.isFunc);
}
bool operator==(array const &lhs, array const &rhs) {
  return (lhs.elements == rhs.elements);
}
bool operator==(function const &lhs, function const &rhs) {
  return (lhs.lines == rhs.lines);
}
template <class Op>
bool operator==(binary_op<Op> const &lhs, binary_op<Op> const &rhs) {
  return (lhs.rhs == rhs.rhs) && (lhs.lhs == rhs.lhs) && (lhs.lval == rhs.lval);
}
} // namespace ast
} // namespace scopion