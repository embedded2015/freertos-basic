#include "host.h"
#include <stdarg.h>

typedef union param_t{
	int pdInt;
	void *pdPtr;
	char *pdChrPtr;
} param;

/*action will be in r0, and argv in r1*/
int host_call(enum HOST_SYSCALL action, void *argv)
{
    /* For Thumb-2 code use the BKPT instruction instead of SWI.
* Refer to:
* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471c/Bgbjhiea.html
* http://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M4 */
    int result;
    __asm__( \
      "bkpt 0xAB\n"\
      "nop\n" \
      "bx lr\n"\
        :"=r" (result) ::\
    );
    return result;
}

int host_system(char *cmd){
	return host_call(SYS_SYSTEM, (param []){{.pdChrPtr=cmd}, {.pdInt=strlen(cmd)}});
}

int host_open(char *path, int mode) {
    return host_call(SYS_OPEN, (param []){{.pdChrPtr=path}, {.pdInt=mode}, {.pdInt=strlen(path)}});
}

int host_close(int handle) {
    return host_call(SYS_CLOSE, (param []){{.pdInt=handle}});
}

int host_write(int handle, void *data, int size) {
    return host_call(SYS_WRITE, (param []){{.pdInt=handle}, {.pdPtr=data}, {.pdInt=size}});
}

int host_action(enum HOST_SYSCALL action, ...)
{
    char *tmpChrPtr;

    va_list v1;
    va_start(v1, action);

    switch(action) {
        case SYS_OPEN:
            tmpChrPtr = va_arg(v1, char *);
            return host_call(SYS_OPEN, (param []){{.pdChrPtr=tmpChrPtr}, {.pdInt=va_arg(v1, int)}, {.pdInt=strlen(tmpChrPtr)}});
        case SYS_CLOSE:
            return host_call(SYS_CLOSE, (param []){{.pdInt=va_arg(v1, int)}});
        case SYS_WRITE:
            return host_call(SYS_WRITE, (param []){{.pdInt=va_arg(v1, int)}, {.pdPtr=va_arg(v1, void *)}, {.pdInt=va_arg(v1, int)}});
        case SYS_SYSTEM:
            tmpChrPtr = va_arg(v1, char *);
            return host_call(SYS_SYSTEM, (param []){{.pdChrPtr=tmpChrPtr}, {.pdInt=strlen(tmpChrPtr)}});
        default:
            break;
    }

    va_end(v1);

    return -1;
}
