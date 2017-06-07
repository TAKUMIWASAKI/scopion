#ifndef SCOPION_AST_H_
#define SCOPION_AST_H_

#include "scopion/ast/attribute.hpp"
#include "scopion/ast/expr.hpp"
#include "scopion/ast/operators.hpp"
#include "scopion/ast/util.hpp"
#include "scopion/ast/value.hpp"
#include "scopion/ast/value_wrapper.hpp"

#include <iostream>

std::ostream &operator<<(std::ostream &os, scopion::ast::expr const &tree);

#endif // SCOPION_AST_H_