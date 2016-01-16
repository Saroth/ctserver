#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <config.h>

#define BUFFER_BLOCK_SIZE   4096        //!< 每次申请内存块大小

typedef enum {                          //!< 返回值定义
    CONF_BUFFER_RET = ERR_CODE_BUFFER,  //!< 起始码
    CONF_BUFFER_RET_MEM,                //!< 内存申请错误，或内存不足
}BUFFER_RET_E;
typedef struct {
    char * addr;                        //!< 
    struct list_head ptr;               //!< 链表节点
}BUF_NODE_T;
typedef struct {                        //!< 缓存结构体
    struct list_head * list;            //!< 申请的缓存链表
    unsigned int limit;                 //!< 缓存上限，为BUFFER_BLOCK_SIZE倍数
    unsigned int count;                 //!< 数据计数, 上限4G
}BUF_T;

/**
 * \brief       初始化数据缓存
 * \param       limit       缓存上限
 * \return      >0:缓存结构体指针   <=0:Error code:BUFFER_RET_E
 */
BUF_T * buf_new(unsigned int limit);
int buf_append(char * buf, unsigned int length, BUF_T * p);
unsigned int buf_get(char * buf, unsigned int length, BUF_T * p);
unsigned int buf_get_count(BUF_T * p);
/**
 * \brief       
 * \param       buf
 * \param       length
 * \param       pos
 * \param       p
 * \return      
 */
int buf_update(char * buf, unsigned int length, unsigned int pos, BUF_T * p);

#endif /* __BUFFER_H__ */

