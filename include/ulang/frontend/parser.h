#ifndef   ULANG_FRONTEND_PARSER_H_
#define   ULANG_FRONTEND_PARSER_H_

// <ERRORS>
enum {
  ULANG_UNEXPECTED_TOKEN = 0,
  ULANG_UNEXPECTED_EOF
};
// </ERRORS>

typedef struct {
  ulang_type_t *items;
  size_t count;
  size_t capacity;
} ulang_types_t;

typedef struct {
  ulang_tokens_t tokens;
  ulang_token_t *ptr;
  ulang_types_t types;
} ulang_parser_t;

ulang_result_t ulang_parser_load(ulang_parser_t *parser, ulang_tokens_t tokens);
ulang_result_t ulang_parser_loadl(ulang_parser_t *parser, ulang_lexer_t *lexer);
ulang_result_t ulang_parser_parse(ulang_parser_t *parser,
                                  ulang_ast_program_t *program);
ulang_result_t ulang_parser_parse_stmt(ulang_parser_t *parser, ulang_ast_stmt_t *stmt);
ulang_result_t ulang_parser_parse_expr(ulang_parser_t *parser, ulang_ast_expr_t *expr);
ulang_result_t ulang_parser_parse_scope(ulang_parser_t *parser, ulang_ast_stmt_scope_t *scope);
ulang_result_t ulang_parser_parse_vardef(ulang_parser_t *parser, ulang_ast_stmt_vardef_t *vardef);
ulang_result_t ulang_parser_parse_funcdef(ulang_parser_t *parser, ulang_ast_stmt_func_t *funcdef);
void ulang_parser_free(ulang_parser_t parser);

#endif // ULANG_FRONTEND_PARSER_H_