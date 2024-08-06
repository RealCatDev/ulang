#include "ulang/ulang.h"

// BYTECODE CODEGEN
const uint16_t ULANG_BC_MAGIC = 0x6411; // ULang because magic.
const uint16_t ULANG_BC_ENTRY = 0x3771;
const uint16_t ULANG_BC_LITS  = 0x1115;
const uint16_t ULANG_BC_TOPS  = 0x1095;

const uint8_t ULANG_BC_STMT = 0x51;
const uint8_t ULANG_BC_EXPR = 0x37;
const uint8_t ULANG_BC_LIT  = 0x11;

const size_t ULANG_VALUE_KIND_TO_SIZE[ULANG_VALUES_COUNT] = {
  [ULANG_VALUE_INT]    = sizeof(int64_t),
  [ULANG_VALUE_UINT]   = sizeof(uint64_t),
  [ULANG_VALUE_FLOAT]  = sizeof(float),
  [ULANG_VALUE_DOUBLE] = sizeof(double),
  [ULANG_VALUE_STRING] = 0
};


#define sb_append_sv(sb, sv)                          \
  nob_sb_append_buf(sb, &sv.count, sizeof(sv.count)); \
  nob_sb_append_buf(sb, sv.data, sv.count)

bool sb_append_value(Nob_String_Builder *sb, ulang_value_t value) {
  nob_sb_append_buf(sb, &value.kind, sizeof(value.kind));
  switch (value.kind) {
  case ULANG_VALUE_FLOAT:
  case ULANG_VALUE_DOUBLE:
  case ULANG_VALUE_UINT:
  case ULANG_VALUE_INT: {
    nob_sb_append_buf(sb, &value.as, ULANG_VALUE_KIND_TO_SIZE[value.kind]);
  } break;
  case ULANG_VALUE_STRING: {
    sb_append_sv(sb, value.as.string);
  } break;
  default: return false;
  }
  return true;
}

ulang_result_t ulang_codegen_bc_gen_expr(ulang_codegen_t *codegen, ulang_ast_expr_t expr) {
  nob_sb_append_buf(&codegen->sb, &expr.kind, sizeof(expr.kind));

  switch (expr.kind) {
  case ULANG_AST_EXPR_LIT: {
    nob_sb_append_buf(&codegen->sb, &expr.as.literal, sizeof(expr.as.literal));
  } break;
  }

  return (ulang_result_t){0};
}

ulang_result_t ulang_codegen_bc_gen_toplevel(ulang_codegen_t *codegen, ulang_ast_toplevel_t toplevel) {
  nob_sb_append_buf(&codegen->sb, &toplevel.kind, sizeof(toplevel.kind));

  switch (toplevel.kind) {
  case ULANG_AST_TOPLEVEL_FUNCDEF: {
    ulang_ast_stmt_func_t funcdef = toplevel.as.funcdef;
    size_t type = funcdef.type | ((funcdef.has_type?1:0)<<7);
    nob_sb_append_buf(&codegen->sb, &type, sizeof(type));
    sb_append_sv(&codegen->sb, funcdef.name);
    nob_sb_append_buf(&codegen->sb, &funcdef.params.count, sizeof(funcdef.params.count));
    for (size_t i = 0; i < funcdef.params.count; ++i) {
      ulang_func_param_t param = funcdef.params.items[i];
      sb_append_sv(&codegen->sb, param.name);
    }
  } break;
  case ULANG_AST_TOPLEVEL_VARDEF: {
    ulang_ast_stmt_vardef_t vardef = toplevel.as.vardef;
    nob_sb_append_buf(&codegen->sb, &vardef.type, sizeof(vardef.type));
    sb_append_sv(&codegen->sb, vardef.name);
    nob_sb_append_buf(&codegen->sb, &ULANG_BC_EXPR, sizeof(ULANG_BC_EXPR));
    ulang_result_t result = {0};
    if (vardef.init && (result = ulang_codegen_bc_gen_expr(codegen, *vardef.init)).kind != ULANG_SUCCESS) return result;
  } break;
  default: return (ulang_result_t){.kind = ULANG_BADARG_ERROR};
  }

  return (ulang_result_t){0};
}

ulang_result_t ulang_codegen_bc_gen(ulang_codegen_t *codegen) {
  if (!codegen) return (ulang_result_t){.kind = ULANG_BADARG_ERROR};
  ulang_result_t result = {0};

  Nob_String_View entry_name = SV("main");

  nob_sb_append_buf(&codegen->sb, &ULANG_BC_MAGIC, sizeof(uint16_t));
  nob_sb_append_buf(&codegen->sb, &ULANG_BC_ENTRY, sizeof(uint16_t));
  sb_append_sv(&codegen->sb, entry_name);

  ulang_literals_t literals = codegen->program.literals;
  nob_sb_append_buf(&codegen->sb, &ULANG_BC_LITS, sizeof(uint16_t));
  nob_sb_append_buf(&codegen->sb, &literals.count, sizeof(literals.count));
  for (size_t i = 0; i < literals.count; ++i) {
    if (!sb_append_value(&codegen->sb, literals.items[i])) return (ulang_result_t){.kind = ULANG_BADARG_ERROR};
  }

  ulang_ast_toplevels_t tops = codegen->program.body;
  nob_sb_append_buf(&codegen->sb, &ULANG_BC_TOPS, sizeof(uint16_t));
  nob_sb_append_buf(&codegen->sb, &tops.count, sizeof(tops.count));
  for (size_t i = 0; i < tops.count; ++i) {
    if ((result = ulang_codegen_bc_gen_toplevel(codegen, tops.items[i])).kind != ULANG_SUCCESS) return result;
  }

  return result;
}

ulang_result_t ulang_codegen_init(ulang_codegen_t *codegen, ulang_codegen_kind_t kind, ulang_parser_t *parser) {
  if (!codegen || !parser) return (ulang_result_t){.kind = ULANG_BADARG_ERROR};
  memset(codegen, sizeof(*codegen), 0);
  codegen->kind = kind;

  ulang_result_t result = ulang_parser_parse(parser, &codegen->program);
  if (result.kind != ULANG_SUCCESS) return result;
  return (ulang_result_t){0};
}

ulang_result_t ulang_codegen_gen(ulang_codegen_t *codegen) {
  switch (codegen->kind) {
  case ULANG_CODEGEN_BYTECODE: return ulang_codegen_bc_gen(codegen);
  default: return (ulang_result_t){.kind = ULANG_PACK_RESULT_KIND(ULANG_CODEGEN_ERROR, ULANG_BADARG_ERROR)};
  }
}

ulang_result_t ulang_codegen_save(ulang_codegen_t *codegen, const char *file_name) {
  if (!codegen || !file_name) return (ulang_result_t){.kind = ULANG_BADARG_ERROR};
  nob_write_entire_file(file_name, codegen->sb.items, codegen->sb.count);
  return (ulang_result_t){0};
}

void ulang_codegen_free(ulang_codegen_t codegen) {
  nob_sb_free(codegen.sb);
  nob_da_free(codegen.program.body);
  nob_da_free(codegen.program.literals);
  nob_da_free(codegen.program.types);
}