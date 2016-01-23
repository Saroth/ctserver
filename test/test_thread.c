#include <config.h>

#include <unistd.h>
#include <pthread.h>

void * test_func(void *p)
{
    int n = *(int *)p;
    int i;
    for(i = 0; i < 5; i++) {
        dbg_out(DS_POOL, "[%d.%d]", n, i);
        sleep(1);
    }
    pthread_t tid = pthread_self();
    pthread_exit((void *)tid);
}

int test_thread_create(void *p)
{
    int i = 0;
    int ret = 0;
    pthread_t tid;
    for(i = 0; i < 5; i++) {
        ret = pthread_create(&tid, NULL, test_func, (void *)&i);
        dbg_out_I(DS_POOL, "Create thread, ret:%d, tid:%#x", ret, tid);
        sleep(1);
    }
    pthread_t t;
    pthread_join(tid, (void **)&t);
    dbg_out_I(DS_POOL, "thread quited, ret:%#x", t);

    return 0;
}


int test_thread(void *p)
{
    dbg_test_setlist(
        { "test",   NULL,   test_thread_create, },
        )
    return 0;
}


