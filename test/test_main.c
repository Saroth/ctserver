#include <stdlib.h> 
#include <config.h>

extern int test_bio(void *p);
extern int test_ini(void *p);
extern int test_buffer(void *p);

int test_main_entry(int argc, char * argv[])
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
        )

    return 0;
}

int __mdl_test_entry__(int argc, char * argv[])
{
    test_main_entry(argc, argv);
    exit(0);
}


