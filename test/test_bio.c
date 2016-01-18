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
        dbg_out_I(DS_TM, "3.Force delete");
        dbg_out_I(DS_TM, "4.P");
        dbg_out_I(DS_TM, "5.V");
        dbg_out_I(DS_TM, "6.Val");
        dbg_out_I(DS_TM, "7.SetVal");
        switch(dbg_in()) {
            case 1: {
                char name[256] = { 0 };
                dbg_out_I(DS_TM, " >> Input name for get key(e.g.: .)");
                dbg_in_S(name, 256);
                dbg_out_I(DS_TM, " << Get name: %s", name);
                dbg_out_I(DS_TM, " >> Input sem number:");
                int num = dbg_in();
                dbg_out_I(DS_TM, " << Get sem number: %d", num);
                dbg_out_I(DS_TM, "Make new semaphore...");
                ret = bio_sem()->new(name, num, 1, &hdl);
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
                char name[256] = { 0 };
                dbg_out_I(DS_TM, " >> Input name for get key(e.g.: .)");
                dbg_in_S(name, 256);
                dbg_out_I(DS_TM, " << Get name: %s", name);
                dbg_out_I(DS_TM, "Force delete semaphore(%x)...", hdl);
                ret = bio_sem()->fdel(name);
                dbg_out_I(DS_TM, "Return: %d, hdl: %x", ret, hdl);
                break;
            }
            case 4: {
                dbg_out_I(DS_TM, " >> Input sem number:");
                int num = dbg_in();
                dbg_out_I(DS_TM, " << Get sem number: %d", num);
                dbg_out_I(DS_TM, " >> Set time:(-1:Forever, 0:noWait)");
                int time = dbg_in();
                dbg_out_I(DS_TM, " << Get time: %d", time);
                ret = bio_sem()->p(num, time, hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 5: {
                dbg_out_I(DS_TM, " >> Input sem number:");
                int num = dbg_in();
                dbg_out_I(DS_TM, " << Get sem number: %d", num);
                ret = bio_sem()->v(num, hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 6: {
                dbg_out_I(DS_TM, " >> Input sem number:");
                int num = dbg_in();
                dbg_out_I(DS_TM, " << Get sem number: %d", num);
                ret = bio_sem()->val(num, hdl);
                dbg_out_I(DS_TM, "Return: %d", ret);
                break;
            }
            case 7: {
                dbg_out_I(DS_TM, " >> Input sem number:");
                int num = dbg_in();
                dbg_out_I(DS_TM, " << Get sem number: %d", num);
                dbg_out_I(DS_TM, " >> Input value:");
                int val = dbg_in();
                dbg_out_I(DS_TM, " << Get val: %d", val);
                ret = bio_sem()->setval(num, val, hdl);
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

