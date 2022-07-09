#include "config.h"
#include "err.h"

void help()
{
  puts("lc: help");
  puts("-o");
  puts("\t\tfile LLVM IR will be sent to");
  puts("-fdump-<phase>");
  puts("\t\tdump all info from phase <phase>");
  puts("\t\tpossible phases include:");
  puts("\t\t\tpreproc, tok, lex, parse, parse-sexpr, ast, ast1, lower");
  puts("-fsyntax-only");
  puts("\t\tstop compilation after parse");
  // puts("-i --interactive");
  // puts("\t\tstart lc in interactive mode");
}

static struct
{
  std::string              infile = "", outfile = "";
  std::vector<std::string> dumps;
  std::vector<std::string> debugs;
  bool                     dumpall    = false;
  bool                     debugall   = false;
  bool                     info       = false;
  bool                     repl       = false;
  bool                     syntaxonly = false;
} opts;

void parse_opts(int argc, char** argv)
{
  if(argc < 2)
  {
    help();
    std::exit(EXIT_FAILURE);
  }
  std::vector<std::string> args(argv, argv + argc);
  auto                     it = args.begin();
  it++;
  while(it != args.end())
  {
    if(*it == "-help" || *it == "--help")
    {
      help();
      std::exit(EXIT_FAILURE);
    }
    else if (*it == "-o")
    {
      it++;
      if (it == args.end())
        reg_msg(LC_MSG{"argparse", "expected argument for '-o'", MSG_FATAL});
      opts.outfile = *it;
    }
    else if((*it).starts_with("-fdump-"))
    {
      const auto d = (*it).substr(7);
      if(d == "all")
        opts.dumpall = true;
      else
        opts.dumps.push_back(d);
    }
    else if((*it).starts_with("-fdebug-"))
    {
      const auto d = (*it).substr(8);
      if(d == "all")
        opts.debugall = true;
      else
        opts.debugs.push_back(d);
    }
    else if (*it == "-fsyntax-only")
      opts.syntaxonly = true;
    else if((*it).starts_with("-finfo"))
      opts.info = true;
    else if(*it == "-i" or *it == "--interactive")
      opts.repl = true;
    else
      opts.infile = *it;
    it++;
  }
}

bool dump(std::string_view sv)
{
  return opts.dumpall || std::find(opts.dumps.begin(), opts.dumps.end(), sv) != opts.dumps.end();
}

bool debug(std::string_view sv)
{
  return opts.debugall ||
         std::find(opts.debugs.begin(), opts.debugs.end(), sv) != opts.debugs.end();
}

bool info()
{
  return opts.info;
}

bool repl()
{
  return opts.repl;
}

bool syntaxonly()
{
  return opts.syntaxonly;
}

std::string infile()
{
  return opts.infile;
}
std::string outfile()
{
  return opts.outfile;
}
