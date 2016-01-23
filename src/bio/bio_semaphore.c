#include <config.h>

#if defined(CFG_SYS_UNIX)
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>

static int unix_sem_wait(long wait, long hdl)
{
    sem_t * sem = (sem_t *)hdl;
    int ret;
    if(wait == -1) {
        ret = sem_wait(sem);
    }
    else if(wait == 0) {
        ret = sem_trywait(sem);
    }
    else if(wait > 0) {
        struct timespec t;
        t.tv_sec = ((unsigned long)wait / 1000);
        t.tv_nsec = ((unsigned long)wait % 1000) * 1000000;
        ret = sem_timedwait(sem, &t);
    }
    else {
        return -1;
    }
    if(ret < 0 && errno != EAGAIN && errno != ETIMEDOUT) {
        dbg_outerr_E(DS_SEM_ERR, "sem_.*wait, wait:%d", wait);
        return -1;
    }
    else if(ret < 0) {
        dbg_outerr_I(DS_SEM, "sem_.*wait, wait:%d", wait);
    }
    return ret;
}
static int unix_sem_post(long hdl)
{
    sem_t * sem = (sem_t *)hdl;
    if(sem_post(sem) < 0) {
        dbg_outerr_E(DS_SEM_ERR, "sem_post");
        return -1;
    }
    return 0;
}
static int unix_sem_val(long hdl)
{
    sem_t * sem = (sem_t *)hdl;
    int val;
    if(sem_getvalue(sem, &val) < 0) {
        dbg_outerr_E(DS_SEM_ERR, "sem_getvalue");
        return -1;
    }
    if(val < 0) {
        dbg_outerr_I(DS_SEM, "sem_getvalue, val:%d", val);
        val = 0;    //!< 不返回负值(负值绝对值表示当前正在等待的线程数)
    }
    return val;
}
static int unix_sem_new(int val, long * hdl)
{
    sem_t * sem = (sem_t *)malloc(sizeof(sem_t));
    if(sem == NULL) {
        dbg_outerr_E(DS_SEM_ERR, "malloc");
        return -1;
    }
    if(sem_init(sem, 0, val) < 0) {
        free(sem);
        dbg_outerr_E(DS_SEM_ERR, "sem_init");
        return -1;
    }
    *hdl = (long)sem;
    return 0;
}
static int unix_sem_del(long * hdl)
{
    sem_t * sem = (sem_t *)(*hdl);
    if(sem == NULL) {
        dbg_out_E(DS_SEM_ERR, "sem_destroy: Bad handle.");
        return -1;
    }
    if(sem_destroy(sem) < 0) {
        dbg_outerr_E(DS_SEM_ERR, "sem_destroy");
    }
    free(sem);
    *hdl = (long)NULL;
    return 0;
}
static BIO_SEM_T s_bio_sem_unix = {
    .desc       = "POSIX semaphore",
    .new        = unix_sem_new,
    .del        = unix_sem_del,
    .wait       = unix_sem_wait,
    .post       = unix_sem_post,
    .val        = unix_sem_val,
};
#endif /* defined(CFG_SYS_UNIX) */

static BIO_SEM_T * s_bio_sem = NULL;
int bio_sem_init(void * p)
{
    if(p) {
        s_bio_sem = (BIO_SEM_T *)p;
    }
    else {
#if defined(CFG_SYS_UNIX)
        s_bio_sem= &s_bio_sem_unix;
#endif /* defined(CONF_SYS_UNIX) */
    }
    return 0;
}
inline BIO_SEM_T * bio_sem(void)
{
    return s_bio_sem;
}

