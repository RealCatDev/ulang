#define NOB_IMPLEMENTATION
#include "ulang/ulang.h"

int main(int argc, char **argv) {
  ulang_lexer_t lexer = {0};
  ulang_result_t result = {0};
  if ((result = ulang_lexer_loadf(&lexer, "../test/test.ulang")).kind != ULANG_SUCCESS) return 1;

  ulang_parser_t parser = {0};
  if ((result = ulang_parser_loadl(&parser, &lexer)).kind != ULANG_SUCCESS) return 1;

  ulang_ast_program_t program = {0};
  if ((result = ulang_parser_parse(&parser, &program)).kind != ULANG_SUCCESS) return 1;

  ulang_lexer_free(lexer);

  return 0;
}