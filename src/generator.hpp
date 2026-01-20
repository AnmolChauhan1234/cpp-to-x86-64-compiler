#include <vector>
#include <sstream>
#include <unordered_map>

class Generator
{

public:
  explicit Generator(NodeProg program) : prog(std::move(program)) {}
  DataType gen_lit(const NodeTermLit *term_lit)
  {
    const Token &tok = term_lit->token;

    switch (tok.type)
    {
    case TokenType::int_lit:
    {

      int64_t value;
      try
      {
        size_t idx;
        value = std::stoll(tok.val.value(), &idx, 10);
        if (idx != tok.val.value().size())
        {
          throw std::invalid_argument("Invalid integer literal");
        }
      }
      catch (const std::out_of_range &)
      {
        std::cerr << "Integer literal out of bounds\n";
        exit(EXIT_FAILURE);
      }
      catch (const std::invalid_argument &)
      {
        std::cerr << "Invalid integer literal\n";
        exit(EXIT_FAILURE);
      }
      output << "    mov rax, " << value << "\n";
      push("rax");
      return DataType::Int;
    }
    case TokenType::char_lit:
    {
      char value = tok.val.value()[0];
      output << "    mov rax, " << static_cast<int64_t>(value) << "\n";
      push("rax");
      return DataType::Char;
    }
    case TokenType::bool_lit:
    {
      std::string value = tok.val.value();

      if (value == "true")
      {
        output << "    mov rax, 1\n";
        push("rax");
      }
      else if (value == "false")
      {
        output << "    mov rax, 0\n";
        push("rax");
      }
      else
      {
        std::cerr << "Unknown boolean literal: " << value << "\n";
        exit(EXIT_FAILURE);
      }

      return DataType::Bool;
    }
    default:
      std::cerr << "Unknown literal type\n";
      exit(EXIT_FAILURE);
    }
  }
  DataType gen_term(const NodeTerm *term)
  {
    struct TermVisitor
    {
      Generator *gen;
      DataType operator()(const NodeTermLit *term_lit) const
      {
        return gen->gen_lit(term_lit);
      }
      DataType operator()(const NodeTermIdent *term_ident) const
      {
        if (!gen->globals.contains(term_ident->ident.val.value()))
        {
          std::cerr << "Variable " << term_ident->ident.val.value() << " not declared" << std::endl;
          exit(EXIT_FAILURE);
        }
        const auto &var = gen->globals.at(term_ident->ident.val.value());
        std::stringstream offset;
        offset << "QWORD [rsp + " << (gen->stack_size - var.stack_loc) * 8 << "]";
        gen->push(offset.str());
        return var.dtype;
      }
      DataType operator()(const NodeTermParen *term_paren) const
      {
        return gen->gen_expr(term_paren->expr);
      }
      DataType operator()(const NodeTermUnary *term_unary) const
      {
        switch (term_unary->op)
        {
        case UnaryOp::Negate:
        {
          DataType dtype = gen->gen_term(term_unary->operand);
          if (dtype != DataType::Int)
          {
            std::cerr << "Cannot use '-' on non integers\n";
            exit(EXIT_FAILURE);
          }
          gen->pop("rax");
          gen->output << "    neg rax\n";
          gen->push("rax");
          return DataType::Int;
        }
        case UnaryOp::Not:
        {
          DataType dtype = gen->gen_term(term_unary->operand);
          if (dtype != DataType::Int && dtype != DataType::Bool)
          {
            std::cerr << "Cannot use '!' on non-integers or non-booleans\n";
            exit(EXIT_FAILURE);
          }

          gen->pop("rax");
          // Compare rax with 0
          gen->output << "    cmp rax, 0\n";
          // Set AL to 1 if equal (operand was 0), else 0
          gen->output << "    sete al\n";
          // Zero-extend AL into RAX
          gen->output << "    movzx rax, al\n";
          gen->push("rax");

          return DataType::Bool;
        }

        default:
          std::cerr << "Unknown unary operator\n";
          exit(EXIT_FAILURE);
        }
      }
    };
    TermVisitor visitor(this);
    return std::visit(visitor, term->val);
  }

  DataType gen_bin_expr(const NodeBinExpr *bin_expr)
  {
    struct BinExprVisitor
    {
      Generator *gen;
      DataType operator()(const NodeBinExprAdd *add) const
      {
        DataType lhs_type = gen->gen_expr(add->lhs);
        DataType rhs_type = gen->gen_expr(add->rhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Addition operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }

        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    add rax, rbx\n";
        gen->output << "    jo overflow_error\n";
        gen->push("rax");
        return DataType::Int;
      }
      DataType operator()(const NodeBinExprMul *mul) const
      {
        DataType lhs_type = gen->gen_expr(mul->lhs);
        DataType rhs_type = gen->gen_expr(mul->rhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Multiplication operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }

        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    imul rbx\n";
        gen->output << "    jo overflow_error\n";
        gen->push("rax");
        return DataType::Int;
      }

      DataType operator()(const NodeBinExprSub *sub) const
      {
        DataType rhs_type = gen->gen_expr(sub->rhs);
        DataType lhs_type = gen->gen_expr(sub->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Subtraction operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }

        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    sub rax, rbx\n";
        gen->output << "    jo overflow_error\n";
        gen->push("rax");
        return DataType::Int;
      }

      DataType operator()(const NodeBinExprDiv *div) const
      {
        DataType rhs_type = gen->gen_expr(div->rhs);
        DataType lhs_type = gen->gen_expr(div->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Division operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    cmp rbx, 0\n";
        gen->output << "    je divzero_error\n"; // check division by zero
        gen->output << "    cqo\n";              // sign-extend RAX -> RDX:RAX
        gen->output << "    idiv rbx\n";         // RAX/RBX -> quotient in RAX, remainder in RDX
        gen->push("rax");
        return DataType::Int;
      }

      DataType operator()(const NodeBinExprMod *mod) const
      {
        DataType rhs_type = gen->gen_expr(mod->rhs);
        DataType lhs_type = gen->gen_expr(mod->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Modulo operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    cmp rbx, 0\n";
        gen->output << "    je divzero_error\n"; // check division by zero
        gen->output << "    cqo\n";
        gen->output << "    idiv rbx\n";
        gen->push("rdx");
        return DataType::Int;
      }

      DataType operator()(const NodeBinExprEq *eq) const
      {
        DataType rhs_type = gen->gen_expr(eq->rhs);
        DataType lhs_type = gen->gen_expr(eq->lhs);

        if (lhs_type != rhs_type)
        {
          std::cerr << "Error: Equality comparison requires both operands to be of the same type" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    sete al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
        return DataType::Bool; // Boolean value of true and false
      }

      DataType operator()(const NodeBinExprNeq *neq) const
      {
        DataType rhs_type = gen->gen_expr(neq->rhs);
        DataType lhs_type = gen->gen_expr(neq->lhs);

        if (lhs_type != rhs_type)
        {
          std::cerr << "Error: Non Equality comparison requires both operands to be of the same type" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setne al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
        return DataType::Bool;
      }

      DataType operator()(const NodeBinExprLt *lt) const
      {
        DataType rhs_type = gen->gen_expr(lt->rhs);
        DataType lhs_type = gen->gen_expr(lt->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Less Then operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setl al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
        return DataType::Bool;
      }

      DataType operator()(const NodeBinExprGt *gt) const
      {
        DataType rhs_type = gen->gen_expr(gt->rhs);
        DataType lhs_type = gen->gen_expr(gt->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Greater Then operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setg al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
        return DataType::Bool;
      }

      DataType operator()(const NodeBinExprLte *lte) const
      {
        DataType rhs_type = gen->gen_expr(lte->rhs);
        DataType lhs_type = gen->gen_expr(lte->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Less Then Equal to operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setle al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
        return DataType::Bool;
      }

      DataType operator()(const NodeBinExprGte *gte) const
      {
        DataType rhs_type = gen->gen_expr(gte->rhs);
        DataType lhs_type = gen->gen_expr(gte->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Greater Then Equal to operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setge al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
        return DataType::Bool;
      }
      DataType operator()(const NodeBinExprAnd *gte) const
      {
        DataType rhs_type = gen->gen_expr(gte->rhs);
        DataType lhs_type = gen->gen_expr(gte->lhs);

        if ((lhs_type != DataType::Int && lhs_type != DataType::Bool) || (rhs_type != DataType::Int && rhs_type != DataType::Bool))
        {
          std::cerr << "Error: Greater Then Equal to operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }

        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs

        gen->output << "    cmp rax, 0\n";
        gen->output << "    setne al\n";
        gen->output << "    movzx rax, al\n";

        gen->output << "    cmp rbx, 0\n";
        gen->output << "    setne bl\n";
        gen->output << "    movzx rbx, bl\n";

        // Logical AND (bitwise and of 0/1 values)
        gen->output << "    and rax, rbx\n";

        gen->push("rax");
        return DataType::Bool;
      }
      DataType operator()(const NodeBinExprOr *gte) const
      {
        DataType rhs_type = gen->gen_expr(gte->rhs);
        DataType lhs_type = gen->gen_expr(gte->lhs);

        if (lhs_type != DataType::Int || rhs_type != DataType::Int)
        {
          std::cerr << "Error: Greater Then Equal to operator requires both operands to be integers" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, 0\n";
        gen->output << "    setne al\n";
        gen->output << "    movzx rax, al\n";

        gen->output << "    cmp rbx, 0\n";
        gen->output << "    setne bl\n";
        gen->output << "    movzx rbx, bl\n";

        // Logical OR (bitwise or of 0/1 values)
        gen->output << "    or rax, rbx\n";

        gen->push("rax");
        return DataType::Bool;
      }
    };
    BinExprVisitor visitor(this);
    return std::visit(visitor, bin_expr->op);
  }

  DataType gen_expr(const NodeExpr *expr)
  {
    struct ExprVisistor
    {
      Generator *gen;
      DataType operator()(const NodeTerm *term) const
      {
        return gen->gen_term(term);
      }
      DataType operator()(const NodeBinExpr *bin_expr) const
      {
        return gen->gen_bin_expr(bin_expr);
      }
    };
    ExprVisistor visitor(this);
    return std::visit(visitor, expr->var);
  }

  void gen_scope(const NodeStmtScope *stmt_scope)
  {
    enter_scope();

    for (const NodeStmt *stmt_s : stmt_scope->stmts)
    {
      gen_stmt(stmt_s);
    }
    exit_scope();
  }

  void gen_exit()
  {
    output << "    mov rax, 1\n";
    output << "    mov rdi, 1\n";
    output << "    lea rsi, [rsp-1]\n";
    output << "    mov byte [rsp-1], 10\n";
    output << "    mov rdx, 1\n";
    output << "    syscall\n";
    output << "    mov rax, 60\n";
    pop("rdi");
    output << "    syscall\n";
    is_terminated = true;
  }

  void gen_if_cont(const NodeStmtIfCont *stmt_if_cont, const std::string &end_label)
  {
    struct StmtIfContVisitor
    {
      Generator *gen;
      const std::string &end_label;

      void operator()(const NodeStmtElif *stmt_elif) const
      {
        gen->gen_expr(stmt_elif->expr);
        gen->pop("rax");
        std::string label = gen->create_label();
        gen->output << "    test rax, rax\n";
        gen->output << "    jz " << label << "\n";
        gen->gen_scope(stmt_elif->scope);
        gen->output << "    jmp " << end_label << "\n";
        if (stmt_elif->cont.has_value())
        {
          gen->output << label << ":\n";
          gen->gen_if_cont(stmt_elif->cont.value(), end_label);
        }
      }

      void operator()(const NodeStmtElse *stmt_else) const
      {
        gen->gen_scope(stmt_else->scope);
      }
    };
    StmtIfContVisitor visitor{this, end_label};
    std::visit(visitor, stmt_if_cont->clause);
  }

  void gen_stmt(const NodeStmt *stmt)
  {
    struct StmtVisitor
    {
      Generator *gen;
      void operator()(const NodeStmtExit *stmt_exit) const
      {
        gen->gen_expr(stmt_exit->expr);
        gen->gen_exit();
      }
      void operator()(const NodeStmtPrint *stmt_print) const
      {
        DataType dtpye = gen->gen_expr(stmt_print->expr);
        gen->pop("rdi");
        switch (dtpye)
        {
        case DataType::Int:
        {
          gen->output << "    call print_int\n";
          break;
        }
        case DataType::Bool:
        {
          gen->output << "    call print_int\n";
          break;
        }
        case DataType::Char:
        {
          gen->output << "    call print_char\n";
          break;
        }

        default:
          break;
        }
      }
      void operator()(const NodeStmtIf *stmt_if) const
      {
        gen->gen_expr(stmt_if->expr);
        gen->pop("rax");
        std::string label = gen->create_label();
        gen->output << "    test rax, rax\n";
        gen->output << "    jz " << label << "\n";
        gen->gen_scope(stmt_if->scope);
        if (stmt_if->cont.has_value())
        {
          const std::string end_label = gen->create_label();
          gen->output << "    jmp " << end_label << "\n";
          gen->output << label << ":\n";
          gen->gen_if_cont(stmt_if->cont.value(), end_label);
          gen->output << end_label << ":\n";
        }
        else
        {
          gen->output << label << ":\n";
        }
      }
      void operator()(const NodeStmtConst *stmt_const) const
      {
        if (gen->is_declared(stmt_const->ident.val.value()))
        {
          std::cerr << "Variable " << stmt_const->ident.val.value() << " already declared" << std::endl;
          exit(EXIT_FAILURE);
        }
        DataType expr_type = gen->gen_expr(stmt_const->expr);
        if (expr_type != stmt_const->dtype)
        {
          std::cerr << "Error: Type mismatch for variable '" << stmt_const->ident.val.value()
                    << "'. Expected " << gen->type_to_string(stmt_const->dtype)
                    << " but got " << gen->type_to_string(expr_type) << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->declare_var(stmt_const->ident.val.value(), Var(gen->stack_size, stmt_const->dtype));
      }
      void operator()(const NodeStmtLet *stmt_let) const
      {
        if (gen->is_declared(stmt_let->ident.val.value()))
        {
          std::cerr << "Variable " << stmt_let->ident.val.value() << " already declared" << std::endl;
          exit(EXIT_FAILURE);
        }
        if (!stmt_let->expr.has_value())
        {
          gen->output << "mov rax, 0\n";
          gen->push("rax");
        }
        else
        {
          DataType expr_type = gen->gen_expr(stmt_let->expr.value());
          if (expr_type != stmt_let->dtype)
          {
            std::cerr << "Error: Type mismatch for variable '" << stmt_let->ident.val.value()
                      << "'. Expected " << gen->type_to_string(stmt_let->dtype)
                      << " but got " << gen->type_to_string(expr_type) << std::endl;
            exit(EXIT_FAILURE);
          }
        }
        gen->declare_var(stmt_let->ident.val.value(), Var(gen->stack_size, stmt_let->dtype, true));
      }
      void operator()(const NodeStmtAssign *stmt_assign)
      {
        if (!stmt_assign->ident.val.has_value())
        {
          std::cerr << "Error: Assignment statement missing identifier." << std::endl;
          exit(EXIT_FAILURE);
        }
        if (!gen->globals.contains(stmt_assign->ident.val.value()))
        {
          std::cerr << "You need to declare the variable first";
          exit(EXIT_FAILURE);
        }
        auto &existing_var = gen->globals.at(stmt_assign->ident.val.value());
        if (!existing_var.mut)
        {
          std::cerr << "Error: Cannot assign to immutable variable '"
                    << stmt_assign->ident.val.value() << "'\n";
          exit(EXIT_FAILURE);
        }
        DataType type = gen->gen_expr(stmt_assign->expr);
        if (type != existing_var.dtype)
        {
          std::cerr << "Error: Type mismatch in assignment to '"
                    << stmt_assign->ident.val.value() << "'. Expected "
                    << gen->type_to_string(existing_var.dtype)
                    << ", got " << gen->type_to_string(type) << "\n";
          exit(EXIT_FAILURE);
        }
        const auto new_var = Var(gen->stack_size, type, true);
        gen->update_var(stmt_assign->ident.val.value(), existing_var, new_var);
      }
      void operator()(const NodeStmtScope *stmt_scope) const
      {
        gen->gen_scope(stmt_scope);
      }
    };
    StmtVisitor visitor{this};
    std::visit(visitor, stmt->stmt);
  }

  std::string gen_prog()
  {

    output << "extern print_int\n"
           << "extern print_string\n"
           << "extern print_char\n"
           << "extern overflow_error\n"
           << "extern divzero_error\n"
           << "global _start\n"
           << "_start:\n";

    for (const NodeStmt *stmt : prog.stmts)
    {
      if (is_terminated)
        return output.str();
      gen_stmt(stmt);
    }

    gen_exit();

    return output.str();
  }

private:
  struct Var
  {
    size_t stack_loc;
    DataType dtype;
    bool mut;
    Var() : stack_loc(0), dtype(DataType::Int), mut(false) {}
    Var(size_t stack_loc, DataType dtype, bool mut = false)
        : stack_loc(stack_loc), dtype(dtype), mut(mut) {}
  };

  struct ScopeEntry
  {
    std::string name;
    std::optional<Var> old_binding; // empty if no shadowing
  };

  void push(const std::string &reg)
  {
    output << "    push " << reg << "\n";
    stack_size++;
  }
  void pop(const std::string &reg)
  {
    output << "    pop " << reg << "\n";
    stack_size--;
  }
  std::string create_label()
  {
    std::stringstream ss;
    ss << "label" << label_count++;
    return ss.str();
  }

  void enter_scope()
  {
    scopes.push_back({});
  }

  void exit_scope()
  {
    for (auto &entry : scopes.back())
    {
      if (entry.old_binding.has_value())
      {
        // Restore the old value (either from shadowing or from assignment)
        globals[entry.name] = entry.old_binding.value();
      }
      else
      {
        // Variable was new, remove it
        globals.erase(entry.name);
      }
    }
    scopes.pop_back();
  }

  void update_var(const std::string &name, Var &old_var, Var new_var)
  {
    // Save the current state before modifying
    if (!scopes.empty())
    {
      scopes.back().push_back({name, old_var});
    }

    // Now update the variable
    old_var = new_var;
  }

  void declare_var(const std::string &name, Var var)
  {
    std::optional<Var> old_binding;
    if (globals.contains(name))
    {
      old_binding = globals[name];
    }

    globals[name] = var;

    if (scopes.empty())
    {
      scopes.push_back({});
    }
    scopes.back().push_back({name, old_binding});
  }

  bool is_declared(const std::string &name) const
  {
    if (scopes.empty())
      return false;
    for (auto &entry : scopes.back())
    {
      if (entry.name == name)
      {
        return true; // declared in this scope
      }
    }
    return false;
  }

  std::string type_to_string(DataType type) const
  {
    switch (type)
    {
    case DataType::Int:
      return "int";
    case DataType::Char:
      return "char";
    default:
      return "unknown";
    }
  }

  bool is_terminated = false;
  std::stringstream output;
  const NodeProg prog;
  size_t stack_size = 0;
  int label_count = 0;
  std::unordered_map<std::string, Var> globals{};
  std::vector<std::vector<ScopeEntry>> scopes;
};