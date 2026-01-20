#pragma once

#include <vector>
#include <iostream>
#include <variant>
#include <optional>
#include "./arenaAllocator.hpp"

enum class DataType
{
  Int,
  Char,
  Bool,
};

enum class UnaryOp
{
  Negate,
  Not,
};

struct NodeExpr;

struct NodeBinExprEq
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprNeq
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprLt
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprGt
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprAnd
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprOr
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprLte
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprGte
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprMod
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprAdd
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprSub
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprDiv
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprMul
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeTerm;

struct NodeTermUnary
{
  UnaryOp op;
  NodeTerm *operand;
};

struct NodeTermLit
{
  Token token;
};

struct NodeTermIdent
{
  Token ident;
};

struct NodeTermParen
{
  NodeExpr *expr;
};
struct NodeTerm
{
  std::variant<NodeTermLit *, NodeTermIdent *, NodeTermParen *, NodeTermUnary *> val;
};

struct NodeBinExpr
{
  std::variant<NodeBinExprAdd *, NodeBinExprMul *, NodeBinExprSub *, NodeBinExprDiv *, NodeBinExprMod *, NodeBinExprEq *, NodeBinExprGt *, NodeBinExprNeq *, NodeBinExprLte *, NodeBinExprGte *, NodeBinExprLt *, NodeBinExprAnd *, NodeBinExprOr *> op;
};

struct NodeExpr
{
  std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit
{
  NodeExpr *expr;
};

struct NodeStmtConst
{
  Token ident;
  DataType dtype;
  NodeExpr *expr;
};

struct NodeStmtAssign
{
  Token ident;
  NodeExpr *expr;
};
struct NodeStmtLet
{
  Token ident;
  DataType dtype;
  std::optional<NodeExpr *> expr;
};

struct NodeStmt;

struct NodeStmtPrint
{
  NodeExpr *expr;
};
struct NodeStmtScope
{
  std::vector<NodeStmt *> stmts;
};

struct NodeStmtIfCont;

struct NodeStmtElse
{
  NodeStmtScope *scope;
};
struct NodeStmtElif
{
  NodeExpr *expr;
  NodeStmtScope *scope;
  std::optional<NodeStmtIfCont *> cont;
};
struct NodeStmtIfCont
{
  std::variant<NodeStmtElse *, NodeStmtElif *> clause;
};

struct NodeStmtIf
{
  NodeExpr *expr;
  NodeStmtScope *scope;
  std::optional<NodeStmtIfCont *> cont;
};
struct NodeStmt
{
  std::variant<NodeStmtExit *, NodeStmtConst *, NodeStmtScope *, NodeStmtPrint *, NodeStmtIf *, NodeStmtLet *, NodeStmtAssign *> stmt;
};

struct NodeProg
{
  std::vector<NodeStmt *> stmts;
};

class Parser
{
public:
  explicit Parser(std::vector<Token> token_vec)
      : tokens(std::move(token_vec)), allocator(1024 * 1024 * 4) {}

  std::optional<NodeTerm *> parse_term(bool allow_unary = true)
  {
    if (auto int_lit_token = try_consume(TokenType::int_lit))
    {
      auto *node_term = allocator.alloc<NodeTerm>();
      auto *node_lit = allocator.alloc<NodeTermLit>();
      node_lit->token = int_lit_token.value();
      node_term->val = node_lit;
      return node_term;
    }
    if (auto char_lit_token = try_consume(TokenType::char_lit))
    {
      auto *node_term = allocator.alloc<NodeTerm>();
      auto *node_lit = allocator.alloc<NodeTermLit>();
      node_lit->token = char_lit_token.value();
      node_term->val = node_lit;
      return node_term;
    }
    if (auto bool_lit_token = try_consume(TokenType::bool_lit))
    {
      auto *node_term = allocator.alloc<NodeTerm>();
      auto *node_lit = allocator.alloc<NodeTermLit>();
      node_lit->token = bool_lit_token.value();
      node_term->val = node_lit;
      return node_term;
    }
    else if (auto minus_token = try_consume(TokenType::sub))
    {
      if (allow_unary == false)
      {
        std::cerr << "Expected term but got minus\n";
        std::exit(EXIT_FAILURE);
      }
      auto operand = parse_term(false);
      if (!operand.has_value())
      {
        std::cerr << "Expected term after unary minus\n";
        std::exit(EXIT_FAILURE);
      }
      auto *node_unary = allocator.alloc<NodeTermUnary>();
      node_unary->op = UnaryOp::Negate;
      node_unary->operand = operand.value();

      auto *node_term = allocator.alloc<NodeTerm>();
      node_term->val = node_unary;
      return node_term;
    }
    else if (auto not_token = try_consume(TokenType::not_))
    {
      if (allow_unary == false)
      {
        std::cerr << "Expected term but got minus\n";
        std::exit(EXIT_FAILURE);
      }
      auto operand = parse_term(false);
      if (!operand.has_value())
      {
        std::cerr << "Expected term after unary minus\n";
        std::exit(EXIT_FAILURE);
      }
      auto *node_unary = allocator.alloc<NodeTermUnary>();
      node_unary->op = UnaryOp::Not;
      node_unary->operand = operand.value();

      auto *node_term = allocator.alloc<NodeTerm>();
      node_term->val = node_unary;
      return node_term;
    }
    else if (auto ident_token = try_consume(TokenType::ident))
    {
      auto *node_term = allocator.alloc<NodeTerm>();
      auto *node_ident = allocator.alloc<NodeTermIdent>();
      node_ident->ident = ident_token.value();
      node_term->val = node_ident;
      return node_term;
    }
    else if (auto open_paren = try_consume(TokenType::open_paren))
    {
      auto node_expr = parse_expr();
      if (node_expr.has_value())
      {
        if (auto close_paren = try_consume(TokenType::close_paren))
        {
          auto term_paren = allocator.alloc<NodeTermParen>();
          term_paren->expr = node_expr.value();
          auto node_term = allocator.alloc<NodeTerm>();
          node_term->val = term_paren;
          return node_term;
        }
        else
        {
          std::cerr << "Expected close parenthesis\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }

    return std::nullopt;
  }

  std::optional<NodeExpr *> parse_expr(int min_prec = 0, bool allow_unary = true)
  {
    std::optional<NodeTerm *> term_lhs = parse_term(allow_unary);
    if (!term_lhs.has_value())
    {
      return std::nullopt;
    }

    auto expr_lhs = allocator.alloc<NodeExpr>();
    expr_lhs->var = term_lhs.value();

    while (true)
    {
      std::optional<Token> curr_tok = peek();
      int prec;
      if (curr_tok.has_value())
      {
        prec = get_precedence(curr_tok->type);
        if (prec < min_prec)
        {
          break;
        }
      }
      else
      {
        std::cerr << "Expected semi\n";
        std::exit(EXIT_FAILURE);
      }
      Token op = consume();
      int next_min_prec = prec + 1;
      auto expr_rhs = parse_expr(next_min_prec, false);
      if (!expr_rhs.has_value())
      {
        std::cerr << "Unable to parse expression" << std::endl;
        exit(EXIT_FAILURE);
      }
      auto bin_expr = allocator.alloc<NodeBinExpr>();
      if (op.type == TokenType::plus)
      {
        auto bin_expr_add = allocator.alloc<NodeBinExprAdd>();
        bin_expr_add->lhs = expr_lhs;
        bin_expr_add->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_add;
      }
      else if (op.type == TokenType::mul)
      {
        auto bin_expr_mul = allocator.alloc<NodeBinExprMul>();
        bin_expr_mul->lhs = expr_lhs;
        bin_expr_mul->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_mul;
      }
      else if (op.type == TokenType::sub)
      {
        auto bin_expr_sub = allocator.alloc<NodeBinExprSub>();
        bin_expr_sub->lhs = expr_lhs;
        bin_expr_sub->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_sub;
      }
      else if (op.type == TokenType::div)
      {
        auto bin_expr_div = allocator.alloc<NodeBinExprDiv>();
        bin_expr_div->lhs = expr_lhs;
        bin_expr_div->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_div;
      }
      else if (op.type == TokenType::mod)
      {
        auto bin_expr_mod = allocator.alloc<NodeBinExprMod>();
        bin_expr_mod->lhs = expr_lhs;
        bin_expr_mod->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_mod;
      }
      else if (op.type == TokenType::eq)
      {
        auto bin_expr_eq = allocator.alloc<NodeBinExprEq>();
        bin_expr_eq->lhs = expr_lhs;
        bin_expr_eq->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_eq;
      }
      else if (op.type == TokenType::neq)
      {
        auto bin_expr_neq = allocator.alloc<NodeBinExprNeq>();
        bin_expr_neq->lhs = expr_lhs;
        bin_expr_neq->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_neq;
      }
      else if (op.type == TokenType::lt)
      {
        auto bin_expr_lt = allocator.alloc<NodeBinExprLt>();
        bin_expr_lt->lhs = expr_lhs;
        bin_expr_lt->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_lt;
      }
      else if (op.type == TokenType::gt)
      {
        auto bin_expr_gt = allocator.alloc<NodeBinExprGt>();
        bin_expr_gt->lhs = expr_lhs;
        bin_expr_gt->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_gt;
      }
      else if (op.type == TokenType::lte)
      {
        auto bin_expr_lte = allocator.alloc<NodeBinExprLte>();
        bin_expr_lte->lhs = expr_lhs;
        bin_expr_lte->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_lte;
      }
      else if (op.type == TokenType::gte)
      {
        auto bin_expr_gte = allocator.alloc<NodeBinExprGte>();
        bin_expr_gte->lhs = expr_lhs;
        bin_expr_gte->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_gte;
      }
      else if (op.type == TokenType::and_)
      {
        auto bin_expr_and = allocator.alloc<NodeBinExprAnd>();
        bin_expr_and->lhs = expr_lhs;
        bin_expr_and->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_and;
      }
      else if (op.type == TokenType::or_)
      {
        auto bin_expr_or = allocator.alloc<NodeBinExprOr>();
        bin_expr_or->lhs = expr_lhs;
        bin_expr_or->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_or;
      }
      else
      {
        std::cerr << "Unexpected operation" << std::endl;
        exit(EXIT_FAILURE);
      }

      auto new_expr = allocator.alloc<NodeExpr>();
      new_expr->var = bin_expr;
      expr_lhs = new_expr;
    }
    return expr_lhs;
  }

  std::optional<NodeStmtScope *> parse_scope()
  {
    if (!try_consume(TokenType::open_curly))
    {
      std::cerr << "Expected '{'\n";
      std::exit(EXIT_FAILURE);
    }
    auto node_scope = allocator.alloc<NodeStmtScope>();
    while (peek().has_value() && peek().value().type != TokenType::close_curly)
    {
      if (auto stmt = parse_stmt())
      {
        node_scope->stmts.push_back(stmt.value());
      }
      else
      {
        std::cerr << "Expected statement inside scope\n";
        std::exit(EXIT_FAILURE);
      }
    }
    if (!try_consume(TokenType::close_curly))
    {
      std::cerr << "Expected '}'\n";
      std::exit(EXIT_FAILURE);
    }
    return node_scope;
  }

  std::optional<NodeStmtIfCont *> parse_if_cont()
  {
    if (try_consume(TokenType::elif))
    {
      if (!try_consume(TokenType::open_paren))
      {
        std::cerr << "Expected '('\n";
        std::exit(EXIT_FAILURE);
      }
      auto *node_elif = allocator.alloc<NodeStmtElif>();
      if (auto node_expr = parse_expr())
      {
        node_elif->expr = node_expr.value();
        if (!try_consume(TokenType::close_paren))
        {
          std::cerr << "Expected ')'\n";
          std::exit(EXIT_FAILURE);
        }

        if (auto node_scope = parse_scope())
        {
          node_elif->scope = node_scope.value();
          node_elif->cont = parse_if_cont();
          auto node_cont = allocator.alloc<NodeStmtIfCont>();
          node_cont->clause = node_elif;
          return node_cont;
        }
        else
        {
          std::cerr << "Expected scope\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else
      {
        std::cerr << "Expected expression\n";
        std::exit(EXIT_FAILURE);
      }
    }
    if (try_consume(TokenType::else_))
    {
      auto node_else = allocator.alloc<NodeStmtElse>();
      if (auto node_scope = parse_scope())
      {
        node_else->scope = node_scope.value();
        auto node_cont = allocator.alloc<NodeStmtIfCont>();
        node_cont->clause = node_else;
        return node_cont;
      }
      else
      {
        std::cerr << "Expected scope\n";
        std::exit(EXIT_FAILURE);
      }
    }
    return std::nullopt;
  }

  std::optional<NodeStmt *> parse_stmt()
  {
    if (peek().has_value() && peek()->type == TokenType::exit)
    {
      consume();
      auto *node_stmt_exit = allocator.alloc<NodeStmtExit>();
      auto *node_stmt = allocator.alloc<NodeStmt>();

      if (auto node_expr = parse_expr())
      {
        node_stmt_exit->expr = node_expr.value();
        node_stmt->stmt = node_stmt_exit;
        if (!try_consume(TokenType::semi))
        {
          std::cerr << "Expected semi\n";
          std::exit(EXIT_FAILURE);
        }
        return node_stmt;
      }
      else
      {
        std::cerr << "Expected Expression\n";
        std::exit(EXIT_FAILURE);
      }
    }
    else if (peek().has_value() && peek()->type == TokenType::print)
    {
      consume();
      auto *node_stmt_print = allocator.alloc<NodeStmtPrint>();
      auto *node_stmt = allocator.alloc<NodeStmt>();
      if (auto node_expr = parse_expr())
      {
        node_stmt_print->expr = node_expr.value();
        node_stmt->stmt = node_stmt_print;
        if (!try_consume(TokenType::semi))
        {
          std::cerr << "Expected semi\n";
          std::exit(EXIT_FAILURE);
        }
        return node_stmt;
      }
      else
      {
        std::cerr << "Expected Expression\n";
        std::exit(EXIT_FAILURE);
      }
    }
    else if (peek().has_value() && peek()->type == TokenType::cnst)
    {
      consume();
      auto *node_stmt_const = allocator.alloc<NodeStmtConst>();
      if (!peek().has_value())
      {
        std::cerr << "Expected type after const\n";
        std::exit(EXIT_FAILURE);
      }
      auto it = typeMappings.find(peek()->type);
      if (it == typeMappings.end())
      {
        std::cerr << "Expected valid type after const\n";
        std::exit(EXIT_FAILURE);
      }
      DataType dtype = it->second;
      node_stmt_const->dtype = dtype;
      consume();
      if (!peek().has_value() || peek()->type != TokenType::ident)
      {
        std::cerr << "Expected identifier after type\n";
        std::exit(EXIT_FAILURE);
      }
      node_stmt_const->ident = consume();
      auto *node_stmt = allocator.alloc<NodeStmt>();
      if (!peek().has_value() || peek()->type != TokenType::assign)
      {
        std::cerr << "Expected '=' after identifier\n";
        std::exit(EXIT_FAILURE);
      }
      consume();
      if (auto node_expr = parse_expr())
      {
        node_stmt_const->expr = node_expr.value();
        if (!try_consume(TokenType::semi))
        {
          std::cerr << "Expected semi\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else
      {
        std::cerr << "Expected Expression\n";
        std::exit(EXIT_FAILURE);
      }

      node_stmt->stmt = node_stmt_const;
      return node_stmt;
    }
    else if (peek().has_value() && peek()->type == TokenType::let)
    {
      consume();
      auto *node_stmt_let = allocator.alloc<NodeStmtLet>();
      if (!peek().has_value())
      {
        std::cerr << "Expected type after const\n";
        std::exit(EXIT_FAILURE);
      }
      auto it = typeMappings.find(peek()->type);
      if (it == typeMappings.end())
      {
        std::cerr << "Expected valid type after const\n";
        std::exit(EXIT_FAILURE);
      }
      DataType dtype = it->second;
      node_stmt_let->dtype = dtype;
      consume();
      if (!peek().has_value() || peek()->type != TokenType::ident)
      {
        std::cerr << "Expected identifier after type\n";
        std::exit(EXIT_FAILURE);
      }
      node_stmt_let->ident = consume();
      auto *node_stmt = allocator.alloc<NodeStmt>();
      // Optional assignment
      if (peek().has_value() && peek()->type == TokenType::assign)
      {
        consume(); // consume '='

        if (auto node_expr = parse_expr())
        {
          node_stmt_let->expr = node_expr.value();
        }
        else
        {
          std::cerr << "Expected expression after '='\n";
          std::exit(EXIT_FAILURE);
        }
      }

      // Require semicolon in both cases
      if (!try_consume(TokenType::semi))
      {
        std::cerr << "Expected ';' after let statement\n";
        std::exit(EXIT_FAILURE);
      }

      node_stmt->stmt = node_stmt_let;
      return node_stmt;
    }
    else if (peek().has_value() && peek()->type == TokenType::ident)
    {
      auto *node_stmt_assign = allocator.alloc<NodeStmtAssign>();
      node_stmt_assign->ident = consume();

      if (!peek().has_value() || peek()->type != TokenType::assign)
      {
        std::cerr << "Expected '=' after identifier\n";
        std::exit(EXIT_FAILURE);
      }
      consume();
      if (auto node_expr = parse_expr())
      {
        node_stmt_assign->expr = node_expr.value();
        if (!try_consume(TokenType::semi))
        {
          std::cerr << "Expected semi\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else
      {
        std::cerr << "Expected Expression\n";
        std::exit(EXIT_FAILURE);
      }
      auto *node_stmt = allocator.alloc<NodeStmt>();
      node_stmt->stmt = node_stmt_assign;
      return node_stmt;
    }
    else if (peek().has_value() && peek()->type == TokenType::open_curly)
    {
      if (auto node_scope = parse_scope())
      {

        auto node_stmt = allocator.alloc<NodeStmt>();
        node_stmt->stmt = node_scope.value();
        return node_stmt;
      }
      else
      {
        std::cerr << "Expected scope\n";
        std::exit(EXIT_FAILURE);
      }
    }
    else if (peek().has_value() && peek()->type == TokenType::if_)
    {
      consume();
      if (!try_consume(TokenType::open_paren))
      {
        std::cerr << "Expected '('\n";
        std::exit(EXIT_FAILURE);
      }
      auto *node_if = allocator.alloc<NodeStmtIf>();

      if (auto node_expr = parse_expr())
      {
        node_if->expr = node_expr.value();

        if (!try_consume(TokenType::close_paren))
        {
          std::cerr << "Expected ')'\n";
          std::exit(EXIT_FAILURE);
        }

        if (auto node_scope = parse_scope())
        {
          node_if->scope = node_scope.value();
          node_if->cont = parse_if_cont();
          auto node_stmt = allocator.alloc<NodeStmt>();
          node_stmt->stmt = node_if;
          return node_stmt;
        }
        else
        {
          std::cerr << "Expected scope\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else
      {
        std::cerr << "Expected expression\n";
        std::exit(EXIT_FAILURE);
      }
    }
    return std::nullopt;
  }

  std::optional<NodeProg>
  parse_prog()
  {
    NodeProg prog;

    while (peek().has_value())
    {

      if (auto node_stmt = parse_stmt())
      {
        prog.stmts.push_back(node_stmt.value());
      }
      else
      {
        std::cerr << "Expected statement\n";
        std::exit(EXIT_FAILURE);
      }
    }

    return prog;
  }

  NodeProg parse()
  {
    if (auto prog = parse_prog())
    {
      return prog.value();
    }
    else
    {
      std::cerr << "Expected Program\n";
      std::exit(EXIT_FAILURE);
    }
  }

private:
  std::optional<Token> peek(int offset = 0)
  {
    if (index + offset >= tokens.size())
    {
      return std::nullopt;
    }
    return tokens[index + offset];
  }

  Token consume()
  {
    return tokens[index++];
  }

  std::optional<Token> try_consume(TokenType type)
  {
    if (auto t = peek(); t && t->type == type)
    {
      return consume();
    }
    return std::nullopt;
  }

  int get_precedence(TokenType type)
  {
    auto it = precedence.find(type);
    if (it != precedence.end())
    {
      return it->second;
    }
    return -1;
  }

  std::unordered_map<TokenType, DataType> typeMappings = {
      {TokenType::int_, DataType::Int},
      {TokenType::char_, DataType::Char},
      {TokenType::bool_, DataType::Bool},

  };

  std::unordered_map<TokenType, int> precedence = {
      {TokenType::or_, 0},  // ||
      {TokenType::and_, 1}, // &&

      {TokenType::eq, 2}, // ==, !=
      {TokenType::neq, 2},

      {TokenType::lt, 3}, // <, >, <=, >=
      {TokenType::gt, 3},
      {TokenType::lte, 3},
      {TokenType::gte, 3},

      {TokenType::plus, 4}, // +, -
      {TokenType::sub, 4},

      {TokenType::mul, 5}, // *, /, %
      {TokenType::div, 5},
      {TokenType::mod, 5},
  };

  std::vector<Token> tokens;
  size_t index = 0;
  ArenaAllocator allocator;
};
