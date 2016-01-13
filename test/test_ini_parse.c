#include <stdio.h>
#include <string.h>

#include <config.h>
#include "../src/utility/conf/conf_ini.h"

int test_str_trim(void *p)
{
    char buf[2][32] = {
        { "\t \x01\x1F\x10 test = 123 \x19\t", },
        { "中文测试", },
    };
    int ret;
    int i = 0;
    for(; i < 2; i++) {
        dbg_out_I(DS_TM, "String before trim:");
        dbg_dmp_HC(DS_TM, buf[i], 32);
        dbg_out_I(DS_TM, "Trim...");
        ret = ini_str_trim(buf[i]);
        dbg_out_I(DS_TM, "ret: %d", ret);
        dbg_out_I(DS_TM, "String after trim:");
        dbg_dmp_HC(DS_TM, buf[i], 32);
    }
    return 0;
}


int test_ini(void *p)
{
    dbg_test_setlist(
        { "string trim",          NULL,           test_str_trim,        },
        )
    return 0;
}

