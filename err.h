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

#define LCASSERT(msg, cond) \
  if(!(cond))               \
    reg_msg(LC_MSG{"ICE", msg, MSG_FATAL});

void reg_msg(LC_MSG err);
void err_printer();
bool any_errors();
