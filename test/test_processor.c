#include <config.h>

int test_get_rand(void *p)
{
    extern void proc_get_rand(char * buf, int len);
    int i;
    char buf[256] = { 0 };
    for(i = 0; i < 20; i++) {
        int len = (i + 1) * 3;
        proc_get_rand(buf, len);
        dbg_dmp_HC(DS_TM, buf, len);
    }
    return 0;
}

int test_processor(void *p)
{
    dbg_test_setlist(
        { "Get random data",    0,  test_get_rand,  },
        );
    return 0;
}

