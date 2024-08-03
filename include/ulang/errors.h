#ifndef   ULANG_ERRORS_H_
#define   ULANG_ERRORS_H_

typedef struct {
  size_t row;
  size_t col;
} ulang_location_t;

#define ULANG_LOC_Fmt "%zu:%zu:"
#define ULANG_LOC_Arg(loc) (loc).row, (loc).col

enum {
  ULANG_SUCCESS = 0,
  ULANG_BADARG_ERROR,
  ULANG_FS_ERROR,
  ULANG_LEXER_ERROR,
  ULANG_PARSER_ERROR,
};

typedef struct {
  uint64_t kind;
  ulang_location_t location;
} ulang_result_t;

#define ULANG_PACK_RESULT_KIND(kind, data) (uint64_t)((kind) | ((uint64_t)data<<32))
#define ULANG_UNPACK_RESULT_KIND(kind) (kind&0x7FFFFFFF)
#define ULANG_UNPACK_RESULT_DATA(kind) (kind>>32)

#endif // ULANG_ERRORS_H_