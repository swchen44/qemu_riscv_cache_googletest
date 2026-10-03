#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "product.h"
#include "replay.h"
#include <stdio.h>
#include <unistd.h>
static QueueHandle_t queue;
static SemaphoreHandle_t producer_done;
static TaskHandle_t supervisor;
static volatile unsigned produced,processed,preemptions,timeouts;
static volatile int failed;
#ifdef RTOS_DIAGNOSTICS
/* Trace collection performs no I/O while tasks exchange data. */
struct trace_event { unsigned task,event,item,value; TickType_t tick; };
static struct trace_event trace_events[96];
static unsigned trace_count,trace_dropped;
static void trace_record(unsigned task,unsigned event,unsigned item,unsigned value){
 taskENTER_CRITICAL();
 if(trace_count<96){
  struct trace_event *t=&trace_events[trace_count++];
  t->task=task;t->event=event;t->item=item;t->value=value;t->tick=xTaskGetTickCount();
 }else ++trace_dropped;
 taskEXIT_CRITICAL();
}
static void trace_dump(void){
 printf("TRACE events=%u dropped=%u tasks=1:producer,2:worker,3:supervisor events=1:send_before,2:send_after,3:receive,4:notify_before,5:notify_after,6:wait_before,7:wait_after,8:producer_done,9:supervisor_done\n",trace_count,trace_dropped);
 for(unsigned i=0;i<trace_count;i++){
  const struct trace_event *t=&trace_events[i];
  printf("TRACE seq=%u task=%u event=%u item=%u value=%u tick=%lu\n",i,t->task,t->event,t->item,t->value,(unsigned long)t->tick);
 }
}
#define TRACE(t,e,i,v) trace_record(t,e,i,v)
#else
#define TRACE(t,e,i,v) ((void)0)
#define trace_dump() ((void)0)
#endif
#define CHECK(x) do { if(!(x)) { printf("FAIL: %s line %d\n",#x,__LINE__); failed=1; } } while(0)
void demo_assert(const char *file,int line){printf("ASSERT %s:%d\n",file,line);_exit(2);}
void vApplicationMallocFailedHook(void){demo_assert("malloc",0);}
void vApplicationStackOverflowHook(TaskHandle_t t,char*n){(void)t;demo_assert(n,0);}
static void producer(void *arg){
 (void)arg; vTaskDelay(5);
 for(unsigned i=0;i<8;i++){
  TRACE(1,1,i,processed);
  CHECK(xQueueSend(queue,&i,portMAX_DELAY)==pdPASS);
  TRACE(1,2,i,processed);
  if(processed==i+1)++preemptions;
  ++produced; vTaskDelay(1);
 }
 /* Completion is synchronized explicitly; elapsed ticks do not prove that
    the lower-priority producer has resumed after its last queue send. */
 TRACE(1,8,8,produced);
 CHECK(xSemaphoreGive(producer_done)==pdTRUE);
 vTaskSuspend(NULL);
}
static void worker(void *arg){
 (void)arg; unsigned id; TickType_t before=xTaskGetTickCount();
 CHECK(xQueueReceive(queue,&id,3)==errQUEUE_EMPTY);
 CHECK(xTaskGetTickCount()-before>=3);++timeouts;
 for(unsigned i=0;i<8;i++){
  CHECK(xQueueReceive(queue,&id,portMAX_DELAY)==pdPASS);CHECK(id==i);TRACE(2,3,i,id);
  struct product_result r;CHECK(product_process(replay_samples,REPLAY_COUNT,REPLAY_OFFSET,&r)==0);
#ifdef INJECT_FAILURE
  CHECK(r.sum==REPLAY_SUM+1);
#else
  CHECK(r.sum==REPLAY_SUM);
#endif
  CHECK(r.checksum==REPLAY_CHECKSUM);++processed;
  TRACE(2,4,i,processed);xTaskNotifyGive(supervisor);TRACE(2,5,i,processed);
 }
 vTaskSuspend(NULL);
}
int rv32_check_os_behavior(void){
 for(unsigned i=0;i<8;i++){
  TRACE(3,6,i,0);
  uint32_t notified=ulTaskNotifyTake(pdFALSE,100);
  TRACE(3,7,i,notified);
  if(notified!=1)printf("NOTIFY_DIAGNOSTIC iteration=%u returned=%lu tick=%lu produced=%u processed=%u\n",i,(unsigned long)notified,(unsigned long)xTaskGetTickCount(),produced,processed);
  CHECK(notified==1);
 }
 CHECK(xSemaphoreTake(producer_done,portMAX_DELAY)==pdTRUE);
 TRACE(3,9,8,produced);
 CHECK(produced==8);CHECK(processed==8);CHECK(preemptions==8);CHECK(timeouts==1);
 printf("RTOS evidence: application_tasks=3 produced=%u processed=%u immediate_priority_preemptions=%u queue_timeouts=%u tick=%lu\n",produced,processed,preemptions,timeouts,(unsigned long)xTaskGetTickCount());
 trace_dump();
 printf("%s: replay, queue block/wakeup, timeout, delay, priority preemption, notifications\n",failed?"FAIL":"PASS");
 return failed?1:0;
}
static void monitor(void *arg){
 (void)arg;
#ifdef RUN_GTEST_IN_RTOS
 extern int rv32_run_gtests(void);
 _exit(rv32_run_gtests());
#else
 _exit(rv32_check_os_behavior());
#endif
}
int demo_main(void){
 setvbuf(stdout,NULL,_IONBF,0);puts("RV32 FreeRTOS actual kernel: demo profile rv32imac_zicsr / ilp32 / QEMU virt, NOT product timing model");
 queue=xQueueCreate(1,sizeof(unsigned));configASSERT(queue);
 producer_done=xSemaphoreCreateBinary();configASSERT(producer_done);
 configASSERT(xTaskCreate(monitor,"supervisor",2048,NULL,3,&supervisor)==pdPASS);
 configASSERT(xTaskCreate(worker,"worker",2048,NULL,2,NULL)==pdPASS);
 configASSERT(xTaskCreate(producer,"producer",2048,NULL,1,NULL)==pdPASS);
 vTaskStartScheduler();return 3;
}
