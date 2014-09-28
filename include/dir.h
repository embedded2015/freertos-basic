#ifndef __DIR_H_
#define __DIR_H__

#include <stdio.h>

#define MAX_DIRS 32

#define ENOTOPEN -1
#define ENOTSUPPORT -2

typedef int (*dirnext_t)(void * opaque, void * buf, size_t bufsize);
typedef int (*dirclose_t)(void * opaque);

typedef struct dirdef_t {
    dirnext_t dirnext;
    dirclose_t dirclose;
    void * opaque;
}dirdef_t;

int dir_open(dirnext_t dirnext, dirclose_t dirclose, void * opaque);
int dir_is_open(int fd);
int dir_next(int dird, void * buf, size_t bufsize);
int dir_close(int dird);
void dir_set_opaque(int dird, void * opaque);

#endif
