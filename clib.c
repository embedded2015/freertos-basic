#include "fio.h"
#include <stdarg.h>
#include "clib.h"

size_t fio_printf(int fd, const char *format, ...){
	return 0;
}

size_t strlen(const char *str){
	size_t count;
	for(count=0;*str;++count);
	return count;
}

char *strcat(char * restrict dest, const char * restrict source){
	/* locate '\0' in dest */
	for(;*dest;++dest);
	/* copy character from source */
	for(;*source; ++dest, ++source)
		*dest=*source;
	*dest='\0';
	return dest;
}
