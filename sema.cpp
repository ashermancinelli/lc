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
    else if(auto id = std::dynamic_pointer_cast<ID>(se->exprs[0]))
    {
      if(id->n == "sum" or id->n == "+")
      {
        se->exprs[0] = std::make_shared<BISUM>(se->exprs[1], se->exprs[2]);
        se->exprs.resize(1);
        return true;
      }
      else if(id->n == "mul" or id->n == "*")
      {
        se->exprs[0] = std::make_shared<BIMUL>(se->exprs[1], se->exprs[2]);
        se->exprs.resize(1);
        return true;
      }
      else if(id->n == "defvar")
      {
        if(se->exprs.size() < 3)
          reg_msg(LC_MSG{"sema", "defvar called with fewer than 2 arguments", MSG_FATAL});
        auto id2 = std::dynamic_pointer_cast<ID>(se->exprs[1]);
        if(!id2)
          reg_msg(LC_MSG{"sema", "defvar called with non-id as first parameter", MSG_FATAL});
        se->exprs[0] = std::make_shared<BIDEFVAR>(id2->n, se->exprs[2]);
        se->exprs.resize(1);
        return true;
      }
      else if(id->n == "defun")
      {
        LCASSERT_P("sema", "defun requires a prototype and a body", se->exprs.size() == 3);

        auto ps = std::dynamic_pointer_cast<SEXPR>(se->exprs[1]);
        LCASSERT_P("sema", "defun prototype must be a sexpr", ps);
        auto proto = std::make_shared<PROTOTYPE>(ps);

        auto body = std::dynamic_pointer_cast<SEXPR>(se->exprs[2]);
        LCASSERT_P("sema", "defun body must be a sexpr", body);

        auto f       = std::make_shared<USERFUNC>(proto, body);
        se->exprs[0] = f;
        se->exprs.resize(1);
        return true;
      }
      else
      {
        auto                               calleeid = std::dynamic_pointer_cast<ID>(se->exprs[0]);
        auto                               callee   = calleeid->n;
        std::vector<std::shared_ptr<EXPR>> args;
        for(int i = 1; i < se->exprs.size(); i++)
          args.push_back(se->exprs[i]);
        se->exprs[0] = std::make_shared<CALLEXPR>(callee, args);
        se->exprs.resize(1);
        return true;
      }
    }
    return false;
  }
  bool visitID(std::shared_ptr<ID> id) override { return false; }
};

void sema_builtins(std::shared_ptr<MODULE> m)
{
again:
  bool anychanged = false;
  for(int i = 0; i < m->sexprs.size(); i++)
  {
    anychanged = anychanged or expr_visit(m->sexprs[i], std::make_shared<replace_builtins>());
  }
  // Continue visiting until nothing has changed
  if (anychanged)
    goto again;
}
