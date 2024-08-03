#include "ulang/ulang.h"

bool ulang_parser_is_type(ulang_parser_t *parser, Nob_String_View name, ulang_type_t *type) {
  for (size_t i = 0; i < parser->types.count; ++i)
    if (nob_sv_eq(parser->types.items[i].name, name)) return (*type = parser->types.items[i], true);
  return false;
}

bool ulang_parser_peek(ulang_parser_t *parser, size_t offset, ulang_token_t *token) {
  if (parser->ptr-parser->tokens.items+offset >= parser->tokens.count) return false;
  if (token) *token = parser->ptr[offset];
  return true;
}

bool ulang_parser_eat(ulang_parser_t *parser, ulang_token_kind_t kind, ulang_token_t *token) {
  if (token) *token = *(parser->ptr);
  if ((parser->ptr++)->kind != kind) return false;
  return true;
}

bool ulang_parser_advance(ulang_parser_t *parser, ulang_token_t *token) {
  if (parser->ptr-parser->tokens.items >= parser->tokens.count)
    return false;
  return (token&&(token=++parser->ptr),1);
}

ulang_result_t ulang_parser_load(ulang_parser_t *parser, ulang_tokens_t tokens) {
  if (!parser || !tokens.items) return (ulang_result_t){
    .kind = ULANG_BADARG_ERROR,
    .location = (ulang_location_t){0}
  };
  memset(parser, sizeof(*parser), 0);
  memcpy(&parser->tokens, &tokens, sizeof(tokens));
  parser->ptr = parser->tokens.items;
  return (ulang_result_t){
    .kind = ULANG_SUCCESS
  };
}

ulang_result_t ulang_parser_loadl(ulang_parser_t *parser, ulang_lexer_t *lexer) {
  if (!parser || !lexer) return (ulang_result_t){
    .kind = ULANG_BADARG_ERROR,
    .location = (ulang_location_t){0}
  };
  ulang_tokens_t tokens = {0};
  ulang_result_t result = {0};
  if ((result = ulang_lexer_tokenize(lexer, &tokens)).kind != ULANG_SUCCESS) return result;
  return ulang_parser_load(parser, tokens);
}

ulang_result_t ulang_parser_parse(ulang_parser_t *parser, ulang_ast_program_t *program) {
  ulang_location_t location = {0};

  while (parser->tokens.items-parser->ptr < parser->tokens.count) {
    ulang_ast_stmt_t stmt = {0};
    ulang_result_t result = {0};
    if ((result = ulang_parser_parse_stmt(parser, &stmt)).kind != ULANG_SUCCESS) return result;
    nob_da_append(&program->body, stmt);
  }

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_stmt(ulang_parser_t *parser, ulang_ast_stmt_t *stmt) {
  ulang_token_t token = *parser->ptr;
  ulang_location_t location = token.location;

  switch (token.kind) {
  case ULANG_TOKEN_ID: {
    ulang_type_t type = {0};
    if (nob_sv_eq(token.value.as.string, SV("func"))) { // func def
      ulang_ast_stmt_func_t funcdef = {0};
      ulang_result_t result = {0};
      if ((result = ulang_parser_parse_funcdef(parser, &funcdef)).kind != ULANG_SUCCESS) return result;
      *stmt = (ulang_ast_stmt_t){
        .kind = ULANG_AST_STMT_FUNC,
        .as.func = funcdef
      };
    } else if (nob_sv_eq(token.value.as.string, SV("if"))) { // if

    } else if (ulang_parser_is_type(parser, token.value.as.string, &type)) {
      ulang_token_t tok = {0};
      if (!ulang_parser_peek(parser, 2, &tok)) return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF), .location = (ulang_location_t){0} };
      if (tok.kind == '(') { // func def
        ulang_ast_stmt_func_t funcdef = {0};
        ulang_result_t result = {0};
        if ((result = ulang_parser_parse_funcdef(parser, &funcdef)).kind != ULANG_SUCCESS) return result;
        *stmt = (ulang_ast_stmt_t){
          .kind = ULANG_AST_STMT_FUNC,
          .as.func = funcdef
        };
      } else if (tok.kind == ';' || tok.kind == '=') { // var def
        ulang_ast_stmt_vardef_t vardef = {0};
        ulang_result_t result = {0};
        if ((result = ulang_parser_parse_vardef(parser, &vardef)).kind != ULANG_SUCCESS) return result;
        *stmt = (ulang_ast_stmt_t){
          .kind = ULANG_AST_STMT_VARDEF,
          .as.vardef = vardef
        };
      } else return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN), .location = location };
    } else return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN), .location = location };
  } break;
  default: return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN), .location = location };
  }

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_expr(ulang_parser_t *parser, ulang_ast_expr_t *expr) {
  ulang_token_t token = *parser->ptr;

  switch (token.kind) {
  case ULANG_TOKEN_ID: {
    *expr = (ulang_ast_expr_t){
      .kind = ULANG_AST_EXPR_VAR,
      .as.var = token.value.as.string
    };
  } break;
  default: return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN), .location = token.location };
  }

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_scope(ulang_parser_t *parser, ulang_ast_stmt_scope_t *scope) {
  if (parser->ptr->kind == '{' && (++parser->ptr));
  *scope = (ulang_ast_stmt_scope_t){0};
  while (parser->ptr->kind != '}') {
    ulang_ast_stmt_t stmt = {0};
    ulang_result_t result = {0};
    if ((result = ulang_parser_parse_stmt(parser, &stmt)).kind != ULANG_SUCCESS) return result;
    nob_da_append(scope, stmt);
  } ++parser->ptr;

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_vardef(ulang_parser_t *parser, ulang_ast_stmt_vardef_t *vardef) {
  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_funcdef(ulang_parser_t *parser, ulang_ast_stmt_func_t *funcdef) {
  ulang_token_t type_tok = {0};
  ulang_token_t name_tok = {0};
  ulang_token_t temp_tok = {0};
  if (!ulang_parser_eat(parser, ULANG_TOKEN_ID, &type_tok)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
    .location = type_tok.location
  };
  if (!ulang_parser_eat(parser, ULANG_TOKEN_ID, &name_tok)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
    .location = name_tok.location
  };
  if (!ulang_parser_eat(parser, '(', &temp_tok)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
    .location = temp_tok.location
  };

  ulang_ast_stmt_func_t func = {0};
  func.name = name_tok.value.as.string;

  ulang_token_t tok = {0};
  ulang_token_t prev_type_tok = {0};
  ulang_result_t result = {0};
  for (bool b = false; !ulang_parser_eat(parser, ULANG_TOKEN_NONE, &tok) && tok.kind != ')'; b=true) {
    if (b) {
      if (tok.kind != ',') return (ulang_result_t){
        .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
        .location = tok.location
      }; else if (!ulang_parser_advance(parser, &tok)) return (ulang_result_t){
        .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
      };
    }
    if (tok.kind != ULANG_TOKEN_ID) return (ulang_result_t){
      .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
      .location = tok.location
    };
    ulang_token_t next_tok = {0};
    if (!ulang_parser_peek(parser, 0, &next_tok)) return (ulang_result_t){
      .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
    };
    if (next_tok.kind == ULANG_TOKEN_ID) {
      prev_type_tok = tok;
      if (!ulang_parser_advance(parser, NULL) ||
          !ulang_parser_advance(parser, &tok)) return (ulang_result_t){
        .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
      };
    }
    ulang_func_param_t param = (ulang_func_param_t){
      .name = tok.value.as.string,
      .type = prev_type_tok.value.as.string
    };
    nob_da_append(&func.params, param);
  }
  if (result.kind != ULANG_SUCCESS) return result;

  if ((result = ulang_parser_parse_scope(parser, &func.body)).kind != ULANG_SUCCESS) return result;

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

void ulang_parser_free(ulang_parser_t parser) {
  nob_da_free(parser.tokens);
  nob_da_free(parser.types);
}