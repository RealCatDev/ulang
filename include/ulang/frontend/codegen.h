#ifndef   ULANG_FRONTEND_BYTECODE_H_
#define   ULANG_FRONTEND_BYTECODE_H_

typedef enum {
  ULANG_CODEGEN_BYTECODE = 0,
} ulang_codegen_kind_t;

typedef struct {
  ulang_ast_program_t program;
  Nob_String_Builder sb;
  ulang_codegen_kind_t kind;
} ulang_codegen_t;

ulang_result_t ulang_codegen_init(ulang_codegen_t *codegen, ulang_codegen_kind_t kind, ulang_parser_t *parser);
ulang_result_t ulang_codegen_gen(ulang_codegen_t *codegen);
ulang_result_t ulang_codegen_save(ulang_codegen_t *codegen, const char *file_name);
void ulang_codegen_free(ulang_codegen_t codegen);

#endif // ULANG_FRONTEND_BYTECODE_H_