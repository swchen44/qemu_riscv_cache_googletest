#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
void uart_puts(const char *s) { while (*s) { volatile uint8_t *u=(void *)0x10000000; while (!(u[5]&0x20)) {} u[0]=(uint8_t)*s++; } }
int _write(int fd, const void *buf, size_t n) { (void)fd; const char *p=buf; for(size_t i=0;i<n;i++){ char s[2]={p[i],0};uart_puts(s); } return (int)n; }
void _exit(int status) { *(volatile uint32_t *)0x100000 = status ? ((uint32_t)status << 16) | 0x3333 : 0x5555; for(;;) __asm__ volatile("wfi"); }
void *_sbrk(ptrdiff_t n) { extern char __heap_start[],__heap_end[]; static char *p; if(!p)p=__heap_start; if(n<0 || p+n>__heap_end){errno=ENOMEM;return(void *)-1;} char *old=p;p+=n;return old; }
int _close(int fd){(void)fd;errno=EBADF;return -1;}
int _fstat(int fd,struct stat*s){(void)fd;s->st_mode=S_IFCHR;return 0;}
int _isatty(int fd){return fd>=0&&fd<=2;}
off_t _lseek(int fd,off_t p,int w){(void)fd;(void)p;(void)w;errno=ESPIPE;return -1;}
int _read(int fd,void*b,size_t n){(void)fd;(void)b;(void)n;return 0;}
int _kill(int pid,int sig){(void)pid;(void)sig;errno=ENOSYS;return -1;}
int _getpid(void){return 1;}
int _gettimeofday(struct timeval*t,void*z){(void)z;volatile uint32_t *m=(void*)0x200bff8; uint32_t hi,lo;do{hi=m[1];lo=m[0];}while(hi!=m[1]);uint64_t us=(((uint64_t)hi<<32)|lo)/10;t->tv_sec=us/1000000;t->tv_usec=us%1000000;return 0;}
void _init(void){}
void _fini(void){}
void *__dso_handle = &__dso_handle;
