#include <stdlib.h> 
#include <config.h>

int main_init(void)
{
    int ret;
    do {
        if((ret = bio_init())) {
            dbg_out_E(DS_SVR_ERR, "BIO init failed! %d", ret);
            break;
        }
        if((ret = bio_debug_init())) {
            dbg_out_E(DS_SVR_ERR, "Debug BIO init failed! %d", ret);
            break;
        }
        return 0;
    } while(0);
    return ret;
}

static int main_entry(int argc, char * argv[])
{
    int ret;
    dbg_out_entry(DS_SVR);
    do {
        if((ret = main_init())) {
            break;
        }
        dbg_out_exit(DS_SVR);
        return 0;
    } while(0);

    dbg_out_E(DS_SVR_ERR, "Fatal error: %d(%p), stop!", ret, ret);
    return -1;
}

int __tls_svr_entry__(int argc, char * argv[])
{
    main_entry(argc, argv);
    exit(0);
}

