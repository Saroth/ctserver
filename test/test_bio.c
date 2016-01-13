#include <stdio.h>
#include <config.h>

int test_bio_init(void *p)
{
    int ret;
    dbg_out_I(DS_TM, "main init...");
    ret = main_init();
    dbg_out_I(DS_TM, "ret: %d", ret);
    return 0;
}

int test_bio(void *p)
{
    dbg_test_setlist(
        { "bio init",   NULL,   test_bio_init,  },
        )
    
    return 0;
}

