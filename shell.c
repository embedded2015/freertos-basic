#include "shell.h"
#include <stddef.h>
#include "clib.h"

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

int parse_command(char *str, char *argv[]){
        int b_quote=0, b_dbquote=0;
        int i;
        int count=0, p=0;
        for(i=0; str[i]; ++i){
                if(str[i]=='\'')
                        ++b_quote;
                if(str[i]=='"')
                        ++b_dbquote;
                if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
                        str[i]='\0';
                        argv[count++]=&str[p];
                        p=i+1;
                }
        }
        /* last one */
        argv[count++]=&str[p];

        return count;
}

void ls_command(int n, char *argv[]){

}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}
}

cmdfunc *do_command(const char *cmd){
	static cmdlist cl[]={
		{.name="ls", .fptr=ls_command, .desc="List directory"}
		,{.name="man", .fptr=man_command, .desc="Show the manual of the command"}
	};

	int i;
	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
