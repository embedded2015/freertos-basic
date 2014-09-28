#include <FreeRTOS.h>
#include <semphr.h>
#include "dir.h"
#include "string.h"
#include "hash-djb2.h"

static dirdef_t dirds[MAX_DIRS];
static xSemaphoreHandle dir_sem = NULL;

__attribute__((constructor)) void dir_init() {
    memset(dirds, 0, sizeof(dirds));
    dir_sem = xSemaphoreCreateMutex();
}

static dirdef_t * dir_getdird(int dird){
    if(dird < 0 || dird >= MAX_DIRS)
        return NULL;
    else return dirds + dird;
}

static int dir_is_open_int(int dird){
    dirdef_t * d = dir_getdird(dird);
    if(d == NULL)return 0;
    else return !((d -> dirnext == NULL) &&
                  (d -> dirclose == NULL) &&
                  (d -> opaque == NULL));
}

static int dir_finddird(){
    int i;
    for(i = 0; i < MAX_DIRS; ++i){
        if(!dir_is_open(i))return i;
    }
    return -1;
}

int dir_is_open(int dird){
    int r;
    xSemaphoreTake(dir_sem, portMAX_DELAY); 
    r = dir_is_open_int(dird);
    xSemaphoreGive(dir_sem);
    return r;
}

int dir_open(dirnext_t dirnext, dirclose_t dirclose, void * opaque){
    int dird;

    xSemaphoreTake(dir_sem, portMAX_DELAY); 
    dird = dir_finddird();
    if(dird > 0){
        dirds[dird].dirnext = dirnext;
        dirds[dird].dirclose = dirclose;
        dirds[dird].opaque = opaque;
    }
    xSemaphoreGive(dir_sem);

    return dird;
}

int dir_next(int dird, void * buf, size_t bufsize){
    if(dir_is_open_int(dird)){
        if(dirds[dird].dirnext){
            return dirds[dird].dirnext(dirds[dird].opaque, buf, bufsize); 
        }else{
            return ENOTSUPPORT;
        }
    }else{
        return ENOTOPEN;
    }
}

int dir_close(int dird){
    if(dir_is_open_int(dird)){
        int r = 0;
        if(dirds[dird].dirclose){
            r = dirds[dird].dirclose(dirds[dird].opaque); 
        }
        xSemaphoreTake(dir_sem, portMAX_DELAY); 
        memset(dirds + dird,0,sizeof(dirdef_t));
        xSemaphoreGive(dir_sem);
        return r;
    }else{
        return ENOTOPEN;
    }
}

void dir_set_opaque(int dird, void * opaque){
    if(dir_is_open_int(dird))
        dirds[dird].opaque = opaque;

}

// TODO add dev,romfs's dir* function implementation



