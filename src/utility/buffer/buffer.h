#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <config.h>

#define BUF_TMPFILE_PREFIX ".tmpbq_"
typedef enum {
    BUF_RET = ERR_CODE_BUFFER,
    BUF_RET_MEM,                        //!< 内存申请错误
    BUF_RET_SHM,                        //!< 共享内存申请失败
    BUF_RET_SEM,                        //!< 信号量申请失败
    BUF_RET_PARAM_ERR,                  //!< 参数错误
    BUF_RET_TIMEOUT,                    //!< 等待超时
}BUF_RET_E;
/**
 * \brief       初始化数据缓存
 * \param       name        缓存模式, BUF_MODE_E
 * \param       size        缓存大小
 * \param       hdl         句柄指针地址
 * \return      >0:缓存结构体指针   <=0:Error code:BUFFER_RET_E
 */
int buf_queue_new(char * name, unsigned long size, long * hdl);
/**
 * \brief       删除数据缓存
 * \param       hdl         句柄指针地址
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
int buf_queue_del(long * hdl);
/**
 * \brief       添加数据到缓存队列
 * \param       buf         数据缓存
 * \param       length      长度
 * \param       waittime    等待其他线程读取后留出空间的时间
 * \param       hdl         句柄
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
int buf_queue_append(char * buf, unsigned int length, int waittime, long hdl);
/**
 * \brief       读取数据
 * \param       buf         数据缓存
 * \param       length      长度
 * \param       hdl         句柄
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
int buf_queue_read(char * buf, unsigned int length, long hdl);
/**
 * \brief       获取未读取的数据长度
 * \param       hdl         句柄
 * \return      0:Success   <0:Error code:BUFFER_RET_E
 */
unsigned int buf_queue_size(long hdl);

#endif /* __BUFFER_H__ */

