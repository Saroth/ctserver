#include <stdlib.h> 
#include <config.h>

extern int test_bio(void *p);
extern int test_ini(void *p);
extern int test_buf_queue(void *p);
extern int test_thread(void *p);
extern int test_server(void *p);

int main(int argc, char * argv[])
{
    mwInit();
    dbg_out_W(DS_TM, "TLS_SVR - module test");
    if(main_init()) {
        dbg_out_E(DS_TM, "Main init failed!!!");
        return -1;
    }

    dbg_test_setlist(
        { "bio",        NULL,   test_bio, },
        { "ini parse",  NULL,   test_ini, },
        { "buf queue",  NULL,   test_buf_queue, },
        { "thread",     NULL,   test_thread, },
        { "server",     NULL,   test_server, },
        { "client",     NULL,   NULL, },
        );

    mwTerm();
    return 0;
}

