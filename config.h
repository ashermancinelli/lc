#pragma once
#include <cstddef>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

void help();
void parse_opts(int argc, char** argv);
bool dump(std::string_view sv);
bool debug(std::string_view sv);
bool info();
bool repl();
bool syntaxonly();
std::string outfile();
std::string infile();
std::string llvmroot();
std::string optlevel();

enum class TARGET {
  LLVM,
  ASM,
  NATIVE,
  INTERPRET,
};

TARGET target();
