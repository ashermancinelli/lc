#include "config.h"
#include "err.h"
#include <algorithm>
#include <cctype>

void help() {
  puts("lc: help");
  puts("-o");
  puts("\t\tfile compiler output will be written to");
  puts("-arch <arch>");
  puts("\t\tarchitecture output type. Valid values are:");
  puts("\t\t\tllvm (default), asm, native");
  puts("-fdump-<phase>");
  puts("\t\tdump all info from phase <phase>");
  puts("\t\tpossible phases include:");
  puts("\t\t\tpreproc, tok, lex, parse, parse-sexpr, ast, ast1, lower");
  puts("-info");
  puts("\t\tprint extra information about compilation phases");
  puts("-fsyntax-only");
  puts("\t\tstop compilation after parse");
  puts("-llvm <path>");
  puts("\t\tpath to llvm toolchain to be used internally (default: /usr)");
  puts("-O<level>");
  puts("\t\tuse optimization level <level> when invoking clang (default: -O0)");
}

static struct {
  std::string infile = "", outfile = "", llvmroot = "/usr", lvl = "-O0";
  std::vector<std::string> dumps;
  std::vector<std::string> debugs;
  bool dumpall = false;
  bool debugall = false;
  bool info = false;
  bool repl = false;
  bool syntaxonly = false;
  ARCH arch = ARCH::LLVM;
} opts;

void parse_opts(int argc, char **argv) {
  if (argc < 2) {
    help();
    std::exit(EXIT_FAILURE);
  }
  std::vector<std::string> args(argv, argv + argc);
  auto it = args.begin();
  it++;
  while (it != args.end()) {
    if (*it == "-help" || *it == "--help") {
      help();
      std::exit(EXIT_FAILURE);
    } else if (*it == "-o") {
      it++;
      if (it == args.end())
        reg_msg(LC_MSG{"argparse", "expected argument for '-o'", MSG_FATAL});
      opts.outfile = *it;
    } else if (*it == "-llvm") {
      it++;
      if (it == args.end()) {
        puts("option '-llvm' requires an argument");
        std::exit(EXIT_FAILURE);
      }
      opts.llvmroot = *it;
    } else if ((*it).starts_with("-O")) {
      opts.lvl = *it;
    } else if (*it == "-arch") {
      it++;
      if (it == args.end()) {
        puts("option '-arch' requires an argument");
        std::exit(EXIT_FAILURE);
      }
      std::string arch = *it;
      std::transform(arch.begin(), arch.end(), arch.begin(),
                     [](char c) { return std::tolower(c); });
      if (arch == "asm")
        opts.arch = ARCH::ASM;
      else if (arch == "llvm")
        opts.arch = ARCH::LLVM;
      else if (arch == "native")
        opts.arch = ARCH::NATIVE;
      else {
        std::string err =
            "expected -arch to be one of llvm, asm, or native, but got " + arch;
        printf("%s\n", err.c_str());
        std::exit(EXIT_FAILURE);
      }
    } else if ((*it).starts_with("-fdump-")) {
      const auto d = (*it).substr(7);
      if (d == "all")
        opts.dumpall = true;
      else
        opts.dumps.push_back(d);
    } else if ((*it).starts_with("-fdebug-")) {
      const auto d = (*it).substr(8);
      if (d == "all")
        opts.debugall = true;
      else
        opts.debugs.push_back(d);
    } else if (*it == "-fsyntax-only")
      opts.syntaxonly = true;
    else if ((*it).starts_with("-info"))
      opts.info = true;
    else if (*it == "-i" or *it == "--interactive")
      opts.repl = true;
    else
      opts.infile = *it;
    it++;
  }
}

bool dump(std::string_view sv) {
  return opts.dumpall || std::find(opts.dumps.begin(), opts.dumps.end(), sv) !=
                             opts.dumps.end();
}

bool debug(std::string_view sv) {
  return opts.debugall || std::find(opts.debugs.begin(), opts.debugs.end(),
                                    sv) != opts.debugs.end();
}

bool info() { return opts.info; }

bool repl() { return opts.repl; }

bool syntaxonly() { return opts.syntaxonly; }

std::string infile() { return opts.infile; }
std::string outfile() { return opts.outfile; }

std::string llvmroot() { return opts.llvmroot; }

std::string optlevel() { return opts.lvl; }

ARCH arch() { return opts.arch; }
