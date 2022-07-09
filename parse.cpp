#include <cstdlib>
#include <cstring>
#include "parse.h"
#include "opc.h"
#include "err.h"
#include "lower.h"
#include "dbg.h"
#include "config.h"

static unsigned preproc_providence[MAXFILESZ];

static TOKEN tok_table[TOKSZ];
static int   tok_it  = 1;
static int   tok_max = 1;
static int   cur_tok = 1;
static int   tok_cur()
{
  return cur_tok;
}
static int tok_next_pre()
{
  if(cur_tok < tok_it)
    return ++cur_tok;
  return -1;
}
static int tok_next()
{
  if(cur_tok < tok_it)
    return cur_tok++;
  return -1;
}
static int tok_unget()
{
  if(cur_tok > 1)
    return --cur_tok;
  return -1;
}
static int tok_peek()
{
  if(tok_it < cur_tok)
    return cur_tok + 1;
  return -1;
}
#define TOK_PUSH(T)                                                            \
  {                                                                            \
    if(tok_it + 1 > TOKSZ)                                                     \
      reg_msg(LC_MSG{"tokenize", "exceeded max token table size", MSG_FATAL}); \
    tok_table[tok_it++] = T;                                                   \
    tok_max             = tok_max > tok_it ? tok_max : tok_it;                 \
  }
#define TOK_ITER(X) for(int X = 1; X < tok_it; X++)
#define TOKI(I)     tok_table[I]

TOKEN* tok(int i)
{
  return &TOKI(i);
}

static void dump_tok_i(int toki, const char* msg = nullptr)
{
  if(msg != nullptr)
    printf("%s:", msg);

  switch(TOKI(toki).t)
  {
#define TOK_PROC(T) \
  case T:           \
    printf(#T);     \
    break;
#include "parse.def"
#undef TOK_PROC
  default:
    printf("t:?unknown token?");
  }
  switch(TOKI(toki).t)
  {
  case TOK_NUMLIT:
    printf(":%d", TOKI(toki).val.i_val);
    break;
  case TOK_STRLIT:
    printf(":%s", TOKI(toki).val.s_val);
    break;
  case TOK_ID:
    printf(":%s", TOKI(toki).val.s_val);
    break;
  case TOK_BINOP:
    printf(":%c", TOKI(toki).val.bin);
    break;
  }
  printf("\t\t(offset=%d)", TOKI(toki).offset);
  puts("");
}

std::shared_ptr<EXPR> parse_expr()
{
  static int toki = 1;
  while(true)
  {
    TOKEN t = TOKI(toki);
  tryagain:
    switch(t.t)
    {
    case TOK_EOF:
    case TOK_EOL:
    case TOK_LPAREN:
      reg_msg(LC_MSG{"parse", "sub-sexprs not yet supported", MSG_FATAL});
      break;
    case TOK_NUMLIT:
      return std::make_shared<NUM>(t.val.i_val);
    case TOK_STRLIT:
      return std::make_shared<STR>(std::string(t.val.s_val));
    case TOK_ID:
      return std::make_shared<ID>(std::string(t.val.s_val));
    }
    toki++;
  }
}

/*
void dump_opc_i(int exp, int indent)
{
#define INDENT()                  \
  for(int i = 0; i < indent; i++) \
    printf("  ");

  EXPR e = EXPRI(exp);

  switch(e.opc)
  {
#define OPC_PROC(O) \
  case O:           \
    printf(#O ":"); \
    break;
#include "opc.def"
#undef OPC_PROC
  }

  switch(e.opc)
  {
  case OP_ST:
    printf("%s", e.val.s);
    puts("");
    dump_opc_i(e.next, indent + 1);
    break;
  case OP_SEXPR:
    printf("%s", e.val.s);
    puts("");
    dump_opc_i(e.next, indent + 1);
    break;
  case OP_LD:
    printf("%s", e.val.s);
    break;
  case OP_NUM:
    printf("%d", e.val.i);
    break;
  case OP_STR:
    printf("%s", e.val.s);
    break;
  }
  puts("");
#undef INDENT
}

void dump_opc_table()
{
  for(int i = 0; i < root_expr_it; i++)
  {
    // int e = root_exprs[i];
    // dump_opc_i(e, 0);
  }
}
*/

static void eat(TOK t)
{
  int ti = tok_next();
  if(TOKI(ti).t != t)
  {
    std::string ts = "eat:expected token ";
    switch(t)
    {
#define TOK_PROC(T) \
  case T:           \
    ts += #T;       \
    break;
#include "parse.def"
#undef TOK_PROC
    }
    ts += ", got token ";
    switch(TOKI(ti).t)
    {
#define TOK_PROC(T) \
  case T:           \
    ts += #T;       \
    break;
#include "parse.def"
#undef TOK_PROC
    }
    reg_msg(LC_MSG{"parse", ts, MSG_FATAL, TOKI(ti).offset});
  }
}

std::shared_ptr<SEXPR> parse_sexpr()
{
  if(dump("parse-sexpr"))
    puts("start sexpr parse");
  eat(TOK_LPAREN);
  int  ti = tok_next(), ei = -1;
  TOK  tt;
  auto se = std::make_shared<SEXPR>(TOKI(ti).offset);

  while((tt = TOKI(ti).t) != TOK_RPAREN)
  {
    if(dump("parse-sexpr"))
      switch(tt)
      {
#define TOK_PROC(X) \
  case X:           \
    puts(#X);       \
    break;
#include "parse.def"
#undef TOK_PROC
      }
    if(tt == TOK_LPAREN)
    {
      tok_unget();
      se->exprs.push_back(parse_sexpr());
    }
    else if(tt == TOK_ID)
      se->exprs.push_back(std::make_shared<ID>(std::string(TOKI(ti).val.s_val)));
    else if(tt == TOK_STRLIT)
      se->exprs.push_back(std::make_shared<STR>(std::string(TOKI(ti).val.s_val)));
    else if(tt == TOK_NUMLIT)
      se->exprs.push_back(std::make_shared<NUM>(TOKI(ti).val.i_val));
    ti = tok_next();
  }
  if(dump("parse-sexpr"))
    puts("end sexpr parse");
  return std::move(se);
}

std::shared_ptr<MODULE> parse()
{
  auto m = std::make_shared<MODULE>();

  for(int ti = tok_cur(); TOKI(tok_cur()).t != TOK_EOF; ti = tok_cur())
  {
    if(TOKI(ti).t == TOK_LPAREN)
    {
      m->sexprs.push_back(parse_sexpr());
    }
    else if(TOKI(ti).t == TOK_EOL)
    {
      eat(TOK_EOL);
    }
    else
    {
      reg_msg(LC_MSG{"parse", "unexpected token at top-level parser", MSG_FATAL});
    }
  }

  return std::move(m);
}

int lex(FILE* istream)
{
#define DIE()                                                       \
  do                                                                \
  {                                                                 \
    printf("unrecognized token at position %d, char=%c\n", pos, c); \
    std::abort();                                                   \
  } while(0);
  char  c;
  int   it;
  char  s[1024];
  int   num;
  TOKEN t;
  int   pos = 0;
  while((c = getc(istream)) != EOF)
  {
    t.offset = preproc_providence[ftell(istream)];
    switch(c)
    {

    case '(':
      t.t = TOK_LPAREN;
      break;
    case ')':
      t.t = TOK_RPAREN;
      break;

    /*------------------------------------------------------------------------
     * Skipper
     *----------------------------------------------------------------------*/
    case ' ':
    case '\t':
      continue;
    case '\n':
      t.t = TOK_EOL;
      break;
    case ';':
      while((c = getc(istream)) != '\n')
        ;
      continue;

    /*------------------------------------------------------------------------
     * Literal Parsing
     *----------------------------------------------------------------------*/
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      ungetc(c, istream);
      fscanf(istream, "%d", &num);
      t.t         = TOK_NUMLIT;
      t.val.i_val = num;
      break;

    case '"':
      memset(s, 0, sizeof(s));
      it = 0;
      while((c = getc(istream)) != '"')
      {
        s[it++] = c;
      }
      t.t         = TOK_STRLIT;
      t.val.s_val = strdup(s);
      break;

    /*------------------------------------------------------------------------
     * ID Parsing
     *----------------------------------------------------------------------*/
    default:
      ungetc(c, istream);
      if(fscanf(istream, "%s", s) != EOF)
      {
        if(debug("lex"))
          printf("str=%s\n", s);
        t.t         = TOK_ID;
        t.val.s_val = strdup(s);
      }
      else
      {
        reg_msg(LC_MSG{"lex", "unrecognized token", MSG_ERROR, t.offset});
        continue;
      }
      break;
    }
    TOK_PUSH(t);
    pos++;
  }

  t.t = TOK_EOF;
  TOK_PUSH(t);

  if(dump("lex"))
  {
    puts("-- lex dump:");
    TOK_ITER(i)
    dump_tok_i(i);
  }

  return pos;
}

void parse_finalize()
{
  for(int i = 1; i < tok_max; i++)
  {
    switch(TOKI(i).t)
    {
    case TOK_ID:
    case TOK_STRLIT:
      if(debug("parse"))
        printf("deallocating id token %d\n", i);
      free(TOKI(i).val.s_val);
    }
  }
}

void dump_tok()
{
  int   indent = 0;
  TOKEN t;
  int   indent_i;
#define INDENT()                                       \
  for(int indent_i = 0; indent_i < indent; indent_i++) \
    printf("  ");

  for(int i = 1; i < tok_max; i++)
  {
    t = tok_table[i];
    switch(t.t)
    {
    case TOK_LPAREN:
      INDENT();
      printf("(");
      indent++;
      break;
    case TOK_RPAREN:
      indent--;
      INDENT();
      printf(")");
      break;
    case TOK_ID:
      INDENT();
      printf("id:%s", tok_table[i].val.s_val);
      break;
    case TOK_STRLIT:
      INDENT();
      printf("str:%s", tok_table[i].val.s_val);
      break;
    case TOK_NUMLIT:
      INDENT();
      printf("num:%d", tok_table[i].val.i_val);
      break;
    case TOK_DEFVAR:
      INDENT();
      printf("defvar");
      break;
    case TOK_EOF:
    case TOK_EOL:
      continue;
    case TOK_BINOP:
      INDENT();
      printf("binop:%c", TOKI(i).val.bin);
      break;
    default:
      INDENT();
      printf("?");
    }
    puts("");
  }
#undef INDENT
}

void tok_iter(void (*visitor)(TOKEN*))
{
  TOK_ITER(i)
  {
    (*visitor)(&TOKI(i));
  }
}

/*
 * Owns: istream
 * Nonowner of return fp
 */
FILE* preproc(FILE* istream)
{
  char  c;
  FILE* iostream = tmpfile();

  // maps offsets in the source file to offsets in the preprocesses source file
  int srci = 0, proci = 0;
  if(iostream == NULL)
  {
    printf("couldnt open temp file\n");
    std::abort();
  }
  while((c = getc(istream)) != EOF)
  {
    if(c == '(' || c == ')')
    {
      preproc_providence[proci++] = srci;
      fputc(' ', iostream);
    }
    preproc_providence[proci++] = srci++;
    fputc(c, iostream);
    if(c == '(' || c == ')')
    {
      preproc_providence[proci++] = srci;
      fputc(' ', iostream);
    }
  }
  fclose(istream);

  rewind(iostream);
  return iostream;
}
