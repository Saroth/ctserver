#include <stdio.h>
#include <string.h>

#include <config.h>
#include "../src/utility/conf/conf_ini.h"

int test_str_trim(void *p)
{
    extern int str_trim(char * buf);
    char buf[3][32] = {
        { "\t \x01\x1F\x10 test = 123 \x19\t", },
        { "中文测试", },
        { "\x02 中 文 测 试 \t  ", },
    };
    int ret;
    int i = 0;
    for(; i < 3; i++) {
        dbg_out_I(DS_TM, "String before trim:");
        dbg_dmp_HC(DS_TM, buf[i], 32);
        dbg_out_I(DS_TM, "Trim...");
        ret = str_trim(buf[i]);
        dbg_out_I(DS_TM, "ret: %d", ret);
        dbg_out_I(DS_TM, "String after trim:");
        dbg_dmp_HC(DS_TM, buf[i], 32);
    }
    return 0;
}

int test_ini_analyze(void *p)
{
    long fp = NULL;
    char * test_ini_file = "../test/ini_test.ini";
    dbg_out_I(DS_TM, "Open file: %s", test_ini_file);
    int ret = bio_fctl()->open(test_ini_file, "rb+", &fp);
    if(ret) {
        dbg_out_E(DS_TM, "Open file failed: %s, ret: %d", test_ini_file, ret);
        return -1;
    }
    dbg_out_I(DS_TM, "Analyze ini...");
    ret = conf_ini_load(fp, NULL, 0);
    if(ret) {
        dbg_out_E(DS_TM, "ini load, ret: %d", ret);
        return -1;
    }
    bio_fctl()->close(&fp);

    return 0;
}

int test_ini(void *p)
{
    dbg_test_setlist(
        { "string trim",          NULL,           test_str_trim,        },
        { "ini analyze",          NULL,           test_ini_analyze,     },
        )
    return 0;
}

