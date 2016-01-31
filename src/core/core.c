#include <stdlib.h> 
#include <config.h>

int main(int argc, char * argv[])
{
    int ret;
    do {
        if((ret = main_init())) {       //!< 程序初始化
            break;
        }
        if((ret = option_parse(argc, argv))) {  //!< 选项处理
            break;
        }
        if((ret = main_process())) {    //!< 主流程
            break;
        }
        return 0;
    } while(0);

    dbg_out_E(DS_MAIN_ERR, "Fatal error: %d(%p), stop!", ret, ret);
    return -1;
}

