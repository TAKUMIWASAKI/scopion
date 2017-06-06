#ifndef SCOPION_AST_EXPR_H_
#define SCOPION_AST_EXPR_H_

#include "scopion/ast/value.h"

#include <boost/variant.hpp>

namespace scopion {
namespace ast {

struct add;
struct sub;
struct mul;
struct div;
struct rem;
struct shl;
struct shr;
struct iand;
struct ior;
struct ixor;
struct land;
struct lor;
struct eeq;
struct neq;
struct gt;
struct lt;
struct gtq;
struct ltq;
struct assign;
struct call;
struct at;

struct ret;
struct load;
struct lnot;
struct inot;
struct inc;
struct dec;

template <class Op> struct single_op;
template <class Op> struct binary_op;

using expr_base =
    boost::variant<value, boost::recursive_wrapper<binary_op<add>>,
                   boost::recursive_wrapper<binary_op<sub>>,
                   boost::recursive_wrapper<binary_op<mul>>,
                   boost::recursive_wrapper<binary_op<div>>,
                   boost::recursive_wrapper<binary_op<rem>>,
                   boost::recursive_wrapper<binary_op<shl>>,
                   boost::recursive_wrapper<binary_op<shr>>,
                   boost::recursive_wrapper<binary_op<iand>>,
                   boost::recursive_wrapper<binary_op<ior>>,
                   boost::recursive_wrapper<binary_op<ixor>>,
                   boost::recursive_wrapper<binary_op<land>>,
                   boost::recursive_wrapper<binary_op<lor>>,
                   boost::recursive_wrapper<binary_op<eeq>>,
                   boost::recursive_wrapper<binary_op<neq>>,
                   boost::recursive_wrapper<binary_op<gt>>,
                   boost::recursive_wrapper<binary_op<lt>>,
                   boost::recursive_wrapper<binary_op<gtq>>,
                   boost::recursive_wrapper<binary_op<ltq>>,
                   boost::recursive_wrapper<binary_op<assign>>,
                   boost::recursive_wrapper<single_op<ret>>,
                   boost::recursive_wrapper<binary_op<call>>,
                   boost::recursive_wrapper<binary_op<at>>,
                   boost::recursive_wrapper<single_op<load>>,
                   boost::recursive_wrapper<single_op<lnot>>,
                   boost::recursive_wrapper<single_op<inot>>,
                   boost::recursive_wrapper<single_op<inc>>,
                   boost::recursive_wrapper<single_op<dec>>>;

struct expr : expr_base {
  using expr_base::expr_base;
  using expr_base::operator=;

  expr(expr_base const &ex) : expr_base(ex) {}
};
bool operator==(expr const &lhs, expr const &rhs);

}; // namespace ast
}; // namespace scopion

#endif