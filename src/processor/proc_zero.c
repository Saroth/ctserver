#include <config.h>

static void proc(void * p)
{
    int ret;
    SOCK_LINK_INFO_T * info = *(SOCK_LINK_INFO_T **)p;
    char buf[1024];
    while(pool_thread_run(g_hdl_thread_pool)) {
        ret = info->read(info, buf, 1024);
        if(ret <= 0) {
            break;
        }
        memset(buf, 0x00, ret);
        ret = info->write(info, buf, ret);
        if(ret <= 0) {
            break;
        }
    }
    info->postproc(info, SOCK_LINK_STATE_IDLE);
}

PROC_T g_proc_zero = {
    .name       = "zero",
    .process    = proc,
    .close      = NULL,
};

