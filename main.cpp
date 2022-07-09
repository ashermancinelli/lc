#include <cstdlib>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "config.h"
#include "err.h"
#include "lower.h"
#include "parse.h"
#include "sema.h"

namespace fs = std::filesystem;

// Get the input file without a file extension
std::string infile_noext() {
  auto inf = infile();

  auto dot = inf.rfind('.');
  inf.erase(dot);

  auto slash = inf.rfind('/');
  inf.erase(0, slash + 1);

  return inf;
}

void emit_llvm() {
  if (outfile().size()) {
    std::error_code ec;
    raw_fd_ostream os{outfile(), ec};
    if (ec)
      reg_msg(
          LC_MSG{"lower", "could not open output file for writing", MSG_FATAL});
    else
      get_module().print(os, nullptr);
  } else {
    auto of = infile_noext();
    of += ".ll";
    if (info())
      printf("writing output to %s\n", of.c_str());
    std::error_code ec;
    raw_fd_ostream os{of, ec};
    if (ec)
      reg_msg(
          LC_MSG{"lower", "could not open output file for writing", MSG_FATAL});
    get_module().print(os, nullptr);
  }
}

std::string suuid() {
  uuid_t uuid;
  uuid_generate_time_safe(uuid);
  char id[37];
  uuid_unparse_lower(uuid, id);
  return std::string(id);
}

void emit_asm() {
  char tmpfile[1024];
  sprintf(tmpfile, "/tmp/lc-llvm-%s.ll", suuid().c_str());

  if (info())
    printf("writing llvm ir to temporary file %s\n", tmpfile);

  std::error_code ec;
  raw_fd_ostream os{tmpfile, ec};
  if (ec)
    reg_msg(LC_MSG{"asm", "could not open output file for writing", MSG_FATAL});
  else
    get_module().print(os, nullptr);

  std::string of;
  if ((of = outfile()) == "") {
    of = infile_noext() + ".s";
  }

  if (info())
    printf("writing asm output to %s\n", of.c_str());

  pid_t pid;
  pid = fork();

  if (pid == 0) // child builds asm
  {
    auto clang = llvmroot() + "/bin/clang";
    auto ol = optlevel();
    char *const argv[] = {clang.data(), "-Wno-override-module",
                          ol.data(),    "-S",
                          tmpfile,      "-o",
                          of.data(),    NULL};
    if (info()) {
      puts("exec'ing the following command:");
      int i = 0;
      while (argv[i] != NULL) {
        printf("%s ", argv[i++]);
      }
      puts("");
    }
    execv(argv[0], argv);
    std::exit(0);
  }

  while (wait(NULL) > 0)
    ;
  if (!fs::exists(fs::path(of))) {
    char msg[1024];
    sprintf(msg, "clang asm generation failed");
    reg_msg(LC_MSG{"asm", msg, MSG_FATAL});
  }
}

void emit_native() {
  char tmpfile[1024];
  sprintf(tmpfile, "/tmp/lc-llvm-%s.ll", suuid().c_str());

  if (info())
    printf("writing llvm ir to temporary file %s\n", tmpfile);

  std::error_code ec;
  raw_fd_ostream os{tmpfile, ec};
  if (ec)
    reg_msg(
        LC_MSG{"native", "could not open output file for writing", MSG_FATAL});
  else
    get_module().print(os, nullptr);

  std::string of;
  if ((of = outfile()) == "") {
    of = "a.out";
  }

  if (info())
    printf("writing asm output to %s\n", of.c_str());

  pid_t pid;
  pid = fork();

  if (pid == 0) // child builds asm
  {
    auto clang = llvmroot() + "/bin/clang";
    auto ol = optlevel();
    char *const argv[] = {clang.data(), "-Wno-override-module",
                          ol.data(),    tmpfile,
                          "-o",         of.data(),
                          NULL};
    if (info()) {
      puts("exec'ing the following command:");
      int i = 0;
      while (argv[i] != NULL) {
        printf("%s ", argv[i++]);
      }
      puts("");
    }
    execv(argv[0], argv);
    std::exit(0);
  }

  while (wait(NULL) > 0)
    ;
  if (!fs::exists(fs::path(of))) {
    char msg[1024];
    sprintf(msg, "clang native generation failed");
    reg_msg(LC_MSG{"asm", msg, MSG_FATAL});
  }
}

int main(int argc, char **argv) {
  parse_opts(argc, argv);

  atexit(err_printer);
  atexit(parse_finalize);

  FILE *fp = fopen(infile().c_str(), "r");
  fp = preproc(fp);
  if (dump("preproc")) {
    puts("-- preproc dump:");
    char c;
    while ((c = getc(fp)) != EOF)
      putc(c, stdout);
    rewind(fp);
  }

  lex(fp);
  fclose(fp);

  if (any_errors())
    std::exit(EXIT_FAILURE);

  if (dump("tok")) {
    puts("-- parse tok:");
    dump_tok();
  }

  lex_sema();

  auto m = parse();

  if (dump("ast")) {
    puts("-- ast first-pass");
    m->print(0);
  }

  sema_builtins(m);
  if (dump("ast1")) {
    puts("-- ast after subsitution");
    m->print(0);
  }

  if (syntaxonly())
    goto cleanup;

  lower(m);

  switch (arch()) {
  case ARCH::LLVM:
    emit_llvm();
    break;
  case ARCH::ASM:
    emit_asm();
    break;
  case ARCH::NATIVE:
    emit_native();
    break;
  }

cleanup:
  if (any_errors())
    std::exit(EXIT_FAILURE);

  return 0;
}
