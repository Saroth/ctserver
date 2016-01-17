#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <config.h>

#define BUFFER_BLOCK_SIZE   0x40000     //!< 每次申请内存块大小
#define BUFFER_BLOCK_NUM    64          //!< 块数量
#define BUFFER_BIG_BLOCK_NUM    64      //!< 大块数量

typedef enum {                          //!< 返回值定义
    CONF_BUFFER_RET = ERR_CODE_BUFFER,  //!< 起始码
    CONF_BUFFER_RET_MEM,                //!< 内存申请错误，或内存不足
}BUFFER_RET_E;

/**
 * \brief       初始化数据缓存
 * \param       limit       缓存上限
 * \return      >0:缓存结构体指针   <=0:Error code:BUFFER_RET_E
 */
int buf_new(unsigned int limit, long * p);
int buf_del(long * p);
int buf_append(char * buf, unsigned int length, long p);
unsigned int buf_get(char * buf, unsigned int length, long p);
unsigned int buf_get_count(long p);
int buf_update(char * buf, unsigned int length, long long pos, long p);

#endif /* __BUFFER_H__ */

