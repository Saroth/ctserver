#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include "../sock/sock.h"

typedef enum {                          //!< 返回值定义
    PROC_RET = ERR_CODE_PROCESSOR,      //!< 起始码
}PROC_RET_E;
typedef struct {                        //!< 数据处理方法结构体
    char * name;                        //!< 处理方法名
    /**
     * \brief       数据交互处理函数类型
     * \param       p           链接信息, 参考: SOCK_LINK_INFO_T
     * \return      0:Success   <0:PROC_RET_E
     * \detail      函数需调用结构体SOCK_LINK_INFO_T的read接口读取数据，
     *                  直到读取返回0;
     *              函数需调用结构体SOCK_LINK_INFO_T的write接口写入数据;
     *              函数执行完成后，需调用结构体SOCK_LINK_INFO_T的postproc
     *                  接口处理端口相关任务;
     *              函数不应在内部等待读取数据，只能处理由epoll触发的第一次
     *                  read事件，读取完成时应立即退出;
     *              函数需保存符合协议的不完整包数据，用于下次触发
     *                  读取时进行组合处理;
     *              函数在读取或写入遇到返回错误时，应立即执行postproc并退出;
     */
    TASK_FUNC_T process;                //!< 数据交互处理函数指针
    /**
     * \brief       关闭链接前资源释放处理函数类型
     * \param       p           链接信息, 参考: SOCK_LINK_INFO_T
     * \return      0:Success   <0:PROC_RET_E
     * \detail      该函数仅由sock模块调用;
     *              用于非正常关闭链接前，释放在PROC_FUNC_INTERACTION_T中
     *                  申请的资源;
     */
    TASK_FUNC_T close;                  //!< 关闭链接前资源释放处理函数指针
}PROC_T;
typedef enum {                          //!< 数据处理方法类型
    PROC_METHOD_NULL = 0,               //!< 
    PROC_METHOD_ZERO,
    PROC_METHOD_RANDOM,
    PROC_METHOD_ECHO,
    // PROC_METHOD_SSL_ZERO,
    // PROC_METHOD_SSL_RANDOM,
    // PROC_METHOD_SSL_ECHO,
    PROC_METHOD_TYPES_MAX,
}PROC_METHOD_TYPES_E;
extern PROC_T *g_processors[PROC_METHOD_TYPES_MAX]; //!< 数据处理方法列表
extern int g_processor_type_cur;        //!< 当前使用的数据处理器类型

#endif /* __PROCESSOR_H__ */

