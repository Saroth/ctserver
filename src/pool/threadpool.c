#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <config.h>

typedef struct {                        //!< 任务结构体
    char * name;                        //!< 任务名
    void * arg;                         //!< 任务接口参数
    TASK_FUNC_T func;                   //!< 任务接口函数指针
    struct list_head ptr;               //!< 列表节点
}TASK_T;
typedef enum {                          //!< 线程状态
    IDLE,                               //!< idle
    BUSY,                               //!< BUSY
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
    int thread_num;            //!< 当前线程数
    int idle_thread_num;       //!< 当前空闲线程数
    int flag_halt;             //!< 销毁标记, 0:Disable; 1:Enable
}POOL_THREAD_T;
typedef struct {                        //!< 线程传入参数结构体
    POOL_THREAD_T * p;                  //!< 线程池结构体指针
    THREAD_T * t;                       //!< 线程结构体指针数组下标
}THREAD_ARG_T;

/**
 * \brief       退出线程时清理线程池资源
 * \param       arg         线程池结构体指针
 */
static void pool_cleanup(void * arg)
{
    POOL_THREAD_T * p = (POOL_THREAD_T *)arg;
    //!< 在pthread_cond_wait等待中撤销线程会导致死锁，此处必须要重新解锁
    if(pthread_mutex_unlock(&p->mutex)) {
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
    }
}
/**
 * \brief       退出线程时清理线程资源
 * \param       arg         线程结构体指针
 */
static void thread_cleanup(void * arg)
{
    THREAD_T * t = (THREAD_T *)arg;
    //!< 在任务中撤销线程时，锁未释放，销毁锁时会警告
    if(pthread_mutex_unlock(&t->mutex)) {
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
    }
}
/**
 * \brief       线程任务处理流程
 * \param       arg         线程传入参数结构体指针, THREAD_ARG_T
 * \return      ~:Success
 * \detail      线程池各线程常规运行函数
 *              有任务时执行任务函数
 *              无任务时休眠等待
 */
static void * routine(void * arg)
{
    POOL_THREAD_T * p = ((THREAD_ARG_T *)arg)->p;
    THREAD_T * t = ((THREAD_ARG_T *)arg)->t;
    pthread_t tid = t->tid;
    free(arg);                          //!< 释放传入的结构体指针
    arg = NULL;
    while(!(p->flag_halt)) {
        if(pthread_mutex_lock(&p->mutex)) { //!< 线程池结构体访问上锁
            dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_lock");
            break;
        }
        p->idle_thread_num++;
        while(list_empty(&p->list_task) && !(p->flag_halt)) {
            pthread_cleanup_push(pool_cleanup, p);
            if(pthread_cond_wait(&p->cond, &p->mutex)) {    //!< 解锁并等待条件变量信号
                dbg_outerr_W(DS_POOL_ERR, "pthread_cond_wait");
            }
            pthread_cleanup_pop(0);
        }
        if(p->flag_halt) {
            if(pthread_mutex_unlock(&p->mutex)) {   //!< 退出前先解锁
                dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
            }
            break;                      //!< 线程池准备销毁
        }
        if(p->list_task_num > 0) {
            p->list_task_num--;
        }
        if(pthread_mutex_lock(&t->mutex)) { //!< 线程结构体访问上锁
            dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_lock");
        }
        t->task = list_first_entry(&p->list_task, TASK_T, ptr);    //!< 取任务
        list_del(&t->task->ptr);        //!< 从任务列表中删除取出的任务
        dbg_out_I(DS_POOL, "Thread %#x get task: %s(%#x)",
                tid, t->task->name, t->task);
        p->idle_thread_num--;
        if(pthread_mutex_unlock(&p->mutex)) {   //!< 线程池结构体访问解锁
            dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
        }
        /// 执行任务
        t->state = BUSY;                //!< 线程设为忙状态
        if(t->task->func) {
            pthread_cleanup_push(thread_cleanup, t);
            t->task->func(t->task->arg);
            pthread_cleanup_pop(0);
        }
        if(t->task) {
            free(t->task);
            t->task = NULL;
        }
        t->state = IDLE;                //!< 线程设为空闲状态
        if(pthread_mutex_unlock(&t->mutex)) {   //!< 线程结构体访问解锁
            dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
        }
    }
    p->thread_num--;
    if(t->state == IDLE) {
        p->idle_thread_num--;
    }
    list_del(&t->ptr);          //!< 从线程池列表中删除线程结构体
    if(pthread_mutex_destroy(&t->mutex)) {  //!< 销毁互斥锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_destroy");
    }
    free(t);
    t = NULL;
    dbg_out_I(DS_POOL, "Thread %#x exit", tid);
    pthread_exit(NULL);
}
/**
 * \brief       添加新线程到池
 * \param       p           线程池句柄指针
 * \return      0:Success   <0:Err, POOL_THREAD_RET_E
 */
static int thread_add(POOL_THREAD_T * p)
{
    THREAD_T * t = (THREAD_T *)malloc(sizeof(THREAD_T));
    if(t == NULL) {
        dbg_outerr_E(DS_POOL_ERR, "malloc");
        return POOL_THREAD_RET_MEM;
    }
    memset(t, 0x00, sizeof(THREAD_T));
    t->state = IDLE;
    THREAD_ARG_T * arg = (THREAD_ARG_T *)malloc(sizeof(THREAD_ARG_T));
    if(arg == NULL) {
        dbg_outerr_E(DS_POOL_ERR, "malloc");
        return POOL_THREAD_RET_MEM;
    }
    memset(arg, 0x00, sizeof(THREAD_ARG_T));
    arg->t = t;
    arg->p = p;
    if(pthread_mutex_init(&t->mutex, NULL)) {   //!< 初始化互斥锁
        dbg_outerr_E(DS_POOL_ERR, "pthread_mutex_init");
        free(t);
        t = NULL;
        return POOL_THREAD_RET_MUTEX_INIT;
    }
    if(pthread_create(&t->tid, NULL, routine, arg)) {  //!< 创建新线程
        dbg_outerr_E(DS_POOL_ERR, "pthread_create");
        if(pthread_mutex_destroy(&t->mutex)) {  //!< 线程创建失败，销毁互斥锁
            dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_destroy");
        }
        free(t);
        t = NULL;
        return POOL_THREAD_RET_THREAD_CREATE;
    }
    dbg_out_I(DS_POOL, "Create thread: %#x", t->tid);
    if(pthread_mutex_lock(&p->mutex)) { //!< 线程池结构体访问上锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_lock");
    }
    list_add_tail(&t->ptr, &p->list_thread);    //!< 添加新节点到链表末尾
    p->thread_num++;
    if(pthread_mutex_unlock(&p->mutex)) {;  //!< 线程池结构体访问解锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
    }

    return 0;
}
/**
 * \brief       从池中删除一个空闲的线程
 * \param       p           线程池句柄指针
 * \param       force       0:删除一个空闲线程  !0:强制删除第一个线程
 * \return      0:Success   <0:Err, POOL_THREAD_RET_E
 */
static int thread_del(POOL_THREAD_T * p, int force)
{
    struct list_head * ptr = NULL;
    THREAD_T * t = NULL;
    if(force) {
        t = list_first_entry(&p->list_thread, THREAD_T, ptr);
    }
    else {
        list_for_each(ptr, &p->list_thread) {   //!< 遍历所有节点
            t = list_entry(ptr, THREAD_T, ptr);
            if(t->state == IDLE) {
                break;
            }
        }
        if(t->state != IDLE) {
            dbg_out_W(DS_POOL_ERR, "Cancel thread: nothing to do");
            return POOL_THREAD_RET_NO_THREAD_CANCELED;
        }
    }
    if(pthread_cancel(t->tid)) {        //!< 撤销线程
        dbg_outerr_W(DS_POOL_ERR, "pthread_cancel");
    }
    void * ret;
    if(pthread_join(t->tid, &ret)) {    //!< 等待线程退出
        dbg_outerr_W(DS_POOL_ERR, "pthread_join");
    }
    if(ret != PTHREAD_CANCELED) {
        dbg_out_W(DS_POOL_ERR,
                "Thread wasn't canceled (shouldn't happend!)");
    }
    list_del(&t->ptr);                  //!< 从线程池列表中删除线程结构体
    p->thread_num--;
    if(t->state == IDLE) {
        p->idle_thread_num--;
    }
    if(t->task) {
        free(t->task);                  //!< 释放正在运行的任务结构体
        t->task = NULL;
    }
    if(pthread_mutex_destroy(&t->mutex)) {  //!< 销毁互斥锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_destroy");
    }
    dbg_out_I(DS_POOL, "Cancel thread: %#x", t->tid);
    free(t);
    t = NULL;
    return 0;
}
/**
 * \brief       线程池管理器
 * \param       arg         线程池句柄
 * \detail      初始化时，函数本身作为线程池的一个任务，被添加到任务列表；
 *              将长期占用一个线程，直到线程池被销毁；
 *              线程池管理策略:
 *                  1. 任务列表有任务且无空闲线程时，创建新线程；
 *                  2. 任务列表无任务且有过多空闲线程时，销毁部分空闲线程；
 */
static void manager(void * arg)
{
    POOL_THREAD_T * p = (POOL_THREAD_T *)arg;
    int interval_create = 0;
    int interval_destroy = 0;
    int interval_cond = 0;
    dbg_out_I(DS_POOL, "Thread pool manager start");
    while(p->flag_halt == 0) {
        pthread_testcancel();           //!< 线程取消点
        while(interval_create == 0) {
            if(p->thread_num < THREAD_NUM_MAX) {    //!< 线程数少于上限
                if(p->thread_num < THREAD_NUM_MIN   //!< 线程数少于保留量 或
                        || (!list_empty(&p->list_task)  //!< 任务列表有任务 且
                            && p->idle_thread_num <= 0)) {  //!< 空闲线程为0
                    interval_create = THREAD_INTERVAL_CREATE;   //!< 重置间隔
                    if(thread_add(p)) {             //!< 添加新线程
                        dbg_out_W(DS_POOL_ERR, "Create thread failed!");
                        break;
                    }
                    continue;
                }
            }
            break;
        }
        while(interval_destroy <= 0) {
            if(p->thread_num > THREAD_NUM_MIN) {    //!< 线程数大于下限
                if(p->thread_num > THREAD_NUM_MAX   //!< 线程数大于上限 或
                        || (list_empty(&p->list_task)   //!< 无任务 且
                            && p->idle_thread_num > THREAD_IDLE_MAX)
                        ) {             //!< 空闲线程数超出限制
                    interval_destroy = THREAD_INTERVAL_DESTROY; //!< 重置间隔
                    if(thread_del(p, 0)) {  //!< 移除空闲线程，如果存在
                        dbg_out_W(DS_POOL_ERR, "No thread canceled!");
                        break;
                    }
                    continue;
                }
            }
            break;
        }
        if(interval_cond <= 0
                && !list_empty(&p->list_task)   //!< 有任务 且
                && p->idle_thread_num > 0) {    //!< 有空闲线程
            if(pthread_cond_signal(&p->cond)) { //!< 发送信号
                dbg_outerr_W(DS_POOL_ERR, "pthread_cond_signal");
            }
            interval_cond = THREAD_INTERVAL_COND_SIGNAL;
        }
        if(interval_create > 0) {
            interval_create--;
        }
        if(interval_destroy > 0) {
            interval_destroy--;
        }
        if(interval_cond > 0) {
            interval_cond--;
        }
        sleep(1);
    }
    dbg_out_I(DS_POOL, "Thread pool manager stop");
}
/** \brief       添加任务到列表 */
int pool_thread_task_add(long hdl, char * name, void * arg, TASK_FUNC_T func)
{
    POOL_THREAD_T * p = (POOL_THREAD_T *)hdl;
    if(p == NULL) {
        return POOL_THREAD_RET_PARAM_ERR;
    }
    if(p->flag_halt) {
        return 0;                       //!< 准备销毁，不再添加任务
    }
    TASK_T * t = (TASK_T *)malloc(sizeof(TASK_T));
    if(t == NULL) {
        dbg_outerr_E(DS_POOL_ERR, "malloc");
        return POOL_THREAD_RET_MEM;
    }
    memset(t, 0x00, sizeof(TASK_T));
    t->name = name;
    t->arg = arg;
    t->func = func;
    if(pthread_mutex_lock(&p->mutex)) { //!< 线程池结构体访问上锁
        free(t);
        dbg_outerr_E(DS_POOL_ERR, "pthread_mutex_lock");
        return POOL_THREAD_RET_MUTEX_LOCK;
    }
    dbg_out_I(DS_POOL, "Append task: %s(%#x)", name, t);
    list_add_tail(&t->ptr, &p->list_task);  //!< 添加任务到列表末尾
    p->list_task_num++;
    if(pthread_cond_signal(&p->cond)) { //!< 发送信号
        dbg_outerr_W(DS_POOL_ERR, "pthread_cond_signal");
    }
    if(pthread_mutex_unlock(&p->mutex)) {   //!< 线程池结构体访问解锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
    }
    return 0;
}
/** \brief       销毁线程池 */
int pool_thread_del(long * hdl)
{
    POOL_THREAD_T * p = (POOL_THREAD_T *)*hdl;
    if(p == NULL) {
        return POOL_THREAD_RET_PARAM_ERR;
    }

    dbg_out_I(DS_POOL, "Send halt signal...");
    if(pthread_mutex_lock(&p->mutex)) { //!< 线程池结构体访问上锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_lock");
    }
    p->flag_halt = 1;                   //!< 设置终止标记
    while(!list_empty(&p->list_task)) { //!< 清空任务列表
        TASK_T * t = list_first_entry(&p->list_task, TASK_T, ptr);
        if(t) {
            list_del(&t->ptr);
            free(t);
            t = NULL;
        }
    }
    p->list_task_num = 0;
    if(pthread_cond_broadcast(&p->cond)) {  //!< 广播通告所有线程自动退出
        dbg_outerr_W(DS_POOL_ERR, "pthread_cond_broadcast");
    }
    if(pthread_mutex_unlock(&p->mutex)) {   //!< 线程池结构体访问解锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_unlock");
    }
    dbg_out_I(DS_POOL, "Waiting threads quit...");
    int i = THREAD_QUIT_WAITTIME_MAX;
    while(i-- > 0 && p->idle_thread_num > 0) {
        sleep(1);                       //!< 等待短任务线程和空闲线程自动退出
    }
    while(!list_empty(&p->list_thread)) {
        thread_del(p, 1);               //!< 强制清空线程列表
    }

    int ret;
    if((ret = pthread_mutex_destroy(&p->mutex))) {  //!< 销毁互斥锁
        //!< 如果前面使用了撤销线程，此处可能会出现忙错误
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_destroy[%s(%d)]",
                strerror(ret), ret);
    }
    if(pthread_cond_destroy(&p->cond)) {    //!< 销毁条件变量
        dbg_outerr_W(DS_POOL_ERR, "pthread_cond_destroy");
    }
    free(p);
    *hdl = (long)NULL;
    return 0;
}
/** \brief       创建线程池 */
int pool_thread_new(long * hdl)
{
    int ret;
    POOL_THREAD_T * p = (POOL_THREAD_T *)malloc(sizeof(POOL_THREAD_T));
    do {
        if(p == NULL) {
            dbg_outerr_E(DS_POOL_ERR, "malloc");
            ret = POOL_THREAD_RET_MEM;
            break;
        }
        memset(p, 0x00, sizeof(POOL_THREAD_T));
        if(pthread_mutex_init(&p->mutex, NULL)) {
            dbg_outerr_E(DS_POOL_ERR, "pthread_mutex_init");
            ret = POOL_THREAD_RET_MUTEX_INIT;
            break;
        }
        if(pthread_cond_init(&p->cond, NULL)) {
            dbg_outerr_E(DS_POOL_ERR, "pthread_cond_init");
            ret = POOL_THREAD_RET_COND_INIT;
            break;
        }
        INIT_LIST_HEAD(&p->list_task);
        p->list_task_num = 0;
        INIT_LIST_HEAD(&p->list_thread);
        p->thread_num = 0;
        p->idle_thread_num = 0;
        p->flag_halt = 0;

        if((ret = pool_thread_task_add((long)p, "Thread pool manager",
                        p, manager))) {
            dbg_out_E(DS_POOL_ERR, "Create thread pool manager failed!");
            break;
        }
        if(thread_add(p)) {
            dbg_out_E(DS_POOL_ERR, "Create thread failed!");
            ret = POOL_THREAD_RET_THREAD_CREATE;
            break;
        }
        *hdl = (long)p;
        return 0;
    } while(0);
    if(pthread_mutex_destroy(&p->mutex)) {  //!< 销毁互斥锁
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_destroy");
    }
    if(pthread_cond_destroy(&p->cond)) {    //!< 销毁条件变量
        dbg_outerr_W(DS_POOL_ERR, "pthread_mutex_destroy");
    }
    free(p);
    return ret;
}

