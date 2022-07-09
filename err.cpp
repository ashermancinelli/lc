#include "err.h"
#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>

static std::vector<LC_MSG> errs;

void reg_msg(LC_MSG err)
{
  errs.push_back(err);
  if (err.lvl == MSG_FATAL)
    std::exit(EXIT_FAILURE);
}

void err_printer()
{
  FILE* fp = fopen(infile().c_str(), "r");
  for (auto const& e : errs)
  {
    if (e.lvl == MSG_INFO and !info())
      continue;
    std::string lvl;
    switch (e.lvl)
    {
#define MSG_PROC(E) case E: lvl = #E; break;
#include "err.def"
#undef MSG_PROC
      default:
        lvl = "?";
    }
    printf("LC(%s):%s:%s", lvl.c_str(), e.phase.c_str(), e.msg.c_str());
    if (e.offset > 0)
    {
      printf(":offset=%d",e.offset);
      int start=0, end=0;
      char* line=nullptr;
      size_t len, last_offset = 0, lineno = 0;
      while (getline(&line, &len, fp) >= 0)
      {
        if (ftell(fp) >= e.offset)
          break;
        last_offset = ftell(fp);
        lineno++;
      }
      int col = e.offset - last_offset;
      int linelen = strlen(line);
      line[linelen-1] = 0;
      printf("\n%s:%zu:%d:\n%s\n", infile().c_str(), lineno, col, line);
      for (int i=0; i < col-1; i++)
        printf("~");
      printf("^");
      free(line);
    }
    puts("");
  }

  if (any_errors())
  {
    puts("LC: fatal errors occured. lc did not successfully run to completion.");
  }
  fclose(fp);
}

bool any_errors()
{
  for (auto const& e : errs)
  {
    if (e.lvl == MSG_ERROR or e.lvl == MSG_FATAL)
      return true;
  }
  return false;
}
