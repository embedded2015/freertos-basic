#include "host.h"

typedef union param_t{
	int pdInt;
	const void *pdPtr;
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

int host_system(const char *cmd){
	param p[]={
		{.pdChrPtr=cmd},
		{.pdInt=strlen(cmd)}
	};
	return host_call(SYS_SYSTEM, p);
}
