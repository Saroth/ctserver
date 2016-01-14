#include <stdio.h>

#include <config.h>

#if defined(CFG_SYS_UNIX)

static int unix_output(char * buf, int len)
{
    printf(buf);
    fflush(stdout);
    return 0;
}
static int unix_input(char * buf, int len)
{
    int ch = 0;
    char * b = buf;
    while(1) {
        ch = getchar();
        if(ch == EOF) {
            break;
        }
        else {
            *b = ch;
            if(*b == '\n' || (++b - buf) >= len) {
                break;
            }
        }
    }
    return 0;
}
static BIO_IO_T s_bio_io_unix = {
    .desc   = "Unix standard I/O",
    .output = unix_output,
    .input  = unix_input,
};
#endif /* defined(CONF_SYS_UNIX) */

static BIO_IO_T * s_bio_io = NULL;
int bio_io_init(void * p)
{
    if(p) {
        s_bio_io = (BIO_IO_T *)p;
    }
    else {
#if defined(CFG_SYS_UNIX)
        s_bio_io = &s_bio_io_unix;
#endif /* defined(CONF_SYS_UNIX) */
    }
    return 0;
}
BIO_IO_T * bio_io(void)
{
    return s_bio_io;
}

