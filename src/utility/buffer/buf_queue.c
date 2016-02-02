#include <stdlib.h>
#include <unistd.h>
#include <config.h>

typedef struct {
    unsigned int size;                  //!< 缓存大小
    long hdl_shm;                       //!< 共享内存句柄
    long addr;                          //!< 共享内存地址
    long hdl_sem;                       //!< 读写指针控制信号量
    unsigned int ptr_w;                 //!< <sem>写指针(偏移量)
    unsigned int ptr_r;                 //!< <sem>读指针(偏移量)
}BUF_QUEUE_T;

/** \brief       初始化数据缓存 */
int buf_queue_new(char * name, unsigned long size, long * hdl)
{
    /// 申请缓存
    BUF_QUEUE_T * q = (BUF_QUEUE_T *)malloc(sizeof(BUF_QUEUE_T));
    if(q == NULL) {
        dbg_outerr_E(DS_BUF_QUEUE_ERR, "malloc");
        return BUF_RET_MEM;
    }
    memset(q, 0x00, sizeof(BUF_QUEUE_T));
    q->size = size;
    /// 申请信号量
    int ret = bio_sem()->new(1, &q->hdl_sem);
    if(ret) {
        dbg_out_E(DS_BUF_QUEUE_ERR, "Get new semaphore failed!");
        free(q);
        q = NULL;
        return BUF_RET_SEM;
    }
    /// 申请共享内存
    char filename[256];
    int i = 0;
    do {
        sprintf(filename, "%s%s%d", BUF_TMPFILE_PREFIX, name, i++);
        if(access(filename, F_OK) == 0) {
            dbg_out_W(DS_BUF_QUEUE_ERR, "File `%s` exist, retry", filename);
            continue;
        }
        ret = bio_shm()->new(SHM_TYPE_FILE, filename, q->size, &q->hdl_shm);
        if(ret) {
            dbg_out_E(DS_BUF_QUEUE_ERR, "Get new shared memory failed!");
            bio_sem()->del(&q->hdl_sem);
            free(q);
            q = NULL;
            return BUF_RET_SHM;
        }
        q->addr = (long)bio_shm()->addr(q->hdl_shm);
        break;
    } while(1);
    *hdl = (long)q;
    return 0;
}
/** \brief       删除数据缓存 */
int buf_queue_del(long * hdl)
{
    BUF_QUEUE_T * q = (BUF_QUEUE_T *)(*hdl);
    if(q == NULL) {
        dbg_out_E(DS_BUF_QUEUE_ERR, "Bad handle.", q);
        return BUF_RET_PARAM_ERR;
    }
    bio_shm()->del(&q->hdl_shm);
    bio_sem()->del(&q->hdl_sem);
    free(q);
    *hdl = (long)NULL;
    return 0;
}
/** \brief       添加数据到缓存队列 */
int buf_queue_append(char * buf, unsigned int length, int waittime, long hdl)
{
    BUF_QUEUE_T * q = (BUF_QUEUE_T *)hdl;
    if(buf == NULL || hdl == 0 || length >= q->size - 1
            || (waittime < 0 && waittime != -1)) {
        dbg_out_E(DS_BUF_QUEUE_ERR,
                "Bad param, buf:%#x, hdl:%#x, length:%d, waittime:%d",
                buf, hdl, length, waittime);
        return BUF_RET_PARAM_ERR;
    }
    while(1) {
        bio_sem()->wait(-1, q->hdl_sem);
        unsigned long ptr_a = (q->ptr_w + length) % q->size;
        if((q->ptr_w < ptr_a) && (q->ptr_r <= q->ptr_w || q->ptr_r > ptr_a)) {
            memmove((char *)q->addr + q->ptr_w, buf, length);
            q->ptr_w = ptr_a;
            bio_sem()->post(q->hdl_sem);
            return length;
        }
        else if(q->ptr_r <= q->ptr_w && q->ptr_r > ptr_a) {
            int left = q->size - q->ptr_w;
            memmove((char *)q->addr + q->ptr_w, buf, left);
            memmove((char *)q->addr, buf + left, length - left);
            q->ptr_w = ptr_a;
            bio_sem()->post(q->hdl_sem);
            return length;
        }
        if(waittime == -1) {
            ;                           //!< 永久等待
        }
        else if(waittime-- <= 0) {
            bio_sem()->post(q->hdl_sem);
            dbg_out_E(DS_BUF_QUEUE_ERR, "Buffer space not enough!");
            return BUF_RET_TIMEOUT;     //!< 超时退出
        }
        dbg_out_I(DS_BUF_QUEUE,
                "Wait queue(%ds), size:%d, pr:%d, pw:%d, length:%d",
                waittime, q->size, q->ptr_r, q->ptr_w, length);
        bio_sem()->post(q->hdl_sem);
        sleep(1);                       //!< 等待读取后留出空间
    }
    return 0;
}
/** \brief       读取数据 */
int buf_queue_read(char * buf, unsigned int length, long hdl)
{
    BUF_QUEUE_T * q = (BUF_QUEUE_T *)hdl;
    if(buf == NULL || hdl == 0 || length >= q->size - 1) {
        dbg_out_E(DS_BUF_QUEUE_ERR,
                "Bad param, buf:%#x, hdl:%#x, length:%d", buf, hdl, length);
        return BUF_RET_PARAM_ERR;
    }
    unsigned int len = 0;
    bio_sem()->wait(-1, q->hdl_sem);
    while(q->ptr_r != q->ptr_w && len < length) {
        *(buf + len) = *((char *)q->addr + q->ptr_r);
        if(++q->ptr_r >= q->size) {
            q->ptr_r = 0;
        }
        len++;
    }
    bio_sem()->post(q->hdl_sem);
    return len;
}
/** \brief       获取未读取的数据长度 */
unsigned int buf_queue_size(long hdl)
{
    BUF_QUEUE_T * q = (BUF_QUEUE_T *)hdl;
    if(q == NULL) {
        dbg_out_E(DS_BUF_QUEUE_ERR, "Bad handle.", q);
        return BUF_RET_PARAM_ERR;
    }
    bio_sem()->wait(-1, q->hdl_sem);
    int len = q->ptr_w - q->ptr_r;
    bio_sem()->post(q->hdl_sem);
    return (len + q->size) % q->size;
}

