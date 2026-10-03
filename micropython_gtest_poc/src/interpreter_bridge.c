#include "interpreter_bridge.h"
#include <string.h>
#include "py/compile.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "port/micropython_embed.h"
static uintptr_t gc_heap[512 * 1024 / sizeof(uintptr_t)];
static char last_error[64];
void interpreter_init(void *stack_top) {last_error[0]=0;mp_embed_init(gc_heap,sizeof gc_heap,stack_top);}
void interpreter_deinit(void) {mp_embed_deinit();}
const char *interpreter_last_error(void) {return last_error;}
static int failed(void *value) {
 const char *name=qstr_str(mp_obj_get_type((mp_obj_t)value)->name);
 strncpy(last_error,name,sizeof last_error-1);last_error[sizeof last_error-1]=0;return -1;
}
/* NLR/setjmp stays wholly inside C; never jumps across GoogleTest C++ frames. */
int interpreter_exec(const char *source) {
 nlr_buf_t nlr;last_error[0]=0;
 if(nlr_push(&nlr)==0) {
  mp_lexer_t *lex=mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_,source,strlen(source),0);
  qstr name=lex->source_name;mp_parse_tree_t tree=mp_parse(lex,MP_PARSE_FILE_INPUT);
  mp_obj_t fun=mp_compile(&tree,name,true);mp_call_function_0(fun);nlr_pop();return 0;
 }
 return failed(nlr.ret_val);
}
int interpreter_eval_int(const char *source,int64_t *out) {
 nlr_buf_t nlr;last_error[0]=0;
 if(nlr_push(&nlr)==0) {
  mp_lexer_t *lex=mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_,source,strlen(source),0);
  qstr name=lex->source_name;mp_parse_tree_t tree=mp_parse(lex,MP_PARSE_EVAL_INPUT);
  mp_obj_t fun=mp_compile(&tree,name,true);mp_obj_t result=mp_call_function_0(fun);
  *out=(int64_t)mp_obj_get_int(result);nlr_pop();return 0;
 }
 return failed(nlr.ret_val);
}
void interpreter_collect(void) {gc_collect();}
size_t interpreter_heap_free(void) {gc_info_t info;gc_info(&info);return info.free;}
/* Platform stdout seam only. The parser/VM/objects/GC are real MicroPython. */
void mp_hal_stdout_tx_strn_cooked(const char *str,size_t len) {platform_log_write(str,len);}
