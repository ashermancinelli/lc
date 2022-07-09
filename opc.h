#pragma once

#define OPC_PROC(X) X,
enum OPC
{
#include "opc.def"
};
#undef OPC_PROC
