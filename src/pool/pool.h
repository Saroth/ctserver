#ifndef __POOL_H__
#define __POOL_H__

/**
 * \block:      Thread pool
 * @{ */
#define THREAD_NUM_MIN  4               //!< 最少保留线程数
#define THREAD_NUM_MAX  32              //!< 线程数上限
#define THREAD_IDLE_MAX 4               //!< 最大空闲线程数
#define THREAD_INTERVAL_CREATE  2       //!< 线程创建间隔(s)
#define THREAD_INTERVAL_DESTROY 16      //!< 线程销毁间隔(s)
#define THREAD_INTERVAL_COND_SIGNAL 4   //!< 任务信号发送间隔(s)
#define THREAD_QUIT_WAITTIME_MAX    4   //!< 线程池销毁时等待各线程自动退出的时间

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
typedef void (* TASK_FUNC_T)(void *); //!< 任务接口函数指针
/**
 * \brief       添加任务到列表
 * \param       hdl         线程池句柄
 * \param       name        任务名
 * \param       arg         任务参数指针
 * \param       func        任务函数指针
 * \return      0:Success   <0:Error, POOL_THREAD_RET_E
 */
int pool_thread_task_add(long hdl, char * name, void * arg, TASK_FUNC_T func);
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
/** @} */


#endif /* __POOL_H__ */

