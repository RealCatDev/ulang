#ifndef   ULANG_ULANG_H_
#define   ULANG_ULANG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// include nob (https://github.com/tsoding/ht/blob/master/nob.h)
#include "nob.h"

#define SV(str) (Nob_String_View){ .data = str, .count = sizeof(str)-1 }

#include "ulang/type.h"
#include "ulang/variable.h"
#include "ulang/value.h"
#include "ulang/errors.h"
#include "ulang/frontend/lexer.h"
#include "ulang/frontend/ast.h"
#include "ulang/frontend/parser.h"

#endif // ULANG_ULANG_H_