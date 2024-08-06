#define NOB_IMPLEMENTATION
#include "ulang/ulang.h"

int main(int argc, char **argv) {
  ulang_lexer_t lexer = {0};
  ulang_result_t result = {0};
  if ((result = ulang_lexer_loadf(&lexer, "../test/test.ulang")).kind != ULANG_SUCCESS) return 1;

  ulang_parser_t parser = {0};
  if ((result = ulang_parser_loadl(&parser, &lexer)).kind != ULANG_SUCCESS) return 1;

  ulang_codegen_t codegen = {0};
  if ((result = ulang_codegen_init(&codegen, ULANG_CODEGEN_BYTECODE, &parser)).kind != ULANG_SUCCESS) return 1;
  if ((result = ulang_codegen_gen(&codegen)).kind != ULANG_SUCCESS) return 1;
  if ((result = ulang_codegen_save(&codegen, "../test/test.ubyte")).kind != ULANG_SUCCESS) return 1;

  ulang_codegen_free(codegen);
  ulang_parser_free(parser);
  ulang_lexer_free(lexer);

  return 0;
}