#include <stdlib.h> 
#include <config.h>

extern int test_bio(void *p);
extern int test_ini(void *p);
extern int test_buffer(void *p);
extern int test_thread(void *p);

int main(int argc, char * argv[])
{
    dbg_out_W(DS_TM, "TLS_SVR - module test");
    if(main_init()) {
        dbg_out_E(DS_TM, "Main init failed!!!");
        return -1;
    }

    dbg_test_setlist(
        { "bio",        NULL,   test_bio, },
        { "ini parse",  NULL,   test_ini, },
        { "buffer",     NULL,   test_buffer, },
        { "thread",     NULL,   test_thread, },
        )

    return 0;
}

