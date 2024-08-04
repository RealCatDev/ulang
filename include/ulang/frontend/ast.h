#ifndef   ULANG_FRONTEND_AST_
#define   ULANG_FRONTEND_AST_

typedef ulang_value_t ulang_ast_lit_t;
typedef struct ulang_ast_expr ulang_ast_expr_t;
typedef struct ulang_ast_stmt ulang_ast_stmt_t;
typedef struct ulang_ast_toplevel ulang_ast_toplevel_t;
typedef struct ulang_ast_program ulang_ast_program_t;

// Expressions

typedef enum {
  ULANG_AST_EXPR_LIT = 0,
  ULANG_AST_EXPR_PAREN,
  ULANG_AST_EXPR_BINOP,
  ULANG_AST_EXPR_VAR,
  ULANG_AST_EXPR_KIND_COUNT
} ulang_ast_expr_kind_t;

typedef struct {
  ulang_ast_expr_t *lhs;
  ulang_ast_expr_t *rhs;
  char op;
} ulang_ast_expr_binop_t;

typedef Nob_String_View ulang_ast_expr_var_t;

struct ulang_ast_expr {
  ulang_ast_expr_kind_t kind;
  union {
    ulang_ast_lit_t literal;
    ulang_ast_expr_t *paren;
    ulang_ast_expr_binop_t bin_op;
    ulang_ast_expr_var_t var;
  } as;
};

// Statements

typedef enum {
  ULANG_AST_STMT_EXPR = 0,
  ULANG_AST_STMT_SCOPE,
  ULANG_AST_STMT_VARDEF,
  ULANG_AST_STMT_FUNC,
  ULANG_AST_STMT_KIND_COUNT
} ulang_ast_stmt_kind_t;

typedef struct {
  ulang_ast_stmt_t *items;
  size_t count;
  size_t capacity;
} ulang_ast_stmt_scope_t;

typedef struct {
  Nob_String_View name;
  ulang_type_t type;
  ulang_ast_expr_t *init;
} ulang_ast_stmt_vardef_t;

struct ulang_ast_stmt {
  ulang_ast_stmt_kind_t kind;
  union {
    ulang_ast_expr_t expr;
    ulang_ast_stmt_scope_t scope;
    ulang_ast_stmt_vardef_t vardef;
  } as;
};

// Top level

typedef enum {
  ULANG_AST_TOPLEVEL_FUNCDEF = 0,
  ULANG_AST_TOPLEVEL_VARDEF,
} ulang_ast_toplevel_kind_t;

typedef struct {
  Nob_String_View type;
  Nob_String_View name;
} ulang_func_param_t;

typedef struct {
  ulang_func_param_t *items;
  size_t count;
  size_t capacity;
} ulang_func_params_t;

typedef struct {
  Nob_String_View name;
  ulang_func_params_t params;
  ulang_ast_stmt_scope_t body;
} ulang_ast_stmt_func_t;

struct ulang_ast_toplevel {
  ulang_ast_toplevel_kind_t kind;
  union {
    ulang_ast_stmt_func_t funcdef;
    ulang_ast_stmt_vardef_t vardef;
  } as;
};

typedef struct {
  ulang_ast_toplevel_t *items;
  size_t count;
  size_t capacity;
} ulang_ast_toplevels_t;

struct ulang_ast_program {
  ulang_ast_toplevels_t body;
};

#endif // ULANG_FRONTEND_AST_