#include <config.h>

void proc_get_rand(char * buf, int len)
{
    unsigned int t = time(NULL);
    static unsigned int c = 1;
    c += 3;
    srand(c + t);
    int i;
    for(i = 0; i < len; i++) {
        buf[i] = rand() % 0xFF;
    }
}

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
        proc_get_rand(buf, ret);
        ret = info->write(info, buf, ret);
        if(ret <= 0) {
            break;
        }
    }
    info->postproc(info, SOCK_LINK_STATE_IDLE);
}

PROC_T g_proc_random = {
    .name       = "random",
    .process    = proc,
    .close      = NULL,
};

