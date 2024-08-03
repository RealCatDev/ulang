#ifndef   ULANG_FRONTEND_LEXER_H_
#define   ULANG_FRONTEND_LEXER_H_

// <ERRORS>
enum {
  ULANG_LEXER_IDENT_ERROR = 0,
  ULANG_LEXER_NUM_ERROR,
};
// </ERRORS>

typedef enum {
  ULANG_TOKEN_ID   = -1,
  ULANG_TOKEN_INT  = -2,
  ULANG_TOKEN_NONE = -3,
} ulang_token_kind_t;

typedef struct {
  ulang_location_t location;
  ulang_token_kind_t kind;
  ulang_value_t value;
} ulang_token_t;

typedef struct {
  ulang_token_t *items;
  size_t count;
  size_t capacity;
} ulang_tokens_t;

typedef struct {
  const char *content;
  size_t content_size, row;
  const char *ptr, *prev_line;
} ulang_lexer_t;

ulang_result_t ulang_lexer_load(ulang_lexer_t *lexer, Nob_String_View content);
ulang_result_t ulang_lexer_loadf(ulang_lexer_t *lexer, const char *file_path);
ulang_result_t ulang_lexer_tokenize(ulang_lexer_t *lexer, ulang_tokens_t *tokens);
void ulang_lexer_free(ulang_lexer_t lexer);

#endif // ULANG_FRONTEND_LEXER_H_