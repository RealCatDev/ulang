#include "ulang/ulang.h"

#include <ctype.h>

bool equals_compatible(char c) {
  return c == '=' || c == '!' || c == '<' || c == '>'
      || c == '+' || c == '-' || c == '*' || c == '/';
}

ulang_result_t ulang_lexer_load(ulang_lexer_t *lexer, Nob_String_View content) {
  if (!lexer) return (ulang_result_t){
    .kind = ULANG_BADARG_ERROR,
    .location = (ulang_location_t){0}
  };
  memset(lexer, sizeof(*lexer), 0);
  lexer->content = content.data;
  lexer->content_size = content.count;
  lexer->ptr = lexer->prev_line = content.data;
  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_lexer_loadf(ulang_lexer_t *lexer, const char *file_path) {
  Nob_String_Builder sb = {0};
  if (!nob_read_entire_file(file_path, &sb)) return (ulang_result_t){
    .kind = ULANG_FS_ERROR,
    .location = (ulang_location_t){0}
  }; 
  return ulang_lexer_load(lexer, (Nob_String_View){ .data = sb.items, .count = sb.count });
}

ulang_result_t ulang_lexer_tokenize(ulang_lexer_t *lexer, ulang_tokens_t *tokens) {
  if (!lexer || !lexer->ptr || !tokens) return (ulang_result_t){ .kind = ULANG_BADARG_ERROR, .location = (ulang_location_t){0} };
  memset(tokens, sizeof(*tokens), 0);

  ulang_location_t location = {0};
  uint32_t data = 0;
  while (*lexer->ptr
      &&  lexer->ptr-lexer->content < lexer->content_size) {
    while (*lexer->ptr && isspace(*lexer->ptr) && ((*lexer->ptr=='\n'&&++lexer->row)||true) && ++lexer->ptr);
    const char *start = NULL;
    location = (ulang_location_t){
      .row = lexer->row,
      .col = lexer->ptr-lexer->prev_line
    };
    char c = *(start = (lexer->ptr++));

    if (c == '-' || c == '.' || isdigit(c)) {
      data = ULANG_LEXER_NUM_ERROR;
      ulang_token_t token = {0};
      token.location = location;

      uint8_t byte = (*start == '-') | ((*start == '.')<<1);
      while (*lexer->ptr && lexer->ptr-lexer->content < lexer->content_size) {
        c = tolower(*lexer->ptr);
        if (c == '.' && (byte & 2)) break;
        if (c == 'f') { if (byte & 4) break; }
        else if (c != 'x' && !isdigit(c)) break;
        ++lexer->ptr;
        if (c == '.') byte |= 2;
        else if (c == 'f') { byte |= 4; break; }
      }

      token.kind = ULANG_TOKEN_INT;
      if (byte == 0) { // uint
        token.value.kind = ULANG_VALUE_UINT;
        char *end = NULL;
        token.value.as.unsigned_int = strtoull(start, &end, 0);
        if (end != lexer->ptr) goto error;
      } else if (byte == 1) { // int
        token.value.kind = ULANG_VALUE_INT;
        char *end = NULL;
        token.value.as.unsigned_int = strtoll(start, &end, 0);
        if (end != lexer->ptr) goto error;
      } else if ((byte & 6) == 6) {
        token.value.kind = ULANG_VALUE_FLOAT;
        char *end = NULL;
        float ret = strtof(start, &end);
        token.value.as.floating_point = ret;
        if (end != lexer->ptr-1) goto error;
      } else if (byte & 2) {
        token.value.kind = ULANG_VALUE_DOUBLE;
        char *end = NULL;
        token.value.as.double_precission = strtod(start, &end);
        if (end != lexer->ptr) goto error;
      }
      
      nob_da_append(tokens, token);
    } else if (isalnum(c)) {
      data = ULANG_LEXER_IDENT_ERROR;

      ulang_token_t token = {0};
      token.location = location;
      token.kind = ULANG_TOKEN_ID;
      token.value.kind = ULANG_VALUE_STRING;

      while (isalnum(*lexer->ptr) && lexer->ptr-lexer->content < lexer->content_size && ++lexer->ptr);
      token.value.as.string = (Nob_String_View){ .data = start, .count = lexer->ptr-start };

      nob_da_append(tokens, token);
    } else {
      ulang_token_t token = {0};
      token.location = location;

      if (equals_compatible(c) && *lexer->ptr == '=') {
        token.kind = '=';
        token.value.kind = ULANG_VALUE_STRING;
        token.value.as.string = (Nob_String_View){ .data = lexer->ptr-1, .count = 1 };
        ++lexer->ptr;
      } else token.kind = c;
     
      nob_da_append(tokens, token);
    }
  }

  return (ulang_result_t){ .kind = ULANG_SUCCESS, .location = location };
error:
  return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_LEXER_ERROR, data),
    .location = location
  };
}

void ulang_lexer_free(ulang_lexer_t lexer) {
  free((char*)lexer.content);
}