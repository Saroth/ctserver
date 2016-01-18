#include <config.h>

#if defined(CFG_SYS_UNIX)
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/sem.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                               (Linux-specific) */
};
#define BIO_UNIX_SEM_ID 0x1             //!< 获取key时使用的ID，[0, 0xff]
typedef struct {                        //!< 信号量集信息结构体
    int key;                            //!< 用于申请信号量集的key
    int id;                             //!< 申请获得的信号量集id
    int num;                            //!< 信号量集的信号量数
}BIO_SEM_HDL_T;

static inline int unix_sem_p(int num, long wait, long hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    struct sembuf sb;
    sb.sem_num = num;
    sb.sem_op = -1;
    if(wait == 0) {
        sb.sem_flg = IPC_NOWAIT;        //!< 不等待
    }
    else {
        sb.sem_flg = SEM_UNDO;          //!< 永久等待
    }
    // struct timespec t;
    // struct timespec *tp = NULL;
    // if(wait > 0) {
    //     t.tv_sec = wait;
    //     tp = &t;
    // }
    // if(semtimedop(sem->id, &sb, 1, tp) < 0) {
    //     dbg_outerr_I(DS_SEM_ERR, "semtimedop, wait:%d", wait);
    //     return -1;
    // }
    if(semop(sem->id, &sb, 1) < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semop, wait:%d", wait);
        return -1;
    }
    return 0;
}
static inline int unix_sem_v(int num, long hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    struct sembuf sb;
    sb.sem_num = num;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    if(semop(sem->id, &sb, 1) < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semop, V, num:%d", sb.sem_num);
        return -1;
    }
    return 0;
}
static int unix_sem_val(int num, long hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    int ret = semctl(sem->id, num, GETVAL);
    if(ret < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semctl, GETVAL, id:%d num:%d", sem->id, num);
    }
    return ret;
}
static int unix_sem_setval(int num, int val, long hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    union semun arg;
    arg.val = val;
    int ret = semctl(sem->id, num, SETVAL, arg);
    if(ret < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semctl, SETVAL, id:%d num:%d, val:%d",
                sem->id, num, val);
    }
    return ret;
}
static int sem_new(char * name, int num, int val, int id, int mode, long * hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)malloc(sizeof(BIO_SEM_HDL_T));
    if(sem == NULL) {
        dbg_outerr_I(DS_SEM_ERR, "malloc");
        return -1;
    }
    memset(sem, 0x00, sizeof(BIO_SEM_HDL_T));
    do {
        sem->key = ftok(name, id);       //!< 获取key
        if(sem->key < 0) {
            dbg_outerr_I(DS_SEM_ERR, "ftok");
            return -1;
        }
        sem->id = semget(sem->key, num, mode);
        if(sem->id > 0) {
            break;                      //!< 获得信号量集id
        }
        dbg_outerr_I(DS_SEM_ERR, "semget, key:%x, ID:%d", sem->key, id);
#ifdef BIO_SEM_RETRY_WITH_NEW_ID
        if(errno == EEXIST && id++ < BIO_UNIX_SEM_ID + 0x10) {
            dbg_out_W(DS_SEM_ERR, "Retry ID: %d", id);
            continue;                   //!< 使用新ID重试
        }
#endif /* BIO_SEM_RETRY_WITH_NEW_ID */
        return -1;
    } while(1);
    int i = 0;
    for(; i < num; i++) {               //!< 设置初始值
        if(unix_sem_setval(i, val, (long)sem)) {
            return -1;
        }
    }
    sem->num = num;
    *hdl = (long)sem;
    dbg_out_I(DS_SEM, "Get new semaphore, id:%d, key:%x, name:%s",
            sem->id, sem->key, name);
    return 0;
}
static int unix_sem_new(char * name, int num, int val, long * hdl)
{
    return sem_new(name, num, val, BIO_UNIX_SEM_ID,
            0666 | IPC_CREAT | IPC_EXCL, hdl);
}
static int unix_sem_del(long * hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)(*hdl);
    if(sem == NULL) {
        dbg_out_E(DS_SEM_ERR, "semctl, IPC_RMID: Bad handle.");
        return -1;
    }
    dbg_out_I(DS_SEM, "Delete semaphore, id:%d, key:%x", sem->id, sem->key);
    if(semctl(sem->id, 0, IPC_RMID) < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semctl, IPC_RMID");
        return -1;
    }
    free(sem);
    *hdl = (long)NULL;
    return 0;
}
static int unix_sem_fdel(char * name)
{
    long hdl;
    int i = BIO_UNIX_SEM_ID;
    dbg_out_I(DS_SEM, "Force delete semaphore, create by: %s", name);
#ifdef BIO_SEM_RETRY_WITH_NEW_ID
    for(; i < BIO_UNIX_SEM_ID + 0x10; i++)
#endif /* BIO_SEM_RETRY_WITH_NEW_ID */
    {
        if(sem_new(name, 1, 1, i, IPC_EXCL, &hdl) == 0) {
            unix_sem_del(&hdl);
        }
    }
    return 0;
}
static BIO_SEM_T s_bio_sem_unix = {
    .desc       = "Unix standard I/O",
    .new        = unix_sem_new,
    .del        = unix_sem_del,
    .p          = unix_sem_p,
    .v          = unix_sem_v,
    .val        = unix_sem_val,
    .setval     = unix_sem_setval,
    .fdel       = unix_sem_fdel,
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

