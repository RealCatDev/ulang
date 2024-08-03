#ifndef   ULANG_VALUE_H_
#define   ULANG_VALUE_H_

typedef enum {
  ULANG_VALUE_STRING,
  ULANG_VALUE_INT,
  ULANG_VALUE_UINT,
  ULANG_VALUE_DOUBLE,
  ULANG_VALUE_FLOAT,
} ulang_value_kind_t;

typedef struct {
  ulang_value_kind_t kind;
  union {
    Nob_String_View string;
    int64_t integer;
    uint64_t unsigned_int;
    double double_precission;
    float floating_point;
  } as;
} ulang_value_t;

#endif // ULANG_VALUE_H_