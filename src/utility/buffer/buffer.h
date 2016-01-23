#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <config.h>

/*
 * \brief       数据缓存
 *              用于临时存放缓存数据。
 *              初始空间为0，数据存入时，如果空间不足，则申请一个新块。
 *              空间总量达到上限时不再申请新块，新存入的数据将覆盖最旧的数据。
 */

#define BUFFER_BLOCK_SIZE   0x10 // 0x10000     //!< 每次申请内存块大小
#define BUFFER_BLOCK_NUM    4 // 64          //!< 块数量
#define BUFFER_BIG_BLOCK_NUM    2 // 64      //!< 大块数量
#define BUFFER_MAXSIZE \
    (BUFFER_BLOCK_SIZE * BUFFER_BLOCK_NUM * BUFFER_BIG_BLOCK_NUM) //!< 内存上限
typedef enum {                          //!< 返回值定义
    CONF_BUFFER_RET = ERR_CODE_BUFFER,  //!< 起始码
    CONF_BUFFER_RET_MEM,                //!< 内存申请错误，或内存不足
    CONF_BUFFER_RET_DATALOST,           //!< 数据已失效
    CONF_BUFFER_RET_PARAM_ERR,          //!< 参数错误或超出范围
    CONF_BUFFER_RET_SEM_ERR,            //!< 信号量集申请失败
}BUFFER_RET_E;

/**
 * \brief       初始化数据缓存
 * \param       limit       缓存上限，自动补齐为内存块倍数
 * \param       p           缓存结构体指针地址
 * \return      >0:缓存结构体指针   <=0:Error code:BUFFER_RET_E
 */
int buf_new(unsigned int limit, long * p);
/**
 * \brief       删除数据缓存
 * \param       p           缓存结构体指针地址
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
int buf_del(long * p);
/**
 * \brief       添加数据到缓存
 * \param       buf         数据缓存
 * \param       length      长度
 * \param       p           缓存结构体指针
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
int buf_append(char * buf, unsigned int length, long p);
/**
 * \brief       读取数据
 * \param       buf         数据缓存
 * \param       length      长度
 * \param       pos         读取位置
 * \param       p           缓存结构体指针
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
int buf_read(char * buf, unsigned int length, long long pos, long p);
/**
 * \brief       获取数据长度
 * \param       p           缓存结构体指针
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
unsigned long long buf_size(long p);

#endif /* __BUFFER_H__ */

