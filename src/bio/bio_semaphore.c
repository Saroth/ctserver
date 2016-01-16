#include <stdlib.h>
#include <string.h>
#include <config.h>

#if defined(CFG_SYS_UNIX)
#include <sys/sem.h>
#include <errno.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                               (Linux-specific) */
};
#define BIO_UNIX_SEM_ID 0x10            //!< 获取key时使用的ID，[0, 0xff]
typedef struct {                        //!< 信号量集信息结构体
    unsigned long key;                  //!< 用于申请信号量集的key
    unsigned int id;                    //!< 申请获得的信号量集id
    unsigned int num;                   //!< 信号量集的信号量数
}BIO_SEM_HDL_T;

static int unix_sem_pv(unsigned short num, long hdl, unsigned short op,
        unsigned short flg)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    struct sembuf sb;
    sb.sem_num = num;
    sb.sem_op = op;
    sb.sem_flg = flg;                   //!< SEM_UNDO:等待  IPC_NOWAIT:不等待
    if(semop(sem->id, &sb, 1) < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semop, op:%d, flg:%d", op, flg);
        return -1;
    }
    return 0;
}
static inline int unix_sem_p(unsigned short num, long hdl)
{
    return unix_sem_pv(num, hdl, -1, SEM_UNDO);
}
static inline int unix_sem_v(unsigned short num, long hdl)
{
    return unix_sem_pv(num, hdl, 1, SEM_UNDO);
}
static int unix_sem_val(unsigned short num, long hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    int ret = semctl(sem->id, num, GETVAL);
    if(ret < 0) {
        dbg_outerr_I(DS_SEM_ERR, "smectl, GETVAL");
    }
    return ret;
}
static int unix_sem_setval(unsigned short num, int val, long hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)hdl;
    union semun arg;
    arg.val = val;
    int ret = semctl(sem->id, num, SETVAL, arg);
    if(ret < 0) {
        dbg_outerr_I(DS_SEM_ERR, "smectl, SETVAL");
    }
    return ret;
}
static int unix_sem_new(unsigned short num, int val, long * hdl)
{
    int id = BIO_UNIX_SEM_ID;
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)malloc(sizeof(BIO_SEM_HDL_T));
    if(sem == NULL) {
        dbg_outerr_I(DS_SEM_ERR, "malloc");
        return -1;
    }
    memset(sem, 0x00, sizeof(BIO_SEM_HDL_T));
    do {
        sem->key = ftok(".", id);       //!< 获取key
        if(sem->key < 0) {
            dbg_outerr_I(DS_SEM_ERR, "ftok");
            return -1;
        }
        sem->id = semget(sem->key, num, IPC_CREAT | IPC_EXCL);
        if(sem->id >= 0) {
            break;                      //!< 获得信号量集id
        }
        if(errno == EEXIST) {           //!< 信号量集id已存在
            dbg_outerr_I(DS_SEM_ERR, "semget, key:%d, ID:%d", sem->key, id);
            if(id++ >= BIO_UNIX_SEM_ID + 0x10) {
                dbg_out_E(DS_SEM_ERR, "Get semaphore failed! No available id");
                return -1;              //!< 重试仍然失败
            }
            dbg_out_W(DS_SEM_ERR, "Retry ID: %d", id);
            continue;                   //!< 使用新ID重试
        }
        dbg_outerr_I(DS_SEM_ERR, "semget"); //!< 其他错误
        return -1;
    } while(1);
    unsigned short i = 0;
    for(; i < num; i++) {               //!< 设置初始值
        if(unix_sem_setval(i, val, (long)sem)) {
            return -1;
        }
    }
    sem->num = num;
    *hdl = (long)(&sem);
    dbg_out_I(DS_SEM, "Get new semaphore, id: %d, key: %d", sem->id, sem->key);
    return 0;
}
static int unix_sem_del(long * hdl)
{
    BIO_SEM_HDL_T * sem = (BIO_SEM_HDL_T *)(*hdl);
    if(semctl(sem->id, 0, IPC_RMID) < 0) {
        dbg_outerr_I(DS_SEM_ERR, "semctl, IPC_RMID");
        return -1;
    }
    free(sem);
    *hdl = (long)NULL;
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

