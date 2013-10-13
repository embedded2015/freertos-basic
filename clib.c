#include "fio.h"
#include <stdarg.h>
#include "clib.h"

void send_byte(char );

size_t fio_printf(int fd, const char *format, ...){
	int i,count=0;

	va_list(v1);
	va_start(v1, format);

	int tmpint;
	char *tmpcharp;
	
	for(i=0; format[i]; ++i){
		if(format[i]=='%'){
			switch(format[i+1]){
				case '%':
					send_byte('%'); break;
				case 'd':
				case 'X':
					tmpint = va_arg(v1, int);
					tmpcharp = itoa(tmpint, format[i+1]=='d'?10: 16);
					fio_write(fd, tmpcharp, 3);
					break;
				case 's':
					tmpcharp = va_arg(v1, char *);
					fio_write(fd, tmpcharp, strlen(tmpcharp));
					break;
			}
			/* Skip the next character */
			++i;
		}else
			fio_write(fd, format+i, 1);
	}

	va_end(v1);
	return count;
}

size_t strlen(const char *str){
	size_t count;
	for(count=0;*str;++count, ++str);
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

char *itoa(int num, unsigned int base){
	static char buf[32]={0};
	int i;
	if(num==0){
		buf[30]='0';
		return &buf[30];
	}
	int negative=(num<0);
	if(negative) num=-num;
	for(i=30; i>=0&&num; --i, num/=base)
		buf[i] = "0123456789ABCDEF" [num % base];
	if(negative){
		buf[i]='-';
		--i;
	}
	return buf+i+1;
}
