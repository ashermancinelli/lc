#pragma once
#include <string>

enum MSGLVL
{
#define MSG_PROC(X) X,
#include "err.def"
#undef MSG_PROC
};

struct LC_MSG
{
  std::string phase;
  std::string msg;
  MSGLVL      lvl;
  int         offset = -1;
};

#define LCASSERT_P(ph, msg, cond) \
  if(!(cond))                     \
    reg_msg(LC_MSG{ph, msg, MSG_FATAL});

#define LCASSERT(msg, cond) LCASSERT_P("internal compiler error", msg, (cond));

void reg_msg(LC_MSG err);
void err_printer();
bool any_errors();
