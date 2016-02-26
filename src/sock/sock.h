#ifndef __SOCK_H__
#define __SOCK_H__

#define SOCK_AUTOCLOSE_IDLELINK_TIME_S  60  //!< 自动关闭空闲链接时间(秒)
typedef enum {                          //!< 返回值定义
    SOCK_RET = ERR_CODE_SOCK,           //!< 起始码
    SOCK_RET_PARAM_ERR,                 //!< 参数错误
    SOCK_RET_MEM,                       //!< 内存申请错误
    SOCK_RET_GET_SOCKETFD_ERR,          //!< 获取套接口描述符失败
    SOCK_RET_BIND_ERR,                  //!< 监听失败
    SOCK_RET_LISTEN_ERR,                //!< 监听失败
    SOCK_RET_GET_EPOLLFD_ERR,           //!< 获取epoll描述符失败
    SOCK_RET_EPOLL_CTL_FD_SOCK_ERR,     //!< 套接口描述符设置监听失败
    SOCK_RET_FCNTL_GETFL,               //!< 获取文件状态标记错误
    SOCK_RET_FCNTL_SETFL,               //!< 设置文件状态标记错误

    SOCK_RET_CLOSED,                    //!< 链接已被关闭
    SOCK_RET_READ_ERR,                  //!< 读错误
    SOCK_RET_WRITE_ERR,                 //!< 写错误
}SOCK_RET_E;
typedef enum {                          //!< 连接状态类型
    SOCK_LINK_STATE_IDLE,               //!< 空闲
    SOCK_LINK_STATE_BUSY,               //!< 数据交互处理中
    SOCK_LINK_STATE_DISCONNECT,         //!< 链接异常或被关闭
}SOCK_LINK_STATE_E;
/**
 * \brief       链接数据读取回调函数类型
 * \param       p           链接信息结构体指针
 * \param       buf         数据缓存
 * \param       count       期望读取的数据长度
 * \return      >0:Read length; 0:No data; <0:Error, SOCK_RET_E
 * \detail      count可能比实际可读取量大，所以读完数据立即返回
 *              读取错误则关闭链接
 */
typedef int (*SOCK_READ_T)(void * p, void * buf, size_t count);
/**
 * \brief       链接数据写入回调函数类型
 * \param       p           链接信息结构体指针
 * \param       buf         数据缓存
 * \param       count       期望写入的数据长度
 * \return      >=0:Write length; <0:Error, SOCK_RET_T
 * \detail      只有在count长度的数据都发送完成才成功，否则报错并关闭链接
 */
typedef int (*SOCK_WRITE_T)(void * p, const void * buf, size_t count);
/**
 * \brief       链接数据处理后执行回调函数类型
 * \param       p           链接信息结构体指针
 * \param       state       设置退出后链接的状态, SOCK_LINK_STATE_E
 * \return      0:Success   <0:Error
 */
typedef int (*SOCK_POSTPROCESS_T)(void * p, SOCK_LINK_STATE_E state);
typedef struct {                        //!< 链接信息结构体
    int fd_peer;                        //!< 通道端口文件描述符
    int state;                          //!< 链接状态标记, SOCK_LINK_STATE_E
    int lasttime;                       //!< 最后一次正常交互的时间
    int process_type;                   //!< 数据处理类型
    long process_param;                 //!< 数据处理参数结构体指针，仅用于给
                                        //!<    processor存储临时参数
    long svr;                           //!< 服务器参数结构体指针
    SOCK_READ_T read;                   //!< 数据读取回调函数
    SOCK_WRITE_T write;                 //!< 数据写入回调函数
    SOCK_POSTPROCESS_T postproc;        //!< 数据后处理
}SOCK_LINK_INFO_T;

int sock_svr_start(int port, long * hdl);
int sock_svr_stop(long * hdl);

int sock_cli_start(long ip, int port);
int sock_cli_stop(long * hdl);
int sock_cli_send(long * hdl);

#endif /* __SOCK_H__ */

