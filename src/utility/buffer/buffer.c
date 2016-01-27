#include <stdlib.h>
#include <config.h>

typedef struct {                        //!< 大块结构体
    long sem_hdl[BUFFER_BLOCK_NUM];     //!< 信号量句柄数组，控制各块指针访问
    char * b[BUFFER_BLOCK_NUM];         //!< 块指针数组
}BUF_BIG_BLOCK_T;
typedef struct {                        //!< 缓存结构体
    unsigned int limit;                 //!< 缓存上限，为BUFFER_BLOCK_SIZE倍数
    unsigned long long count;           //!< 数据量计数
    long sem_hdl;                       //!< 信号量句柄，控制计数
    BUF_BIG_BLOCK_T * bb[BUFFER_BIG_BLOCK_NUM]; //!< 大块指针数组
}BUF_T;

enum BUF_OPE_MODES_E {                  //!< 缓存操作模式
    BUF_WRITE,                          //!< 写缓存
    BUF_READ,                           //!< 读缓存
    BUF_MARK,                           //!< 缓存标记
    BUF_UNMARK,                         //!< 缓存标记撤销
};
static const char * s_buf_ope_modes[] = {   //!< 模式提示
    "Write",
    "Read",
    "Mark",
    "Unmark",
};
struct BUF_OPE_PARAMS_T {               //!< 缓存操作参数
    char * buf;                         //!< 外部缓存
    unsigned int length;                //!< 操作长度
    unsigned long long pos;             //!< 位置
    long hdl;                           //!< 缓存指针
};
static int buf_operate(struct BUF_OPE_PARAMS_T * p, int mode)
{
    BUF_T * hdl = (BUF_T *)p->hdl;
    char * buf = p->buf;
    unsigned int len = p->length;
    unsigned int allofs = (unsigned int)(p->pos % (long long)hdl->limit);
    unsigned int blkidx = allofs / BUFFER_BLOCK_SIZE;
    unsigned int blkofs = allofs % BUFFER_BLOCK_SIZE;
    unsigned int blkcnt = 0;
    unsigned int k = 0;
    dbg_out_I(DS_BUFFER, "Buffer operate, mode:%s, length:%d, pos:%d",
            s_buf_ope_modes[mode], p->length, p->pos);
    while(1) {
        dbg_out(DS_BUFFER, ".");
        unsigned int i = blkidx / BUFFER_BLOCK_NUM;
        unsigned int j = blkidx % BUFFER_BLOCK_NUM;
        if(hdl->bb[i] == NULL) {
            hdl->bb[i] = (BUF_BIG_BLOCK_T *)malloc(sizeof(BUF_BIG_BLOCK_T));
            if(hdl->bb[i] == NULL) {
                dbg_outerr_E(DS_BUFFER_ERR, "malloc");
                return CONF_BUFFER_RET_MEM;
            }
            memset(hdl->bb[i], 0x00, sizeof(BUF_BIG_BLOCK_T));
            dbg_out_I(DS_BUFFER, "Malloc big block: %#x", hdl->bb[i]);
        }
        if(hdl->bb[i]->b[j] == NULL) {
            hdl->bb[i]->b[j] = (char *)malloc(BUFFER_BLOCK_SIZE);
            if(hdl->bb[i]->b[j] == NULL) {
                dbg_outerr_E(DS_BUFFER_ERR, "malloc");
                return CONF_BUFFER_RET_MEM;
            }
            memset(hdl->bb[i]->b[j], 0x00, sizeof(BUFFER_BLOCK_SIZE));
            if(bio_sem()->new(1, &hdl->bb[i]->sem_hdl[j]) < 0) {
                dbg_out_E(DS_BUFFER_ERR, "Create sem failed");
                return CONF_BUFFER_RET_SEM_ERR;
            }
            dbg_out_I(DS_BUFFER, "Malloc block: %#x", hdl->bb[i]->b[j]);
        }
        if(k || blkofs) {
            int len_t = (len < (BUFFER_BLOCK_SIZE - blkofs))
                ? len : (BUFFER_BLOCK_SIZE - blkofs);
            switch(mode) {
                case BUF_WRITE: {
                    memmove(&hdl->bb[i]->b[j][blkofs], buf, len_t);
                    dbg_out_I(DS_BUFFER, "[%d][%d][%d]Write:%d",
                            i, j, blkofs, len_t);
                    break;
                }
                case BUF_READ: {
                    memmove(buf, &hdl->bb[i]->b[j][blkofs], len_t);
                    dbg_out_I(DS_BUFFER, "[%d][%d][%d]Read:%d",
                            i, j, blkofs, len_t);
                    break;
                }
                case BUF_MARK: {
                    bio_sem()->wait(-1, hdl->bb[i]->sem_hdl[j]);
                    dbg_out_I(DS_BUFFER, "[%d][%d][%d]Mark:%d",
                            i, j, blkofs, len_t);
                    break;
                }
                case BUF_UNMARK: {
                    bio_sem()->post(hdl->bb[i]->sem_hdl[j]);
                    dbg_out_I(DS_BUFFER, "[%d][%d][%d]Unmark:%d",
                            i, j, blkofs, len_t);
                    break;
                }
            }
            blkofs = 0;
            buf += len_t;
            len -= len_t;
            if(len <= 0) {
                break;                  //!< 剩余数据长度长度为0时，退出
            }
            if(++blkcnt >= (hdl->limit / BUFFER_BLOCK_SIZE)) {
                break;                  //!< 操作的块数超出总数时，退出
            }
            if(++blkidx >= (hdl->limit / BUFFER_BLOCK_SIZE)) {
                blkidx = 0;             //!< 块下标到达末块时，从第0块开始
            }
        }
        k++;
    }
    return (p->length - len);
}
/** \brief       添加数据到缓存 */
int buf_append(char * buf, unsigned int length, long p)
{
    dbg_out_I(DS_BUFFER, "Buffer write, length:%d", length);
    BUF_T * hdl = (BUF_T *)p;
    if(hdl == NULL) {
        dbg_out_E(DS_BUFFER_ERR, "Bad param, p:%#x", hdl);
        return CONF_BUFFER_RET_PARAM_ERR;
    }
    int ret;
    int len = length;
    struct BUF_OPE_PARAMS_T o = { buf, length, hdl->count, p, };
    while(1) {
        bio_sem()->wait(-1, hdl->sem_hdl);
        if((ret = buf_operate(&o, BUF_MARK)) < 0) { //!< 锁定要操作的块
            bio_sem()->post(hdl->sem_hdl);
            break;
        }
        hdl->count += ret;              //!< 更新长度计数，防止其他线程读旧数据
        bio_sem()->post(hdl->sem_hdl);
        if((ret = buf_operate(&o, BUF_WRITE)) < 0) {    //!< 写数据
            break;
        }
        if((ret = buf_operate(&o, BUF_UNMARK)) < 0) {   //!< 完成操作，解锁
            break;
        }
        len -= ret;
        if(len <= 0) {
            return 0;
        }
        o.length = len;
    }
    return ret;
}
/** \brief       读取数据 */
int buf_read(char * buf, unsigned int length, long long pos, long p)
{
    dbg_out_I(DS_BUFFER, "Buffer read, length:%d, pos:%d", length, pos);
    BUF_T * hdl = (BUF_T *)p;
    if(hdl == NULL) {
        dbg_out_E(DS_BUFFER_ERR, "Bad param, p:%#x", hdl);
        return CONF_BUFFER_RET_PARAM_ERR;
    }
    int ret;
    int len = length;
    struct BUF_OPE_PARAMS_T o = { buf, length, pos, p, };
    while(1) {
        bio_sem()->wait(-1, hdl->sem_hdl);
        if(o.pos >= hdl->count) {
            bio_sem()->post(hdl->sem_hdl);
            dbg_out_E(DS_BUFFER_ERR, "Bad param, pos:%d, length:%d, count:%d",
                    o.pos, hdl->count);
            return CONF_BUFFER_RET_PARAM_ERR;   //!< 读取位置超出范围
        }
        if(o.pos + hdl->limit < hdl->count) {
            bio_sem()->post(hdl->sem_hdl);
            return CONF_BUFFER_RET_DATALOST;    //!< 读取位置的数据已被覆盖
        }
        if(o.pos + o.length > hdl->count) {
            o.length = hdl->count - o.pos;  //!< 读取长度超出范围，调整读取长度
            len = o.length;
            dbg_out_I(DS_BUFFER, "Read length %d out of range, change to %d.",
                    length, o.length);
        }
        if((ret = buf_operate(&o, BUF_MARK)) < 0) { //!< 锁定要操作的块
            bio_sem()->post(hdl->sem_hdl);
            break;
        }
        bio_sem()->post(hdl->sem_hdl);
        if((ret = buf_operate(&o, BUF_READ)) < 0) { //!< 读数据
            break;
        }
        if((ret = buf_operate(&o, BUF_UNMARK)) < 0) {   //!< 完成操作，解锁
            break;
        }
        len -= ret;
        if(len <= 0) {
            return o.length;
        }
        o.length = len;
    }
    return ret;
}
/** \brief       获取数据长度 */
unsigned long long buf_size(long p)
{
    BUF_T * hdl = (BUF_T *)p;
    if(hdl == NULL) {
        dbg_out_E(DS_BUFFER_ERR, "Bad param, p:%#x", hdl);
        return CONF_BUFFER_RET_PARAM_ERR;
    }
    bio_sem()->wait(-1, hdl->sem_hdl);
    unsigned long long cnt = hdl->count;
    bio_sem()->post(hdl->sem_hdl);
    return cnt;
}
/** \brief       初始化数据缓存 */
int buf_new(unsigned int limit, long * p)
{
    BUF_T * hdl = (BUF_T *)malloc(sizeof(BUF_T));
    if(hdl == NULL) {
        dbg_outerr_E(DS_BUFFER_ERR, "malloc");
        return CONF_BUFFER_RET_MEM;
    }
    memset(hdl, 0x00, sizeof(BUF_T));
    int lmt = limit;
    if(limit > BUFFER_MAXSIZE) {
        dbg_out_E(DS_BUFFER_ERR, "Limit %d out of range", limit);
        return CONF_BUFFER_RET_PARAM_ERR;
    }
    if(limit % BUFFER_BLOCK_SIZE) {
        lmt = (limit / BUFFER_BLOCK_SIZE + 1) * BUFFER_BLOCK_SIZE;
        dbg_out_W(DS_BUFFER_ERR, "Bad limit:%d, fill to %d", limit, lmt);
    }
    hdl->limit = lmt;
    if(bio_sem()->new(1, &hdl->sem_hdl) < 0) {
        dbg_out_E(DS_BUFFER_ERR, "Create semaphore failed");
        return CONF_BUFFER_RET_SEM_ERR;
    }
    *p = (long)hdl;
    dbg_out_I(DS_BUFFER, "Create new buffer entry: %#x", hdl);
    return 0;
}
/** \brief       删除数据缓存 */
int buf_del(long *p)
{
    BUF_T * hdl = (BUF_T *)(*p);
    if(hdl == NULL) {
        dbg_out_E(DS_BUFFER_ERR, "Bad param, p:%#x", hdl);
        return CONF_BUFFER_RET_PARAM_ERR;
    }
    dbg_out_I(DS_BUFFER, "Buffer delete entry: %#x", hdl);
    unsigned int blockcount = hdl->limit / BUFFER_BLOCK_SIZE;
    unsigned int i, j, k;
    for(i = 0, j = 0; j < BUFFER_BIG_BLOCK_NUM && i < blockcount; j++) {
        if(hdl->bb[j] == NULL) {
            dbg_out_I(DS_BUFFER, "Bad big block[%d] address.", j);
            i += BUFFER_BLOCK_NUM;
            continue;
        }
        for(k = 0; k < BUFFER_BLOCK_NUM && i < blockcount; k++, i++) {
            if(hdl->bb[j]->b[k] == NULL) {
                dbg_out_I(DS_BUFFER, "Bad block[%d][%d] address.", j, k);
                continue;
            }
            dbg_out_I(DS_BUFFER, "\t > free block:%#x", hdl->bb[j]->b[k]);
            free(hdl->bb[j]->b[k]);
            hdl->bb[j]->b[k] = NULL;
            bio_sem()->del(&hdl->bb[j]->sem_hdl[k]);
        }
        dbg_out_I(DS_BUFFER, " > free big block:%#x", hdl->bb[j]);
        free(hdl->bb[j]);
        hdl->bb[j] = NULL;
    }
    bio_sem()->del(&hdl->sem_hdl);
    dbg_out_I(DS_BUFFER, "free entry:%#x", hdl);
    free(hdl);
    *p = (long)NULL;

    return 0;
}


