#include "ulang/ulang.h"

void ulang_ast_program_find_or_append_literal(ulang_ast_program_t *program, ulang_ast_lit_t literal, size_t *index) {
  for (size_t i = 0; i < program->literals.count; ++i) {
    ulang_ast_lit_t it = program->literals.items[i];
    if (ulang_value_eq(it, literal)) {
      if (index) *index = i;
      return;
    }
  }
  *index = program->literals.count;
  nob_da_append(&program->literals, literal);
}

bool ulang_ast_program_find_type(ulang_ast_program_t *program, Nob_String_View sv, size_t *index) {
  for (size_t i = 0; i < program->types.count; ++i) {
    ulang_type_t it = program->types.items[i];
    if (nob_sv_eq(it.name, sv)) {
      if (index) *index = i;
      return true;
    }
  }
  return false;
}

bool ulang_parser_is_variable_taken(ulang_parser_t *parser, Nob_String_View name, ulang_variable_t *var) {
  for (size_t i = 0; i < parser->variables.count; ++i) {
    ulang_variable_t variable = parser->variables.items[i];
    if (nob_sv_eq(variable.name, name)) {
      if (var) *var=variable;
      return true;
    }
  }
  return false;
}

bool ulang_parser_peek(ulang_parser_t *parser, size_t offset, ulang_token_t *token) {
  if (parser->ptr-parser->tokens.items+offset >= parser->tokens.count) return false;
  if (token) *token = *(parser->ptr+offset);
  return true;
}

ulang_result_t ulang_parser_eat(ulang_parser_t *parser, ulang_token_kind_t kind, ulang_token_t *token) {
  if (parser->ptr-parser->tokens.items>=parser->tokens.count) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
  };
  if (token) *token = *(parser->ptr);
  if (kind != ULANG_TOKEN_NONE && (parser->ptr++)->kind != kind) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
    .location = ((ulang_token_t*)parser->ptr-1)->location
  };
  return (ulang_result_t){
    .kind = ULANG_SUCCESS
  };
}

bool ulang_parser_advance(ulang_parser_t *parser, ulang_token_t *token) {
  if (parser->ptr-parser->tokens.items+1>=parser->tokens.count)
    return false;
  return (++parser->ptr, token&&(*token=*parser->ptr,1),1);
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
  if (!program) return (ulang_result_t){.kind = ULANG_BADARG_ERROR};
  memset(program, sizeof(*program), 0);

  ulang_type_t intType = (ulang_type_t){
    .name = SV("int"),
    .size = 4
  };
  nob_da_append(&program->types, intType);

  ulang_location_t location = {0};

  while (parser->ptr-parser->tokens.items < parser->tokens.count) {
    ulang_ast_toplevel_t toplevel = {0};
    ulang_result_t result = {0};
    if ((result = ulang_parser_parse_toplevel(parser, &toplevel, program)).kind != ULANG_SUCCESS) return result;
    nob_da_append(&program->body, toplevel);
  }

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_toplevel(ulang_parser_t *parser, ulang_ast_toplevel_t *toplevel, ulang_ast_program_t *program) {
  ulang_token_t token = {0};
  if (!ulang_parser_peek(parser, 0, &token)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
  };
  switch (token.kind) {
  case ULANG_TOKEN_ID: {
    bool func_only = false;
    if (ulang_ast_program_find_type(program, token.value.as.string, NULL) || (func_only = nob_sv_eq(token.value.as.string, SV("func")))) {
      ulang_token_t tok = {0};
      if (!ulang_parser_peek(parser, 2, &tok)) return (ulang_result_t){
        .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
      };
      if (tok.kind == '(') { // Function def
        *toplevel = (ulang_ast_toplevel_t){
          .kind = ULANG_AST_TOPLEVEL_FUNCDEF,
        };
        ulang_result_t result = ulang_parser_parse_funcdef(parser, &toplevel->as.funcdef, program);
        if (result.kind != ULANG_SUCCESS) return result;
      } else if (!func_only && (tok.kind == ';' || tok.kind == '=')) { // Var def
        *toplevel = (ulang_ast_toplevel_t){
          .kind = ULANG_AST_TOPLEVEL_VARDEF
        };
        ulang_result_t result = ulang_parser_parse_vardef(parser, &toplevel->as.vardef, program);
        if (result.kind != ULANG_SUCCESS) return result;
      } else {
        return (ulang_result_t){
          .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
          .location = tok.location
        };
      }
    } else {
      return (ulang_result_t){
        .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
        .location = token.location
      };
    }
  } break;
  default: {
    return (ulang_result_t){
      .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
      .location = token.location
    };
  }
  }

  return (ulang_result_t){
    .kind = ULANG_SUCCESS
  };
}

ulang_result_t ulang_parser_parse_funcdef(ulang_parser_t *parser, ulang_ast_stmt_func_t *funcdef, ulang_ast_program_t *program) {
  ulang_token_t type_tok = {0};
  ulang_token_t name_tok = {0};
  ulang_token_t temp_tok = {0};

  memset(funcdef, sizeof(*funcdef), 0);

  ulang_result_t result = {0};
  if ((result = ulang_parser_eat(parser, ULANG_TOKEN_ID, &type_tok)).kind != ULANG_SUCCESS) return result;
  if (!(funcdef->has_type = ulang_ast_program_find_type(program, type_tok.value.as.string, &funcdef->type)) &&
      !nob_sv_eq(type_tok.value.as.string, SV("func"))) return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_TYPE_NOT_FOUND), .location = type_tok.location};
  if ((result = ulang_parser_eat(parser, ULANG_TOKEN_ID, &name_tok)).kind != ULANG_SUCCESS) return result;
  if ((result = ulang_parser_eat(parser, '(', &temp_tok)).kind != ULANG_SUCCESS) return result;

  funcdef->name = name_tok.value.as.string;

  ulang_token_t tok = {0};
  size_t type = 0;
  for (bool b = false; (ulang_parser_peek(parser, 0, &tok)||(result=(ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF) }, 0)) && tok.kind != ')'; b=true) {
    if (b && (result = ulang_parser_eat(parser, ',', NULL)).kind != ULANG_SUCCESS) return result;
    if (b && !ulang_parser_peek(parser, 0, &tok)) return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF) };
    if (tok.kind != ULANG_TOKEN_ID) return (ulang_result_t){
      .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
      .location = tok.location
    };
    ulang_token_t next_tok = {0};
    if (!ulang_parser_peek(parser, 0, &next_tok)) return (ulang_result_t){
      .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
    };
    if (next_tok.kind == ULANG_TOKEN_ID) {
      if (!ulang_ast_program_find_type(program, tok.value.as.string, &type)) return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_TYPE_NOT_FOUND), .location = tok.location};
      if (!ulang_parser_advance(parser, &tok) ||
          !ulang_parser_advance(parser, NULL)) return (ulang_result_t){
        .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
      };
    } else if (!b || next_tok.kind != ',') return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN), .location = next_tok.location};
    ulang_func_param_t param = (ulang_func_param_t){
      .name = tok.value.as.string,
      .type = type
    };
    nob_da_append(&funcdef->params, param);
  }
  if (result.kind != ULANG_SUCCESS || (!ulang_parser_advance(parser, NULL)&&(result=(ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF) },1))) return result;

  if ((result = ulang_parser_parse_scope(parser, &funcdef->body, program)).kind != ULANG_SUCCESS) return result;

  return (ulang_result_t){
    .kind = ULANG_SUCCESS,
    .location = (ulang_location_t){0}
  };
}

ulang_result_t ulang_parser_parse_stmt(ulang_parser_t *parser, ulang_ast_stmt_t *stmt, ulang_ast_program_t *program) {
  ulang_token_t token = *parser->ptr;
  ulang_location_t location = token.location;

  switch (token.kind) {
  case ULANG_TOKEN_ID: {
    if (nob_sv_eq(token.value.as.string, SV("if"))) { // if

    } else if (ulang_ast_program_find_type(program, token.value.as.string, NULL)) {
      ulang_token_t tok = {0};
      if (!ulang_parser_peek(parser, 2, &tok)) return (ulang_result_t){ .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF), .location = (ulang_location_t){0} };
      if (tok.kind == ';' || tok.kind == '=') { // var def
        ulang_ast_stmt_vardef_t vardef = {0};
        ulang_result_t result = {0};
        if ((result = ulang_parser_parse_vardef(parser, &vardef, program)).kind != ULANG_SUCCESS) return result;
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

ulang_result_t ulang_parser_parse_expr(ulang_parser_t *parser, ulang_ast_expr_t *expr, ulang_ast_program_t *program) {
  memset(expr, sizeof(*expr), 0);
  ulang_token_t token = {0};
  if (!ulang_parser_peek(parser, 0, &token)) return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)};

  switch (token.kind) {
  case ULANG_TOKEN_ID: {
    *expr = (ulang_ast_expr_t){
      .kind = ULANG_AST_EXPR_VAR,
      .as.var = token.value.as.string
    };
  } break;
  default: {
    goto literal;
  } break;
  }

  return (ulang_result_t){.kind = (ulang_parser_advance(parser, NULL)?ULANG_SUCCESS:ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF))};
literal:
  expr->kind = ULANG_AST_EXPR_LIT;
  return ulang_parser_parse_literal(parser, &expr->as.literal, program);
}

ulang_result_t ulang_parser_parse_scope(ulang_parser_t *parser, ulang_ast_stmt_scope_t *scope, ulang_ast_program_t *program) {
  if (parser->ptr->kind == '{' && (++parser->ptr));
  *scope = (ulang_ast_stmt_scope_t){0};
  while (parser->ptr-parser->tokens.items<parser->tokens.count && parser->ptr->kind != '}') {
    ulang_ast_stmt_t stmt = {0};
    ulang_result_t result = {0};
    if ((result = ulang_parser_parse_stmt(parser, &stmt, program)).kind != ULANG_SUCCESS) return result;
    nob_da_append(scope, stmt);
  }
  return ulang_parser_eat(parser, '}', NULL);
}

ulang_result_t ulang_parser_parse_vardef(ulang_parser_t *parser, ulang_ast_stmt_vardef_t *vardef, ulang_ast_program_t *program) {
  ulang_token_t type_tok = {0};
  ulang_token_t name_tok = {0};
  size_t type = {0};

  ulang_result_t result = {0};
  if ((result = ulang_parser_eat(parser, ULANG_TOKEN_ID, &type_tok)).kind != ULANG_SUCCESS) return result;
  if (!ulang_ast_program_find_type(program, type_tok.value.as.string, &type)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_TYPE_NOT_FOUND),
    .location = type_tok.location
  };
  if ((result = ulang_parser_eat(parser, ULANG_TOKEN_ID, &name_tok)).kind != ULANG_SUCCESS) return result;

  if (ulang_parser_is_variable_taken(parser, name_tok.value.as.string, NULL)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_VARIABLE_REDEFINITION),
    .location = type_tok.location
  };

  ulang_token_t tok = {0};
  if (!ulang_parser_peek(parser, 0, &tok) || !ulang_parser_advance(parser, NULL)) return (ulang_result_t){
    .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)
  };

  *vardef = (ulang_ast_stmt_vardef_t){
    .name = name_tok.value.as.string,
    .type = type
  };

  if (tok.kind == '=') {
    vardef->init = malloc(sizeof(*vardef->init));
    if ((result = ulang_parser_parse_expr(parser, vardef->init, program)).kind != ULANG_SUCCESS) return result;
  } else if (tok.kind != ';') {
    return (ulang_result_t){
      .kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN),
      .location = tok.location
    };
  }

  if ((result = ulang_parser_eat(parser, ';', NULL)).kind != ULANG_SUCCESS) return result;

  ulang_variable_t variable = (ulang_variable_t){
    .name = vardef->name,
    .type = vardef->type
  };
  nob_da_append(&parser->variables, variable);

  return result;
}

ulang_result_t ulang_parser_parse_literal(ulang_parser_t *parser, ulang_ast_lit_handle_t *literal, ulang_ast_program_t *program) {
  ulang_token_t token = {0};
  if (!ulang_parser_peek(parser, 0, &token)) return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_EOF)};

  switch (token.kind) {
  case ULANG_TOKEN_INT: {
    ulang_ast_program_find_or_append_literal(program, token.value, literal);
    ulang_parser_advance(parser, NULL);
    return (ulang_result_t){0};
  } break;
  }
  return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_PARSER_ERROR, ULANG_UNEXPECTED_TOKEN), .location = token.location};
}

void ulang_parser_free(ulang_parser_t parser) {
  nob_da_free(parser.tokens);
}