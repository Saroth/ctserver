#ifndef __SOCK_H__
#define __SOCK_H__

typedef enum {                          //!< 返回值定义
    SOCK_RET = ERR_CODE_SOCK,           //!< 起始码
    SOCK_RET_PARAM_ERR,                 //!< 参数错误
    SOCK_RET_MEM,                       //!< 内存申请错误
    SOCK_RET_GET_SOCKETFD_ERR,          //!< 获取套接口描述符失败
    SOCK_RET_BIND_ERR,                  //!< 监听失败
    SOCK_RET_LISTEN_ERR,                //!< 监听失败
    SOCK_RET_GET_EPOLLFD_ERR,           //!< 获取epoll描述符失败
    SOCK_RET_EPOLL_CTL_ERR,             //!< 设置epoll失败
}SOCK_RET_E;
typedef struct {                        //!< 服务程序参数结构体
    unsigned short int port;            //!< 端口号
}SOCK_SVR_PARAM_T;

int sock_svr_start(int port, long * hdl);
int sock_svr_stop(long * hdl);

int sock_cli_start(long ip, int port);
int sock_cli_stop(long * hdl);
int sock_cli_send(long * hdl);

#endif /* __SOCK_H__ */

