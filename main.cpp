#include "lower.h"
#include "parse.h"
#include "config.h"
#include "err.h"
#include "sema.h"

int main(int argc, char** argv)
{
  parse_opts(argc, argv);

  atexit(err_printer);
  atexit(parse_finalize);

  FILE* fp = fopen(infile().c_str(), "r");
  fp       = preproc(fp);
  if(dump("preproc"))
  {
    puts("-- preproc dump:");
    char c;
    while((c = getc(fp)) != EOF)
      putc(c, stdout);
    rewind(fp);
  }

  lex(fp);
  fclose(fp);

  if(any_errors())
    std::exit(EXIT_FAILURE);

  if(dump("tok"))
  {
    puts("-- parse tok:");
    dump_tok();
  }

  lex_sema();

  auto m = parse();

  if(dump("ast"))
  {
    puts("-- ast first-pass");
    m->print(0);
  }

  sema_builtins(m);
  if(dump("ast1"))
  {
    puts("-- ast after subsitution");
    m->print(0);
  }

  lower(m);

  if(any_errors())
    std::exit(EXIT_FAILURE);

  return 0;
}
