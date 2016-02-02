#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t * g_mutex;
static pthread_cond_t * g_cond;

void * test_func(void *p)
{
    int n = *(int *)p;
    int i;
    pthread_t tid = pthread_self();
    dbg_out_I(DS_POOL, "Thread %#x start", tid);
    for(i = 0; i < 4; i++) {
        pthread_mutex_lock(g_mutex);
        pthread_cond_wait(g_cond, g_mutex);
        pthread_mutex_unlock(g_mutex);
        dbg_out_I(DS_POOL, "[%d.%d]", n, i);
        sleep(1);
    }
    dbg_out_I(DS_POOL, "Thread %#x quit", tid);
    pthread_exit((void *)tid);
}
int test_thread_create(void *p)
{
    int i = 0;
    int ret = 0;
    pthread_t tid;
    for(i = 0; i < 4; i++) {
        ret = pthread_create(&tid, NULL, test_func, (void *)&i);
        dbg_out_I(DS_POOL, "Create thread, ret:%d, tid:%#x", ret, tid);
        sleep(1);
    }
    return 0;
}
int test_send_cond_signal(void *p)
{
    dbg_out_I(DS_POOL, "Send condition signal.");
    pthread_cond_signal(g_cond);
    return 0;
}

static long g_threadpool_hdl;
int test_create_threadpool(void *p)
{
    dbg_out_I(DS_TM, "Create thread pool...");
    int ret = pool_thread_new(&g_threadpool_hdl);
    dbg_out_I(DS_TM, "return: %d", ret);
    return 0;
}
void test_task(void *p)
{
    int i = *(int *)p;
    dbg_out_I(DS_TM, "task %d start...", i);
    sleep(5);
    dbg_out_I(DS_TM, "task %d finish", i);
}
int test_add_task(void *p)
{
    dbg_out_I(DS_TM, " >> Input task num:");
    int num = dbg_in();
    dbg_out_I(DS_TM, " << Get num:%d", num);
    int i;
    int ret;
    for(i = 0; i < num; i++) {
        ret = pool_thread_task_add(g_threadpool_hdl, "test", &i,
                sizeof(int), test_task);
        dbg_out_I(DS_TM, "Add task: %d(%d)", i, ret);
    }
    return 0;
}
int test_destroy_threadpool(void *p)
{
    dbg_out_I(DS_TM, "Destroy thread pool...");
    int ret = pool_thread_del(&g_threadpool_hdl);
    dbg_out_I(DS_TM, "return: %d", ret);
    return 0;
}
int test_task_count(void *p)
{
    struct list_head * ptr;
    int i = 0;
    POOL_THREAD_T * hdl = (POOL_THREAD_T *)g_threadpool_hdl;
    if(hdl == NULL) {
        return -1;
    }
    dbg_out_I(DS_TM, "Task num in count: %d", hdl->list_task_num);
    list_for_each(ptr, &hdl->list_task) {
        i++;
    }
    dbg_out_I(DS_TM, "Task num in list: %d", i);
    return 0;
}
int test_thread_monitor(void *p)
{
    struct list_head * ptr;
    int i = 0;
    POOL_THREAD_T * hdl = (POOL_THREAD_T *)g_threadpool_hdl;
    if(hdl == NULL) {
        return -1;
    }
    dbg_out_I(DS_TM, "List all threads: (%d/%d)",
            hdl->idle_thread_num, hdl->thread_num);
    list_for_each(ptr, &hdl->list_thread) {
        i++;
        THREAD_T * t = list_entry(ptr, THREAD_T, ptr);
        dbg_out_I(DS_TM, " > %d. [%#x] state:%d, task:%#x",
                i, t->tid, t->state, t->task);
        if(t->task) {
            dbg_out_I(DS_TM, " \t task name: %#x", t->task->name);
            dbg_out_I(DS_TM, " \t task name: %s", t->task->name);
        }
    }
    return 0;
}

int test_thread(void *p)
{
    g_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    g_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(g_mutex, NULL);
    pthread_cond_init(g_cond, NULL);
    dbg_test_setlist(
        { "Create 4 threads",       NULL,   test_thread_create, },
        { "Send condition signal",  NULL,   test_send_cond_signal,  },
        { "====",                   NULL,   NULL,   },
        { "Create thread pool",     NULL,   test_create_threadpool, },
        { "Add task to pool",       NULL,   test_add_task,  },
        { "Destroy thread pool",    NULL,   test_destroy_threadpool,    },
        { "Get task count",         NULL,   test_task_count,    },
        { "thread monitor",         NULL,   test_thread_monitor,    },
        );
    pthread_mutex_destroy(g_mutex);
    pthread_cond_destroy(g_cond);
    free(g_mutex);
    free(g_cond);
    return 0;
}


