#ifndef INTERPRETER_BRIDGE_H
#define INTERPRETER_BRIDGE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void interpreter_init(void *stack_top);
void interpreter_deinit(void);
int interpreter_exec(const char *source);
int interpreter_eval_int(const char *source, int64_t *out);
const char *interpreter_last_error(void);
void interpreter_collect(void);
size_t interpreter_heap_free(void);
void platform_log_write(const char *text, size_t length);
#ifdef __cplusplus
}
#endif
#endif
