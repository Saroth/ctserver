#include <config.h>

int main_init(void)
{
    int ret;
    do {
        if((ret = bio_init())) {
            dbg_out_E(DS_MAIN_ERR, "BIO init failed! %d", ret);
            break;
        }
        if((ret = bio_debug_init())) {
            dbg_out_E(DS_MAIN_ERR, "Debug BIO init failed! %d", ret);
            break;
        }
        return 0;
    } while(0);
    return ret;
}


