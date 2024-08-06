#ifndef   ULANG_VALUE_H_
#define   ULANG_VALUE_H_

typedef enum {
  ULANG_VALUE_INT = 0,
  ULANG_VALUE_UINT,
  ULANG_VALUE_DOUBLE,
  ULANG_VALUE_FLOAT,
  ULANG_VALUE_STRING,
  ULANG_VALUES_COUNT
} ulang_value_kind_t;

typedef struct {
  ulang_value_kind_t kind;
  union {
    int64_t integer;
    uint64_t unsigned_int;
    double double_precission;
    float floating_point;
    Nob_String_View string;
  } as;
} ulang_value_t;

bool ulang_value_eq(ulang_value_t a, ulang_value_t b);

#endif // ULANG_VALUE_H_