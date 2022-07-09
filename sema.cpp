#include <vector>
#include <string>
#include "parse.h"
#include "config.h"
#include "err.h"
#include "ast_visitor.h"
static int parens     = 0;
static int bad_offset = -1;

static void tok_visitor(TOKEN* t)
{
  parens += t->t == TOK_LPAREN;
  parens -= t->t == TOK_RPAREN;
  if(t->t == TOK_RPAREN && parens < 0)
  {
    bad_offset = t->offset;
  }
}

void lex_sema()
{
  tok_iter(tok_visitor);
  if(parens < 0)
    reg_msg(LC_MSG{"sema", "unbalanced parens", MSG_FATAL, bad_offset});
}

Value* BISUM::codegen()
{
  auto* l = lhs->codegen();
  auto* r = rhs->codegen();
  return get_builder().CreateFAdd(l, r, "sum" + lower_id());
}

Value* BIMUL::codegen()
{
  auto* l = lhs->codegen();
  auto* r = rhs->codegen();
  return get_builder().CreateFMul(l, r, "mul" + lower_id());
}

Value* BIDEFVAR::codegen()
{
  if (dump("lower"))
    printf("lowering defvar '%s'\n", id.c_str());
  add_value(id, v->codegen());
  return get_value(id);
}

static std::vector<std::tuple<std::string, std::string>> repls{
  {"+", "sum"},
  {"-", "sub"},
  {"*", "mul"},
  {"/", "div"},
};
struct replace_builtins : public VISITOR
{
  bool visitSEXPR(std::shared_ptr<SEXPR> se) override
  {
    if(se->exprs.empty())
      return false;
    if(auto id = std::dynamic_pointer_cast<ID>(se->exprs[0]))
    {
      if(id->n == "sum" or id->n == "+")
      {
        se->exprs[0] = std::make_shared<BISUM>(se->exprs[1], se->exprs[2]);
        return true;
      }
      else if (id->n == "mul" or id->n == "*")
      {
        se->exprs[0] = std::make_shared<BIMUL>(se->exprs[1], se->exprs[2]);
        return true;
      }
      else if (id->n == "defvar")
      {
        auto id2 = std::dynamic_pointer_cast<ID>(se->exprs[1]);
        se->exprs[0] = std::make_shared<BIDEFVAR>(id2->n, se->exprs[2]);
        return true;
      }
    }
    return false;
  }
  bool visitID(std::shared_ptr<ID> id) override
  {
    /*
    for (auto const& [k, v] : repls) {
      if (id->n == k)
      {
        id->n = v;
        std::string msg = "replaced id '" + k + "' with builtin '" + v + "'";
        reg_msg(LC_MSG{"sema", msg, MSG_INFO});
        return true;
      }
    }
    */
    return false;
  }
};

void sema_builtins(std::shared_ptr<MODULE> m)
{
  for(int i = 0; i < m->sexprs.size(); i++)
  {
    expr_visit(m->sexprs[i], std::make_shared<replace_builtins>());
  }
}
