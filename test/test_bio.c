#include <stdio.h>
#include <config.h>

int test_bio_sem(void *p)
{
    int ret = 0;
    long hdl = 0;
    while(1) {
        dbg_out_I(DS_TM, "####");
        dbg_out_I(DS_TM, "1.New");
        dbg_out_I(DS_TM, "2.Delete");
        dbg_out_I(DS_TM, "3.Wait");
        dbg_out_I(DS_TM, "4.Post");
        dbg_out_I(DS_TM, "5.Val");
        switch(dbg_in()) {
            case 1: {
                dbg_out_I(DS_TM, "Make new semaphore...");
                ret = bio_sem()->new(1, &hdl);
                dbg_out_I(DS_TM, "Return: %d, hdl: %x", ret, hdl);
                break;
            }
            case 2: {
                dbg_out_I(DS_TM, "Delete semaphore(%x)...", hdl);
                ret = bio_sem()->del(&hdl);
                dbg_out_I(DS_TM, "Return: %d, hdl: %x", ret, hdl);
                break;
            }
            case 3: {
                dbg_out_I(DS_TM, " >> Set time:(-1:Forever, 0:noWait, ~:(ms))");
                int time = dbg_in();
                dbg_out_I(DS_TM, " << Get time: %d", time);
                ret = bio_sem()->wait(time, hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 4: {
                ret = bio_sem()->post(hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 5: {
                ret = bio_sem()->val(hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 0: {
                return 0;
            }
        }
    }

    return 0;
}

int test_bio_file(void *p)
{
    int ret;
    long hdl = 0;
    while(1) {
        dbg_out_I(DS_TM, "####");
        dbg_out_I(DS_TM, "1.Open");
        dbg_out_I(DS_TM, "2.Close");
        dbg_out_I(DS_TM, "3.Write");
        dbg_out_I(DS_TM, "4.Read");
        dbg_out_I(DS_TM, "5.Seek");
        dbg_out_I(DS_TM, "6.Truncate");
        switch(dbg_in()) {
            case 1: {
                char name[256] = { 0 };
                dbg_out_I(DS_TM, "Input file name:");
                dbg_in_S(name, 256);
                dbg_out_I(DS_TM, "Get name:%s", name);
                ret = bio_fctl()->open(name, "ab+", &hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 2: {
                dbg_out_I(DS_TM, "Close...");
                ret = bio_fctl()->close(&hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 3: {
                break;
            }
            case 4: {
                break;
            }
            case 5: {
                break;
            }
            case 6: {
                break;
            }
            case 0: {
                return 0;
            }
        }
    }
    return 0;
}

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
        { "file",       NULL,   test_bio_file,  },
        { "sem",        NULL,   test_bio_sem,  },
        )
    
    return 0;
}

