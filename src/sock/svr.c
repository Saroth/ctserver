#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <config.h>

#define SOCK_LISTEN_QUEUE_LENGTH_MAX 5  //!< 链接请求队列长度
#define SOCK_EPOLL_SIZE     32          //!< epoll文件描述符数期望值，大于0即可
#define SOCK_EPOLL_EVENTS   32          //!< 每次响应的事件数量

typedef enum {                          //!< 服务器状态类型
    SOCK_SVR_STATE_STOP,                //!< 服务器停止
    SOCK_SVR_STATE_RUNNING,             //!< 服务器运行中
    SOCK_SVR_STATE_SUSPEND,             //!< 服务器挂起
}SOCK_SVR_STATE_E;
typedef struct {                        //!< 服务器句柄结构体
    int port;                           //!< 绑定端口
    int state;                          //!< 服务器状态, SOCK_SVR_STATE_E
    int fd_listen;                      //!< 监听套接口文件描述符
    int fd_epoll;                       //!< epoll文件描述符
}SOCK_SERVER_T;

/**
 * \brief       设置文件描述符为非阻塞状态
 * \param       fd          文件描述符
 * \return      0:Success   <0:Error
 */
static int set_nonblocking(int fd)
{
    int opts = 0;
    opts = fcntl(fd, F_GETFL);
    if(opts < 0) {
        dbg_outerr_E(DS_SERVER_ERR, "fcntl, F_GETFL");
        return SOCK_RET_FCNTL_GETFL;
    }
    opts |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, opts) < 0) {
        dbg_outerr_E(DS_SERVER_ERR, "fcntl, F_SETFL");
        return SOCK_RET_FCNTL_SETFL;
    }
    return 0;
}
/**
 * \brief       关闭链接
 * \param       p           链接信息结构体指针
 * \return      0:Success   <0:Error
 */
static int sock_close(void * p)
{
    SOCK_LINK_INFO_T * info = (SOCK_LINK_INFO_T *)p;
    SOCK_SERVER_T * svr = (SOCK_SERVER_T *)info->svr;
    if(info == NULL) {
        return 0;
    }
    if(g_processors[info->process_type]->close) {   //!< 释放数据处理任务
        g_processors[info->process_type]->close(info);
    }
    if(epoll_ctl(svr->fd_epoll, EPOLL_CTL_DEL, info->fd_peer, NULL)) {
        dbg_outerr_W(DS_SERVER_ERR, "epoll_ctl, DEL");
    }
    if(close(info->fd_peer)) {          //!< 关闭套接口
        dbg_outerr_W(DS_SERVER_ERR, "close");
    }
    free(info);
    info = NULL;
    return 0;
}
/** \brief       数据读取处理回调函数类型, SOCK_READ_T */
static int sock_read(void * p, void * buf, size_t count)
{
    SOCK_LINK_INFO_T * info = (SOCK_LINK_INFO_T *)p;
    SOCK_SERVER_T * svr = (SOCK_SERVER_T *)info->svr;
    if(info == NULL) {
        return 0;
    }
    int ret = 0;
    int n = 0;
    while(svr->state == SOCK_SVR_STATE_RUNNING
            && pool_thread_run(g_hdl_thread_pool)
            && ret < count
            && (n = read(info->fd_peer, buf + ret, count - ret)) > 0) {
        ret += n;
    }
    if(ret > 0) {
        info->lasttime = time(NULL);
        return ret;                     //!< 读到数据直接返回，不处理错误
    }
    if(n < 0 && errno != EAGAIN && errno != EINTR) {
        dbg_outerr_W(DS_SERVER_ERR, "read");
        ret = SOCK_RET_READ_ERR;        //!< 读错误
    }
    else if(n == 0) {
        ret = SOCK_RET_CLOSED;          //!< 对方已关闭链接 或 准备停止
    }
    info->state = SOCK_LINK_STATE_DISCONNECT;   //!< 准备关闭套接口
    return ret;
}
/** \brief       链接数据写入回调函数类型, SOCK_WRITE_T */
static int sock_write(void * p, const void * buf, size_t count)
{
    SOCK_LINK_INFO_T * info = (SOCK_LINK_INFO_T *)p;
    SOCK_SERVER_T * svr = (SOCK_SERVER_T *)info->svr;
    if(info == NULL) {
        return 0;
    }
    int ret = 0;
    int n = 0;
    while(svr->state == SOCK_SVR_STATE_RUNNING
            && pool_thread_run(g_hdl_thread_pool)
            && ret < count
            && (n = write(info->fd_peer, buf + ret, count - ret)) > 0) {
        ret += n;
    }
    if(ret == count) {
        info->lasttime = time(NULL);    //!< 写成功
        return ret;
    }
    else {
        dbg_outerr_E(DS_SERVER_ERR, "write");
        ret = SOCK_RET_WRITE_ERR;       //!< 写错误
        info->state = SOCK_LINK_STATE_DISCONNECT;   //!< 准备关闭套接口
    }
    return ret;
}
/** \brief       链接数据处理后执行回调函数类型, SOCK_POSTPROCESS_T */
static int sock_postprocess(void * p, SOCK_LINK_STATE_E state)
{
    SOCK_LINK_INFO_T * info = (SOCK_LINK_INFO_T *)p;
    if(info == NULL) {
        return 0;
    }
    //!< 链接交互错误或对方已关闭 || 数据处理器准备关闭链接
    if(info->state == SOCK_LINK_STATE_DISCONNECT
            || state == SOCK_LINK_STATE_DISCONNECT) {
        sock_close(info);
        return 0;
    }
    info->state = state;
    return 0;
}

/**
 * \brief       初始化服务端网络
 * \param       p           服务器参数结构体指针
 * \return      0:Success   <0:Error, SOCK_RET_E
 */
static int sock_server_init(SOCK_SERVER_T * p)
{
    int ret;
    if(p == NULL) {
        dbg_outerr_E(DS_SERVER_ERR, "Bad param!");
        return SOCK_RET_PARAM_ERR;
    }
    p->fd_epoll = -1;
    p->fd_listen = -1;
    do {
        //!< 申请套接口文件描述符，建立socket
        //!<    AF_INET: ARPA Internet protocols 即使用TCP/IP协议族
        //!<    SOCK_STREAM: 提供面向连接的稳定数据传输
        //!<    0: 由于该协议族中只有一个协议，因此参数3为0
        p->fd_listen = socket(AF_INET, SOCK_STREAM, 0);
        if(p->fd_listen < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "socket");
            ret = SOCK_RET_GET_SOCKETFD_ERR;
            break;
        }
        if((ret = set_nonblocking(p->fd_listen))) {   //!< 防止accept时阻塞
            break;
        }
        //!< 绑定套接口
        struct sockaddr_in addr_svr;
        addr_svr.sin_family = AF_INET;  //!< 使用TCP/IP协议
        addr_svr.sin_port = htons(p->port); //!< 设置端口
        addr_svr.sin_addr.s_addr = INADDR_ANY;  //!< 使用默认IP地址
        ret = bind(p->fd_listen, (struct sockaddr *)&addr_svr, sizeof(addr_svr));
        if(ret < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "bind");
            ret = SOCK_RET_BIND_ERR;
            break;
        }
        //!< 设置监听
        ret = listen(p->fd_listen, SOCK_LISTEN_QUEUE_LENGTH_MAX); //!< 设置监听
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
        struct epoll_event e;
        e.data.fd = p->fd_listen;       //!< 关联文件描述符
        e.events = EPOLLIN | EPOLLET;   //!< 设置事件类型:读入|边缘触发模式
        ret = epoll_ctl(p->fd_epoll, EPOLL_CTL_ADD, p->fd_listen, &e);
        if(ret < 0) {
            dbg_outerr_E(DS_SERVER_ERR, "epoll_ctl, ADD");
            ret = SOCK_RET_EPOLL_CTL_FD_SOCK_ERR;
            break;
        }
        return 0;
    } while(0);
    if(p->fd_listen >= 0) {
        close(p->fd_listen);
    }
    if(p->fd_epoll >= 0) {
        close(p->fd_epoll);
    }

    return ret;
}
/**
 * \brief       线程任务，接受多个新链接
 * \param       arg        服务器参数结构体指针的存储地址
 * \detail      监听端口必须设置为非阻塞，否则服务器无法终止，线程无法退出
 */
static void sock_accept(void * arg)
{
    SOCK_SERVER_T * p = *(SOCK_SERVER_T **)arg;
    struct epoll_event e;
    //!< 循环处理多个链接请求事件
    while(p->state == SOCK_SVR_STATE_RUNNING
            && pool_thread_run(g_hdl_thread_pool)) {
        int fd = accept(p->fd_listen, NULL, NULL);
        if(fd < 0) {
            if(errno == EAGAIN
                    || errno == ECONNABORTED
                    || errno == EPROTO
                    || errno  == EINTR) {
                break;              //!< 无新事件
            }
            dbg_outerr_W(DS_SERVER_ERR, "accept");
            continue;
        }
        if((set_nonblocking(fd))) { //!< 防止读写时阻塞
            dbg_out_W(DS_SERVER_ERR, "Set nonblocking failed!");
            close(fd);
            continue;
        }
        SOCK_LINK_INFO_T * info = (SOCK_LINK_INFO_T *)malloc(
                sizeof(SOCK_LINK_INFO_T));  //!< 申请链接信息存储结构体
        if(info == NULL) {
            dbg_outerr_W(DS_SERVER_ERR, "malloc");
            close(fd);
            continue;
        }
        memset(info, 0x00, sizeof(SOCK_LINK_INFO_T));
        info->fd_peer = fd;
        info->state = SOCK_LINK_STATE_IDLE;
        info->lasttime = time(NULL);
        info->process_type = g_processor_type_cur;
        info->process_param = 0;
        info->svr = (long)p;
        info->read = sock_read;
        info->write = sock_write;
        info->postproc = sock_postprocess;
        e.data.ptr = info;          //!< 添加端口事件监听
        e.events = EPOLLIN | EPOLLET;
        int ret = epoll_ctl(p->fd_epoll, EPOLL_CTL_ADD, fd, &e);
        if(ret == -1) {
            dbg_outerr_W(DS_SERVER_ERR, "epoll_ctl, ADD");
            free(info);
            close(fd);
            continue;
        }
    }
}
/**
 * \brief       线程任务，服务端流程
 * \param       p           服务器参数结构体指针
 * \detail      将长期占用一个线程，直到服务器被停止或线程池被销毁
 */
static void sock_server_routine(void * arg)
{
    SOCK_SERVER_T * p = *(SOCK_SERVER_T **)arg;
    struct epoll_event events[SOCK_EPOLL_EVENTS];
    while(p->state != SOCK_SVR_STATE_STOP
            && pool_thread_run(g_hdl_thread_pool)) {
        pthread_testcancel();           //!< 线程取消点
        if(p->state == SOCK_SVR_STATE_SUSPEND) {
            sleep(1);                   //!< 服务器挂起
        }
        int i;
        int fd_req = epoll_wait(p->fd_epoll, events, SOCK_EPOLL_EVENTS, 500);
        for(i = 0; i < fd_req; i++) {   //!< 处理事件
            pthread_testcancel();       //!< 线程取消点
            if(events[i].data.fd == p->fd_listen) { //!< 新链接事件
                //!< 添加监听任务到线程池，创建监听线程
                pool_thread_task_add(g_hdl_thread_pool, "Server accepter",
                        p, sizeof(SOCK_SERVER_T *), sock_accept);
                continue;
            }
            if(events[i].events & EPOLLIN) {    //!< 其他套接口的读取事件
                SOCK_LINK_INFO_T * info
                    = (SOCK_LINK_INFO_T *)events[i].data.ptr;
                if(info->state == SOCK_LINK_STATE_BUSY) {
                    //!< 其他线程正在操作此链接时收到新事件，为了确保第二次
                    //!<    循环时能继续处理数据，需要改为水平触发模式
                    if(events[i].events & EPOLLET) {
                        events[i].events = EPOLLIN;
                        if(epoll_ctl(p->fd_epoll, EPOLL_CTL_MOD,
                                    info->fd_peer, &events[i])) {
                            dbg_outerr_W(DS_SERVER_ERR, "epoll_ctl, MOD");
                        }
                    }
                    continue;
                }
                if(!(events[i].events & EPOLLET)) {
                    events[i].events = EPOLLIN | EPOLLET;   //!< 恢复为边缘触发
                    if(epoll_ctl(p->fd_epoll, EPOLL_CTL_MOD,
                                info->fd_peer, &events[i])) {
                        dbg_outerr_W(DS_SERVER_ERR, "epoll_ctl, MOD");
                    }
                }
                //!< 添加数据处理任务到线程池
                if(g_processors[info->process_type]->process) {
                    char taskname[32] = { 0 };
                    sprintf(taskname, "link:");
                    if(pool_thread_task_add(g_hdl_thread_pool, taskname,
                                &info, sizeof(SOCK_LINK_INFO_T *),
                                g_processors[info->process_type]->process)) {
                        dbg_out_W(DS_SERVER_ERR, "Add task failed!");
                    }
                    else {
                        info->state = SOCK_LINK_STATE_BUSY;
                    }
                }
                continue;
            }
            if((events[i].events & EPOLLERR)
                    || (events[i].events & EPOLLHUP)
                    || (events[i].events & EPOLLRDHUP)
                    || (!events[i].events & EPOLLIN)) { //!< 异常链接，关闭
                dbg_out_W(DS_SERVER_ERR, "Link error, close.");
                sock_close(events[i].data.ptr);
                continue;
            }
        }
    }
    //!< 退出操作
}

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
    //!< 添加服务器运行任务到线程池
    pool_thread_task_add(g_hdl_thread_pool, "Server",
            p, sizeof(SOCK_SERVER_T *), sock_server_routine);

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


