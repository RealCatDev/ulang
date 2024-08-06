#ifndef   ULANG_BACKEND_VM_H_
#define   ULANG_BACKEND_VM_H_

typedef struct {
  
} ulang_function_t;

typedef struct {
  ulang_function_t *items;
  size_t count;
  size_t capacity;
} ulang_functions_t;

typedef struct {
  ulang_literals_t literals;
  ulang_functions_t functions;
} ulang_vm_t;

#endif // ULANG_BACKEND_VM_H_