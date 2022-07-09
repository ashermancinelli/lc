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
  virtual ~EXPR()
  {
  }
  virtual void   print(int indent = 0) const = 0;
  virtual Value* codegen()                   = 0;
};

struct ID : public EXPR
{
  std::string n;
  ID(std::string n)
      : n(n)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("id=%s\n", n.c_str());
  }
  virtual Value* codegen();
};

struct NUM : public EXPR
{
  int v;
  NUM(int v)
      : v(v)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("%d\n", v);
  }
  virtual Value* codegen();
};

struct STR : public EXPR
{
  std::string s;
  STR(std::string s)
      : s(s)
  {
  }
  void print(int indent = 0) const override
  {
    INDENT(indent);
    printf("\"%s\"\n", s.c_str());
  }
  virtual Value* codegen() { return nullptr; }
};

struct BIFUNC : public EXPR {
  void print(int indent=0) const override = 0;
  Value* codegen() override = 0;
};

/*
struct BIEXTERN : public BIFUNC {
  std::string n;
  void print(int indent=0) const override {
    INDENT(indent);
    printf("extern func '%s'\n", n.c_str();)
  }
};
*/

struct BIDEFVAR : public BIFUNC {
  std::string id;
  std::shared_ptr<EXPR> v;
  BIDEFVAR(std::string id, std::shared_ptr<EXPR> v) : id(id), v(v) {}
  void print(int indent=0) const override {
    INDENT(indent);
    printf("defvar id=%s\n", id.c_str());
  }
  Value* codegen() override;
};

struct BISUM : public BIFUNC {
  std::shared_ptr<EXPR> lhs, rhs;
  BISUM(std::shared_ptr<EXPR> l, std::shared_ptr<EXPR> r) : lhs(l), rhs(r) {}
  void print(int indent=0) const override {
    INDENT(indent);
    puts("+");
  }
  Value* codegen() override;
};

struct BIMUL : public BIFUNC {
  std::shared_ptr<EXPR> lhs, rhs;
  BIMUL(std::shared_ptr<EXPR> l, std::shared_ptr<EXPR> r) : lhs(l), rhs(r) {}
  void print(int indent=0) const override {
    INDENT(indent);
    puts("*");
  }
  Value* codegen() override;
};

struct SEXPR : public EXPR
{
  std::vector<std::shared_ptr<EXPR>> exprs;
  void                               print(int indent = 0) const override
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
  virtual Value* codegen();
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
  virtual Value* codegen() {
    Value * last;
    for (auto se : sexprs)
      last = se->codegen();
    return last;
  }
};

struct E_EOF : public EXPR
{
  void print(int indent = 0) const override
  {
    puts("EOF");
  }
  virtual Value* codegen() { return nullptr; }
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
