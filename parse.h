#pragma once
#include <cstdio>
#include <memory>
#include <vector>
#include "opc.h"
#include "err.h"
#include "ll.h"
#include "lower.h"

#define INDENT(I)            \
  for(int i = 0; i < I; i++) \
    printf("  ");

struct EXPR
{
  int offset;
  EXPR(int offset = -1)
      : offset(offset)
  {
  }
  virtual ~EXPR()
  {
  }
  virtual void   print(int indent = 0) const = 0;
  virtual Value* codegen()                   = 0;
};

struct ID : public EXPR
{
  std::string n;
  ID(std::string n, int offset = -1)
      : n(n)
      , EXPR(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("id=%s\n", n.c_str());
  }
  Value* codegen() override;
};

struct NUM : public EXPR
{
  int v;
  NUM(int v, int offset = -1)
      : v(v)
      , EXPR(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("%d\n", v);
  }
  Value* codegen() override;
};

struct STR : public EXPR
{
  std::string s;
  STR(std::string s, int offset = -1)
      : s(s)
      , EXPR(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("\"%s\"\n", s.c_str());
  }
  Value* codegen() override
  {
    return nullptr;
  }
};

struct BIFUNC : public EXPR
{
  BIFUNC(int offset = -1)
      : EXPR(offset)
  {
  }
  void   print(int indent = 0) const override = 0;
  Value* codegen() override                   = 0;
};

struct BIDEFVAR : public BIFUNC
{
  std::string           id;
  std::shared_ptr<EXPR> v;
  BIDEFVAR(std::string id, std::shared_ptr<EXPR> v, int offset = -1)
      : id(id)
      , v(v)
      , BIFUNC(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("defvar id=%s\n", id.c_str());
  }
  Value* codegen() override;
};

struct BISUM : public BIFUNC
{
  std::shared_ptr<EXPR> lhs, rhs;
  BISUM(std::shared_ptr<EXPR> l, std::shared_ptr<EXPR> r, int offset = -1)
      : lhs(l)
      , rhs(r)
      , BIFUNC(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    puts("+");
    lhs->print(indent+1);
    rhs->print(indent+1);
  }
  Value* codegen() override;
};

struct BIMUL : public BIFUNC
{
  std::shared_ptr<EXPR> lhs, rhs;
  BIMUL(std::shared_ptr<EXPR> l, std::shared_ptr<EXPR> r, int offset = -1)
      : lhs(l)
      , rhs(r)
      , BIFUNC(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    puts("*");
  }
  Value* codegen() override;
};

struct SEXPR : public EXPR
{
  std::vector<std::shared_ptr<EXPR>> exprs;
  SEXPR(int offset)
      : EXPR(offset)
  {
  }
  SEXPR(std::vector<std::shared_ptr<EXPR>> es, int offset = -1)
      : exprs(es)
      , EXPR(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    puts("sexpr:(");
    for(int i = 0; i < exprs.size(); i++)
    {
      exprs[i]->print(indent + 1);
    }
    INDENT(indent);
    puts(")");
  }
  Value* codegen() override;
};

/**
 * Prototype of a function, of the form:
 *
 *  '(' 'defun' <proto sexpr> <body sexpr> ')'
 *  '(' 'extern' <proto sexpr> ')'
 */
struct PROTOTYPE
{
  std::string              n;
  std::vector<std::string> args;
  PROTOTYPE(std::shared_ptr<SEXPR> se);
  void print(int indent = 0) const
  {
    INDENT(indent);
    printf("prototype: double %s(", n.c_str());
    for(int i = 0; i < args.size(); i++)
    {
      printf("double %s", args[i].c_str());
      if(i + 1 < args.size())
        printf(", ");
    }
    puts(")");
  }
  Function* codegen();
};

struct USERFUNC : public EXPR
{
  static std::map<std::string, Value*> local_values;
  std::shared_ptr<PROTOTYPE>           proto;
  std::shared_ptr<SEXPR>               body;
  USERFUNC(std::shared_ptr<PROTOTYPE> p, std::shared_ptr<SEXPR> b, int offset = -1)
      : proto(p)
      , body(b)
      , EXPR(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    puts("user function:");
    proto->print(indent + 1);
    INDENT(indent + 1);
    puts("body:");
    body->print(indent + 2);
  }
  Value* codegen() override;
};

struct CALLEXPR : public EXPR
{
  std::string                        n;
  std::vector<std::shared_ptr<EXPR>> args;
  CALLEXPR(std::string n, std::vector<std::shared_ptr<EXPR>> args, int offset = -1)
      : n(n)
      , args(args)
      , EXPR(offset)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("call expr %s(\n", this->n.c_str());
    for(auto arg : args)
      arg->print(indent + 1);
    INDENT(indent);
    puts(")");
  }
  Value* codegen() override;
};

struct MODULE : public EXPR
{
  std::vector<std::shared_ptr<SEXPR>> sexprs;
  void                                print(int indent = 0) const override
  {
    INDENT(indent);
    puts("MODULE:{");
    for(int i = 0; i < sexprs.size(); i++)
      sexprs[i]->print(indent + 1);
    INDENT(indent);
    puts("}");
  }
  /**
   * Try to generate all functions before creating the dummy entrypoint function
   */
  Value* codegen_funcs();
  Value* codegen() override;
};

struct E_EOF : public EXPR
{
  void print(int indent = 0) const override
  {
    puts("EOF");
  }
  Value* codegen() override
  {
    return nullptr;
  }
};

#undef INDENT

enum TOK
{
#define TOK_PROC(X) X,
#include "parse.def"
#undef TOK_PROC
};

typedef struct
{
  TOK t;
  int offset;
  union
  {
    int   i_val;
    char* s_val;
    char  bin;
  } val;
} TOKEN;

int                     lex(FILE*);
std::shared_ptr<MODULE> parse();
void                    parse_finalize();
void                    parse_dump();

TOKEN* tok(int toki);
void   tok_iter(void (*visitor)(TOKEN*));
EXPR*  expr(int expi);
FILE*  preproc(FILE* istream);
void   dump_tok();
