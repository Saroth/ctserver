#ifndef __POOL_H__
#define __POOL_H__

/**
 * \block:      Thread pool
 * @{ */
#include <pthread.h>

#define THREAD_NUM_MIN  4               //!< 最少保留线程数
#define THREAD_NUM_MAX  32              //!< 线程数上限
#define THREAD_IDLE_MAX 4               //!< 最大空闲线程数
#define THREAD_INTERVAL_CREATE  2       //!< 线程创建间隔(s)
#define THREAD_INTERVAL_DESTROY 16      //!< 线程销毁间隔(s)
#define THREAD_INTERVAL_COND_SIGNAL 4   //!< 任务信号发送间隔(s)
#define THREAD_QUIT_WAITTIME_MAX    8   //!< 线程池销毁时等待各线程自动退出的时间

typedef enum {                          //!< 返回值定义
    POOL_THREAD_RET = ERR_CODE_POOL_THREAD, //!< 起始码
    POOL_THREAD_RET_MEM,                //!< 内存申请错误
    POOL_THREAD_RET_THREAD_CREATE,      //!< 创建线程失败
    POOL_THREAD_RET_MUTEX_INIT,         //!< 初始化互斥锁失败
    POOL_THREAD_RET_MUTEX_LOCK,         //!< 互斥锁上锁失败
    POOL_THREAD_RET_COND_INIT,          //!< 初始化条件变量失败
    POOL_THREAD_RET_NO_THREAD_CANCELED, //!< 没有可销毁的线程
    POOL_THREAD_RET_PARAM_ERR,          //!< 参数错误
}POOL_THREAD_RET_E;
/**
 * \brief       任务接口函数类型
 * \param       p       任务参数，通常是存储某个结构体的指针的地址
 */
typedef void (* TASK_FUNC_T)(void * p);
typedef struct {                        //!< 任务结构体
    char name[32];                      //!< 任务名
    TASK_FUNC_T func;                   //!< 任务接口函数指针
    struct list_head ptr;               //!< 列表节点
    int arg_len;                        //!< 参数数据长度
    char arg[0];                        //!< 任务接口参数数据(不定长)
}TASK_T;
typedef enum {                          //!< 线程状态
    THREAD_STATE_IDLE,                  //!< idle
    THREAD_STATE_BUSY,                  //!< busy
}THREAD_STATE_E;
typedef struct {                        //!< 线程结构体
    TASK_T * task;                      //!< 任务
    int state;                          //!< 线程状态, THREAD_STATE_E
    pthread_t tid;                      //!< 线程ID;
    pthread_mutex_t mutex;              //!< 互斥锁
    struct list_head ptr;               //!< 列表节点
}THREAD_T;
typedef struct {                        //!< 线程池结构体
    pthread_mutex_t mutex;              //!< 互斥锁
    pthread_cond_t cond;                //!< 条件锁
    struct list_head list_task;         //!< 任务列表
    int list_task_num;                  //!< 任务列表任务数
    struct list_head list_thread;       //!< 线程结构体指针数组
    int thread_num;                     //!< 当前线程数
    int idle_thread_num;                //!< 当前空闲线程数
    int flag_halt;                      //!< 销毁标记, 0:Disable; 1:Enable
}POOL_THREAD_T;
/**
 * \brief       添加任务到列表
 * \param       hdl         线程池句柄
 * \param       name        任务名
 * \param       arg         任务参数数据指针
 * \param       arg_len     任务参数数据长度
 * \param       func        任务函数指针
 * \return      0:Success   <0:Error, POOL_THREAD_RET_E
 */
int pool_thread_task_add(long hdl, char * name, void * arg, int arg_len,
        TASK_FUNC_T func);
/**
 * \brief       销毁线程池
 * \param       hdl         线程池句柄指针
 * \return      0:Success   <0:Error, POOL_THREAD_RET_E
 */
int pool_thread_del(long * hdl);
/**
 * \brief       创建线程池
 * \param       hdl         线程池句柄指针
 * \return      0:Success   <0:Error, POOL_THREAD_RET_E
 */
int pool_thread_new(long * hdl);
/**
 * \brief       查看线程池运行状态
 * \param       hdl         线程池句柄
 * \return      false:Stoped    true:Running
 */
int pool_thread_run(long hdl);
/** @} */


#endif /* __POOL_H__ */

