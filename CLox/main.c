#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linenoise.h"

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "membufw.h"
#include "memory.h"
#include "vm.h"

#define LOX_VERSION_STRING "0.0.241106"

#define STR(x) #x
#define XSTR(x) STR(x)

static void printVersion(FILE* fout) {
  fputs("clox " XSTR(LOX_VERSION_STRING) "\n", fout);
}

static void repl(int argc, const char* argv[]) {
  VM vm;
  initVM(&vm, stdout, stderr);
  argsVM(&vm, argc, argv);

  const char prefix[] = "print";
  const size_t inputStart = sizeof(prefix) - 1;

  MemBuffer src;
  initMemBuffer(&src);
  fputs(prefix, src.fptr);

  linenoiseSetMultiLine(1);
  linenoiseHistorySetMaxLen(100);

  printf("clox %s (d0c5c90) type .help for more information\n", LOX_VERSION_STRING);
  
  for (;;) {
    const char* prompt = ftell(src.fptr) > inputStart ? "" : "\x1b[1;32mlox\x1b[0m> ";
    char* line = linenoise(prompt);

    size_t len = strlen(line);
    if (len == 0) {
        free(line);
        continue;
    } else if (len >= 1 && line[len - 1] == '\\') {
        fprintf(src.fptr, "%.*s\n", (int)len- 1, line);
    } else if (strcmp(".q", line) == 0) {
        freeVM(&vm);
        freeMemBuffer(&src);
        free(line);
        break;
    } else if (strcmp(".help", line) == 0) {
        printf("Lox language implementation in C (https://github.com/lbmonsalve/CLox)\n\n");
        printf("Commands:\n\n");
        printf("  .help         this info\n");
        printf("  .quit         quit (or .q)\n\n");
        printf("LICENCE and COPYRIGHT on github site.\n\n");
        free(line);
        continue;
    } else {
        fprintf(src.fptr, "%.*s\n", (int)len, line);

        if (src.buf[inputStart] == '=') {
            src.buf[inputStart] = ' ';
            fputc(';', src.fptr);

            src.buf[ftell(src.fptr)] = '\0';
            interpret(&vm, src.buf);
        } else {
            src.buf[ftell(src.fptr)] = '\0';
            interpret(&vm, src.buf + inputStart);
        }

        freeMemBuffer(&src);
        initMemBuffer(&src);
        fputs(prefix, src.fptr);
    }

    linenoiseHistoryAdd(line);
    free(line);
  }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path, int argc, const char* argv[]) {
  VM vm;
  InterpretResult result;

  initVM(&vm, stdout, stderr);
  argsVM(&vm, argc, argv);
  char* source = readFile(path);
  result = interpret(&vm, source);
  freeVM(&vm);

  if (result == INTERPRET_COMPILE_ERROR) {
    exit(65);
  }
  if (result == INTERPRET_RUNTIME_ERROR) {
    exit(70);
  }
}

static void printHelp(FILE* fout) {
  printVersion(fout);
  fputs(
      "\n"
      "Usage: clox [options] [path]\n"
      "\n"
      "   -D, --dump\t\t(debug) Dump disassembled script\n"
      "   -T, --trace\t\t(debug) Trace script execution\n"
      "   -L, --log-gc\t\t(debug) Log garbage collector\n"
      "   -S, --stress-gc\t(debug) Always collect garbage\n"
      "   -h, -?, --help\tShow help (this message) and exit\n"
      "   -v, --version\tShow version information and exit\n",
      fout);
}

int main(int argc, const char* argv[]) {
  const char* argv0 = argv[0];
  const char* script = NULL;

  while (argc > 1) {
    if (!strcmp(argv[1], "--")) {
      argv++;
      argc--;
      break;
    } else if (!strcmp(argv[1], "--version")) {
      printVersion(stdout);
      return 0;
    } else if (!strcmp(argv[1], "--help")) {
      printHelp(stdout);
      return 0;
    } else if (!strcmp(argv[1], "--dump")) {
      debugPrintCode = true;
    } else if (!strcmp(argv[1], "--trace")) {
      debugTraceExecution = true;
    } else if (!strcmp(argv[1], "--log-gc")) {
      debugLogGC = true;
    } else if (!strcmp(argv[1], "--stress-gc")) {
      debugStressGC = true;
    } else if (!strncmp(argv[1], "--", 2)) {
      fprintf(stderr, "Unknown option: '%s'\n", argv[1]);
      printHelp(stderr);
      return 1;
    } else if (argv[1][0] == '-') {
      if (!argv[1][1]) {
        script = argv[1];
        break;
      }
      for (const char* a = argv[1] + 1; *a; ++a) {
        switch (*a) {
          case 'v': printVersion(stdout); return 0;
          case '?':
          case 'h': printHelp(stdout); return 0;
          case 'D': debugPrintCode = true; break;
          case 'T': debugTraceExecution = true; break;
          case 'L': debugLogGC = true; break;
          case 'S': debugStressGC = true; break;
          default:
            fprintf(stderr, "Unknown option: '%c'\n", *a);
            printHelp(stderr);
            return 1;
        }
      }
    } else {
      script = argv[1];
      break;
    }
    argv++;
    argc--;
  }

  argv[0] = argv0;

  if (script) {
    runFile(script, argc, argv);
  } else {
    repl(argc, argv);
  }

  return 0;
}
