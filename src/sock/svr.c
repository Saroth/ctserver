#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <config.h>

#define SOCK_LISTEN_QUEUE_LENGTH_MAX 5  //!< 链接请求队列长度
#define SOCK_EPOLL_SIZE     32          //!< epoll文件描述符数期望值，大于0即可
#define SOCK_EPOLL_EVENTS   32          //!< 每次响应的事件数量

typedef struct {
    int port;                           //!< 绑定端口
    int fd_sock;                        //!< 套接口文件描述符
    int fd_epoll;                       //!< epoll文件描述符
}SOCK_SERVER_T;

/**
 * \brief       初始化服务端网络
 * \param       port        准备监听的端口号
 * \return      0:Success   <0:Error, SOCK_RET_E
 */
static int sock_server_init(SOCK_SERVER_T * p)
{
    int ret;
    p->fd_epoll = -1;
    p->fd_sock = -1;
    do {
        //!< 申请套接口文件描述符，建立socket
        //!<    AF_INET: ARPA Internet protocols 即使用TCP/IP协议族
        //!<    SOCK_STREAM: 提供面向连接的稳定数据传输
        //!<    0: 由于该协议族中只有一个协议，因此参数3为0
        p->fd_sock = socket(AF_INET, SOCK_STREAM, 0);
        if(p->fd_sock < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "socket");
            ret = SOCK_RET_GET_SOCKETFD_ERR;
            break;
        }
        //!< 绑定套接口
        struct sockaddr_in addr_svr;
        addr_svr.sin_family = AF_INET;      //!< 使用TCP/IP协议
        addr_svr.sin_port = htons(p->port); //!< 设置端口
        addr_svr.sin_addr.s_addr = INADDR_ANY;  //!< 使用默认IP地址
        ret = bind(p->fd_sock, (struct sockaddr *)&addr_svr, sizeof(addr_svr));
        if(ret < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "bind");
            ret = SOCK_RET_BIND_ERR;
            break;
        }
        //!< 设置监听
        ret = listen(p->fd_sock, SOCK_LISTEN_QUEUE_LENGTH_MAX); //!< 设置监听
        if(ret < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "listen");
            ret = SOCK_RET_LISTEN_ERR;
            break;
        }
        //!< 申请epoll文件描述符
        p->fd_epoll = epoll_create(SOCK_EPOLL_SIZE);
        if(p->fd_epoll < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "epoll_create");
            ret = SOCK_RET_GET_EPOLLFD_ERR;
            break;
        }
        //!< 注册epoll事件
        struct epoll_event event;
        event.data.fd = p->fd_sock;            //!< 关联文件描述符
        event.events = EPOLLIN | EPOLLET;   //!< 设置事件类型
        ret = epoll_ctl(p->fd_epoll, EPOLL_CTL_ADD, p->fd_sock, &event);
        if(ret < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "epoll_ctl");
            ret = SOCK_RET_EPOLL_CTL_ERR;
            break;
        }
        return 0;
    } while(0);
    if(p->fd_sock >= 0) {
        close(p->fd_sock);
    }
    if(p->fd_epoll >= 0) {
        close(p->fd_epoll);
    }

    return ret;
}

// static int sock_server_routine(SOCK_SERVER_T * p)
// {
//     int fd_req;
//     struct epoll_event event;
//     struct epoll_event events[SOCK_EPOLL_EVENTS];
//     while(1) {
//         fd_req = epoll_wait();
//     }
//     return 0;
// }

int sock_svr_start(int port, long * hdl)
{
    int ret;
    SOCK_SERVER_T * p = (SOCK_SERVER_T *)malloc(sizeof(SOCK_SERVER_T));
    if(p == NULL) {
        dbg_outerr_E(DS_SERVER_ERR, "malloc");
        return SOCK_RET_MEM;
    }
    memset(p, 0x00, sizeof(SOCK_SERVER_T));
    p->port = port;
    if((ret = sock_server_init(p))) {
        dbg_out_E(DS_SERVER_ERR, "Socket init failed!");
        return ret;
    }
    *hdl = (long)p;

    return 0;
}

int sock_svr_stop(long * hdl)
{
    if(hdl) {
        free(hdl);
        hdl = NULL;
    }
    return 0;
}


