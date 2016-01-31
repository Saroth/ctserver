#include <config.h>

#if defined(CFG_SYS_UNIX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct {
    int type;                           //!< 映射类型, BIO_SHM_TYPE_E
    char name[128];                     //!< 文件名
    unsigned int size;                  //!< 空间大小
    int fd;                             //!< shm_open返回的描述符
    long addr;                          //!< mmap返回的地址
}BIO_SHM_HDL_T;

int unix_shm_new(int type, char *name, unsigned int size, long *addr, long *hdl)
{
    int ret = 0;
    int prot = PROT_READ | PROT_WRITE;
    int flag = MAP_SHARED;
    BIO_SHM_HDL_T * p = (BIO_SHM_HDL_T *)malloc(sizeof(BIO_SHM_HDL_T));
    if(p == NULL) {
        dbg_outerr_E(DS_SHM_ERR, "malloc");
        return -1;
    }
    memset(p, 0x00, sizeof(BIO_SHM_HDL_T));
    p->type = type;
    p->size = size;
    switch(type) {
        case SHM_TYPE_SHM: {
            p->fd = shm_open(name, O_RDWR | O_CREAT, 0644);
            if(p->fd < 0) {
                dbg_outerr_E(DS_SHM_ERR, "shm_open");
                ret = -1;
                break;
            }
            if(ftruncate(p->fd, p->size)) {
                dbg_outerr_E(DS_SHM_ERR, "ftruncate");
                shm_unlink(name);
                ret = -1;
            }
            break;
        }
        case SHM_TYPE_FILE: {
            p->fd = open(name, O_RDWR | O_CREAT, 0644);
            if(p->fd < 0) {
                dbg_outerr_E(DS_SHM_ERR, "open");
                ret = -1;
                break;
            }
            if(ftruncate(p->fd, p->size)) {
                dbg_outerr_E(DS_SHM_ERR, "ftruncate");
                close(p->fd);
                ret = -1;
            }
            break;
        }
        case SHM_TYPE_ANONYMOUS: {
            p->fd = -1;
            flag |= MAP_ANON;
            break;
        }
        default: {
            free(p);
            dbg_out_E(DS_SHM_ERR, "Bad param, type:%d", type);
            ret = -1;
            break;
        }
    }
    if(name) {
        strncpy(p->name, name, sizeof(p->name));
    }
    if(ret == 0) {
        p->addr = (long)mmap(NULL, size, prot, flag, p->fd, 0);
        if((void *)p->addr == MAP_FAILED) {
            dbg_outerr_E(DS_SHM_ERR, "mmap");
            ret = -1;
        }
    }
    if(ret) {
        free(p);
    }
    else {
        *addr = p->addr;
    }
    return ret;
}
int unix_shm_del(long * hdl)
{
    BIO_SHM_HDL_T * p = (BIO_SHM_HDL_T *)(*hdl);
    if(p == NULL) {
        dbg_out_E(DS_SHM_ERR, "Bad handle.");
        return -1;
    }
    switch(p->type) {
        case SHM_TYPE_SHM: {
            shm_unlink(p->name);
            break;
        }
        case SHM_TYPE_FILE: {
            close(p->fd);
            break;
        }
        case SHM_TYPE_ANONYMOUS: {
            break;
        }
    }
    munmap((void *)p->addr, p->size);
    free(p);
    *hdl = (long)NULL;
    return 0;
}
static BIO_SHM_T s_bio_shm_unix = {
    .desc       = "POSIX semaphore",
    .new        = unix_shm_new,
    .del        = unix_shm_del,
};
#endif /* defined(CFG_SYS_UNIX) */

static BIO_SHM_T * s_bio_shm = NULL;
int bio_shm_init(void * p)
{
    if(p) {
        s_bio_shm = (BIO_SHM_T *)p;
    }
    else {
#if defined(CFG_SYS_UNIX)
        s_bio_shm= &s_bio_shm_unix;
#endif /* defined(CONF_SYS_UNIX) */
    }
    return 0;
}
inline BIO_SHM_T * bio_shm(void)
{
    return s_bio_shm;
}


