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
#define CHECK(x) do { if(!(x)) { printf("FAIL: %s line %d\n",#x,__LINE__); failed=1; } } while(0)
void demo_assert(const char *file,int line){printf("ASSERT %s:%d\n",file,line);_exit(2);}
void vApplicationMallocFailedHook(void){demo_assert("malloc",0);}
void vApplicationStackOverflowHook(TaskHandle_t t,char*n){(void)t;demo_assert(n,0);}
static void producer(void *arg){
 (void)arg; vTaskDelay(5);
 for(unsigned i=0;i<8;i++){
  CHECK(xQueueSend(queue,&i,portMAX_DELAY)==pdPASS);
  if(processed==i+1)++preemptions;
  ++produced; vTaskDelay(1);
 }
 /* Completion is synchronized explicitly; elapsed ticks do not prove that
    the lower-priority producer has resumed after its last queue send. */
 CHECK(xSemaphoreGive(producer_done)==pdTRUE);
 vTaskSuspend(NULL);
}
static void worker(void *arg){
 (void)arg; unsigned id; TickType_t before=xTaskGetTickCount();
 CHECK(xQueueReceive(queue,&id,3)==errQUEUE_EMPTY);
 CHECK(xTaskGetTickCount()-before>=3);++timeouts;
 for(unsigned i=0;i<8;i++){
  CHECK(xQueueReceive(queue,&id,portMAX_DELAY)==pdPASS);CHECK(id==i);
  struct product_result r;CHECK(product_process(replay_samples,REPLAY_COUNT,REPLAY_OFFSET,&r)==0);
#ifdef INJECT_FAILURE
  CHECK(r.sum==REPLAY_SUM+1);
#else
  CHECK(r.sum==REPLAY_SUM);
#endif
  CHECK(r.checksum==REPLAY_CHECKSUM);++processed;xTaskNotifyGive(supervisor);
 }
 vTaskSuspend(NULL);
}
int rv32_check_os_behavior(void){
 for(unsigned i=0;i<8;i++){CHECK(ulTaskNotifyTake(pdFALSE,100)==1);}
 CHECK(xSemaphoreTake(producer_done,portMAX_DELAY)==pdTRUE);
 CHECK(produced==8);CHECK(processed==8);CHECK(preemptions==8);CHECK(timeouts==1);
 printf("RTOS evidence: application_tasks=3 produced=%u processed=%u immediate_priority_preemptions=%u queue_timeouts=%u tick=%lu\n",produced,processed,preemptions,timeouts,(unsigned long)xTaskGetTickCount());
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
