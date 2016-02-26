#include <stdlib.h> 
#include <config.h>

extern int test_bio(void *p);
extern int test_ini(void *p);
extern int test_buf_queue(void *p);
extern int test_thread(void *p);
extern int test_server(void *p);
extern int test_processor(void *p);

long g_hdl_thread_pool = NULL;

int main(int argc, char * argv[])
{
    mwInit();
    dbg_out_I(DS_TM, "TLS_SVR - module test");
    if(main_init()) {
        dbg_out_E(DS_TM, "Main init failed!!!");
        return -1;
    }
    if(pool_thread_new(&g_hdl_thread_pool)) {
        dbg_out_E(DS_TM, "Thread pool init failed!!!");
        return -1;
    }

    dbg_test_setlist(
        { "bio",        NULL,   test_bio, },
        { "ini parse",  NULL,   test_ini, },
        { "buf queue",  NULL,   test_buf_queue, },
        { "thread",     NULL,   test_thread, },
        { "server",     NULL,   test_server, },
        { "client",     NULL,   NULL, },
        { "processor",  NULL,   test_processor, },
        );
    pool_thread_del(&g_hdl_thread_pool);

    mwTerm();
    return 0;
}

