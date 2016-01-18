#include <string.h>
#include <stdlib.h>
#include <config.h>

typedef struct {                        //!< 大块结构体
    long sem_hdl;                       //!< 信号量集句柄，控制块指针
    char * b[BUFFER_BLOCK_NUM];         //!< 块指针数组
}BUF_BIG_BLOCK_T;
typedef struct {                        //!< 缓存结构体
    unsigned int limit;                 //!< 缓存上限，为BUFFER_BLOCK_SIZE倍数
    long long count;                    //!< 数据量计数
    long sem_hdl;                       //!< 信号量集句柄，控制计数
    BUF_BIG_BLOCK_T * bb[BUFFER_BIG_BLOCK_NUM]; //!< 大块指针数组
}BUF_T;

int buf_new(unsigned int limit, long * p)
{
    BUF_T * hdl = (BUF_T *)malloc(sizeof(BUF_T));
    if(hdl == NULL) {
        dbg_outerr_I(DS_BUFFER_ERR, "malloc");
        return CONF_BUFFER_RET_MEM;
    }
    memset(hdl, 0x00, sizeof(BUF_T));
    if(bio_sem()->new(".", 1, 1, &hdl->sem_hdl) < 0) {
        dbg_out_E(DS_BUFFER_ERR, "Create semaphore failed");
        return CONF_BUFFER_RET_SEM_ERR;
    }
    hdl->limit = limit;
    *p = (long)hdl;
    dbg_out_I(DS_BUFFER, "Create new buffer entry: %x", hdl);
    return 0;
}
int buf_del(long *p)
{
    BUF_T * hdl = (BUF_T *)(*p);
    dbg_out_I(DS_BUFFER, "Buffer delete entry: %x", hdl);
    if(hdl == NULL) {
        dbg_out_E(DS_BUFFER, "Bad param, hdl:%x", hdl);
        return CONF_BUFFER_RET_PARAM_ERR;
    }
    unsigned int blockcount =
        (unsigned int)(hdl->count % (long long)hdl->limit) / BUFFER_BLOCK_SIZE;
    unsigned int i, j, k;
    for(i = 0, j = 0; j < BUFFER_BIG_BLOCK_NUM && i < blockcount; j++) {
        if(hdl->bb[j] == NULL) {
            dbg_out_W(DS_BUFFER_ERR, "Bad big block[%d] address.", j);
            i += BUFFER_BLOCK_NUM;
            continue;
        }
        for(k = 0; k < BUFFER_BLOCK_NUM && i < blockcount; k++, i++) {
            if(hdl->bb[j]->b[k] == NULL) {
                dbg_out_W(DS_BUFFER_ERR, "Bad block[%d][%d] address.", j, k);
                continue;
            }
            dbg_out_I(DS_BUFFER, "\t > free block:%x", hdl->bb[j]->b[k]);
            bio_sem()->del(&hdl->bb[j]->sem_hdl);
            free(hdl->bb[j]->b[k]);
            hdl->bb[j]->b[k] = NULL;
        }
        dbg_out_I(DS_BUFFER, " > free big block:%x", hdl->bb[j]);
        free(hdl->bb[j]);
        hdl->bb[j] = NULL;
    }
    bio_sem()->del(&hdl->sem_hdl);
    dbg_out_I(DS_BUFFER, "free entry:%x", hdl);
    free(hdl);
    *p = (long)NULL;

    return 0;
}
enum BUF_OPE_MODES_E {                  //!< 缓存操作模式
    BUF_WRITE,                          //!< 写缓存
    BUF_READ,                           //!< 读缓存
    BUF_MARK,                           //!< 缓存标记
    BUF_UNMARK,                         //!< 缓存标记撤销
};
static const char * s_buf_ope_modes[] = {
    "Buffer write",
    "Buffer read",
    "Buffer mark",
    "Buffer unmark",
};
struct BUF_OPE_PARAMS_T {               //!< 缓存操作参数
    char * buf;                         //!< 外部缓存
    unsigned int length;                //!< 操作长度
    long long pos;                      //!< 位置
    long hdl;                           //!< 缓存指针
};
static int buf_operate(struct BUF_OPE_PARAMS_T * p, int mode)
{
    BUF_T * hdl = (BUF_T *)p->hdl;
    dbg_out_I(DS_BUFFER, "Buffer operate, mode:%s, length:%d, pos:%d",
            s_buf_ope_modes[mode], p->length, p->pos);
    unsigned int allofs = (unsigned int)(p->pos % (long long)hdl->limit);
    unsigned int blkidx = allofs / BUFFER_BLOCK_SIZE;
    unsigned int blkofs = allofs % BUFFER_BLOCK_SIZE;
    unsigned int len = p->length;
    unsigned int i = blkidx / BUFFER_BIG_BLOCK_NUM;
    unsigned int j = blkidx % BUFFER_BIG_BLOCK_NUM;
    while(1) {
        if(len != p->length || blkofs) {
            int len_t = (len + blkofs) < BUFFER_BLOCK_SIZE
                ? len : (BUFFER_BLOCK_SIZE - blkofs);
            switch(mode) {
                case BUF_WRITE: {
                    memmove(&hdl->bb[i]->b[j][blkofs], p->buf, len_t);
                    break;
                }
                case BUF_READ: {
                    memmove(p->buf, &hdl->bb[i]->b[j][blkofs], len_t);
                    break;
                }
                case BUF_MARK: {
                    bio_sem()->p(j, -1, hdl->bb[i]->sem_hdl);
                    break;
                }
                case BUF_UNMARK: {
                    bio_sem()->v(j, hdl->bb[i]->sem_hdl);
                    break;
                }
            }
            len -= len_t;
            if(len <= 0) {
                break;
            }
            p->buf += len_t;
            if(blkidx++ >= (hdl->limit / BUFFER_BLOCK_SIZE)) {
                blkidx = 0;
            }
            blkofs = 0;
        }
        i = blkidx / BUFFER_BIG_BLOCK_NUM;
        j = blkidx % BUFFER_BIG_BLOCK_NUM;
        if(hdl->bb[i] == NULL) {
            hdl->bb[i] = (BUF_BIG_BLOCK_T *)malloc(sizeof(BUF_BIG_BLOCK_T));
            if(hdl->bb[i] == NULL) {
                dbg_outerr_I(DS_BUFFER_ERR, "malloc");
                return CONF_BUFFER_RET_MEM;
            }
            if(bio_sem()->new(".", BUFFER_BLOCK_NUM, 1, &hdl->bb[i]->sem_hdl)
                    < 0) {
                dbg_out_E(DS_BUFFER_ERR, "Create sem failed");
                return CONF_BUFFER_RET_SEM_ERR;
            }
            dbg_out_I(DS_BUFFER, "Malloc big block: %x", hdl->bb[i]);
        }
        if(hdl->bb[i]->b[j] == NULL) {
            hdl->bb[i]->b[j] = (char *)malloc(BUFFER_BLOCK_SIZE);
            if(hdl->bb[i]->b[j] == NULL) {
                dbg_outerr_I(DS_BUFFER_ERR, "malloc");
                return 0;
            }
            dbg_out_I(DS_BUFFER, "Malloc block: %x", hdl->bb[i]->b[j]);
        }
    }
    return 0;
}
int buf_append(char * buf, unsigned int length, long p)
{
    dbg_out_I(DS_BUFFER, "Buffer write, length:%d", length);
    BUF_T * hdl = (BUF_T *)p;
    int ret;
    struct BUF_OPE_PARAMS_T o;
    o.buf = buf;
    o.length = length;
    o.hdl = p;
    o.pos = hdl->count;                 //!< 从最新位置开始写
    do {
        bio_sem()->p(0, -1, hdl->sem_hdl);
        hdl->count += length;           //!< 更新长度计数，防止其他线程读旧数据
        if((ret = buf_operate(&o, BUF_MARK)) < 0) {
            bio_sem()->v(0, hdl->sem_hdl);
            break;
        }
        bio_sem()->v(0, hdl->sem_hdl);
        if((ret = buf_operate(&o, BUF_WRITE)) < 0) {
            break;
        }
        if((ret = buf_operate(&o, BUF_UNMARK)) < 0) {
            break;
        }
    } while(0);
    return ret;
}
int buf_get(char * buf, unsigned int length, long long pos, long p)
{
    dbg_out_I(DS_BUFFER, "Buffer read, length:%d, pos:%d", length, pos);
    BUF_T * hdl = (BUF_T *)p;
    int ret;
    struct BUF_OPE_PARAMS_T o;
    o.buf = buf;
    o.length = length;
    o.hdl = p;
    o.pos = pos;
    do {
        bio_sem()->p(0, -1, hdl->sem_hdl);
        if(o.pos + BUFFER_BLOCK_SIZE < hdl->count) {
            bio_sem()->v(0, hdl->sem_hdl);
            return CONF_BUFFER_RET_DATALOST;
        }
        if((ret = buf_operate(&o, BUF_MARK)) < 0) {
            bio_sem()->v(0, hdl->sem_hdl);
            break;
        }
        bio_sem()->v(0, hdl->sem_hdl);
        if((ret = buf_operate(&o, BUF_READ)) < 0) {
            break;
        }
        if((ret = buf_operate(&o, BUF_UNMARK)) < 0) {
            break;
        }
    } while(0);
    return ret;
}
int buf_update(char * buf, unsigned int length, long long pos, long p)
{
    dbg_out_I(DS_BUFFER, "Buffer update, length:%d", length);
    BUF_T * hdl = (BUF_T *)p;
    int ret;
    struct BUF_OPE_PARAMS_T o;
    o.buf = buf;
    o.length = length;
    o.hdl = p;
    o.pos = pos;
    do {
        bio_sem()->p(0, -1, hdl->sem_hdl);
        hdl->count += length;           //!< 更新长度计数，防止其他线程读旧数据
        if((ret = buf_operate(&o, BUF_MARK)) < 0) {
            bio_sem()->v(0, hdl->sem_hdl);
            break;
        }
        bio_sem()->v(0, hdl->sem_hdl);
        if((ret = buf_operate(&o, BUF_WRITE)) < 0) {
            break;
        }
        if((ret = buf_operate(&o, BUF_UNMARK)) < 0) {
            break;
        }
    } while(0);
    return ret;
}
long long buf_get_count(long p)
{
    BUF_T * hdl = (BUF_T *)p;
    bio_sem()->p(0, -1, hdl->sem_hdl);
    long long cnt = hdl->count;
    bio_sem()->v(0, hdl->sem_hdl);
    return cnt;
}


