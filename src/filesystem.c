#include "osdebug.h"
#include "filesystem.h"
#include "fio.h"

#include <stdint.h>
#include <string.h>
#include <hash-djb2.h>

#define MAX_FS 16

struct fs_t {
    uint32_t hash;
    fs_open_t cb;
    fs_open_dir_t dcb;
    void * opaque;
};

static struct fs_t fss[MAX_FS];

__attribute__((constructor)) void fs_init() {
    memset(fss, 0, sizeof(fss));
}

int register_fs(const char * mountpoint, fs_open_t callback, fs_open_dir_t dir_callback, void * opaque) {
    int i;
    DBGOUT("register_fs(\"%s\", %p, %p, %p)\r\n", mountpoint, callback, dir_callback, opaque);
    
    for (i = 0; i < MAX_FS; i++) {
        if (!fss[i].cb) {
            fss[i].hash = hash_djb2((const uint8_t *) mountpoint, -1);
            fss[i].cb = callback;
            fss[i].dcb = dir_callback;
            fss[i].opaque = opaque;
            return 0;
        }
    }
    
    return -1;
}

int fs_open(const char * path, int flags, int mode) {
    const char * slash;
    uint32_t hash;
    int i;
//    DBGOUT("fs_open(\"%s\", %i, %i)\r\n", path, flags, mode);
    
    while (path[0] == '/')
        path++;
    
    slash = strchr(path, '/');
    
    if (!slash)
        return -2;

    hash = hash_djb2((const uint8_t *) path, slash - path);
    path = slash + 1;

    for (i = 0; i < MAX_FS; i++) {
        if (fss[i].hash == hash)
            return fss[i].cb(fss[i].opaque, path, flags, mode);
    }
    
    return -2;
}

static int root_opendir(){
    return OPENDIR_NOTFOUNDFS;        
}

int fs_opendir(const char * path){
    const char * slash;
    uint32_t hash;
    
    if ( path[0] == '\0' || (path[0] == '/' && path[1] == '\0') ){
        return root_opendir();
    }
    
    while (path[0] == '/')
        path++;
    
    slash = strchr(path, '/');
    if (!slash)
        slash = path + strlen(path);

    hash = hash_djb2((const uint8_t *) path, slash - path);
    
    if(*(slash) == '\0'){
        path = "";
    }else{
        path = slash + 1;
    }
    
    for (int i = 0; i < MAX_FS; i++) {
        if (fss[i].hash == hash)
            return fss[i].dcb(fss[i].opaque, path);
    }    

    return OPENDIR_NOTFOUNDFS;
}

