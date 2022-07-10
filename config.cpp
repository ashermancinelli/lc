#include "config.h"
#include "err.h"
#include <algorithm>
#include <cctype>

void help() {
  puts("lc: help");
  puts("-o");
  puts("\t\tfile compiler output will be written to");
  puts("-target <target>");
  puts("\t\ttarget output type. Valid values are:");
  puts("\t\t\tinterpret (default), llvm, asm, native");
  puts("\t\tnote: this implies only temporary files will be generated and the ");
  puts("\t\t-o argument will be ignored if used.");
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
  TARGET target = TARGET::INTERPRET;
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
    } else if (*it == "-i" or *it == "-interpret") {
      opts.target = TARGET::INTERPRET;
    } else if ((*it).starts_with("-O")) {
      opts.lvl = *it;
    } else if (*it == "-target") {
      it++;
      if (it == args.end()) {
        puts("option '-target' requires an argument");
        std::exit(EXIT_FAILURE);
      }
      std::string target = *it;
      std::transform(target.begin(), target.end(), target.begin(),
                     [](char c) { return std::tolower(c); });
      if (target == "asm")
        opts.target = TARGET::ASM;
      else if (target == "llvm")
        opts.target = TARGET::LLVM;
      else if (target == "native")
        opts.target = TARGET::NATIVE;
      else if (target == "interpret")
        opts.target = TARGET::INTERPRET;
      else {
        std::string err =
            "expected -target to be one of llvm, asm, or native, but got " + target;
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

TARGET target() { return opts.target; }
