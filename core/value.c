#include "ulang/ulang.h"

bool ulang_value_eq(ulang_value_t a, ulang_value_t b) {
  if (a.kind != b.kind) return false;

  if (a.kind == ULANG_VALUE_STRING) return nob_sv_eq(a.as.string, b.as.string);
  else return memcmp(&a.as, &b.as, sizeof(a.as)) == 0;
}