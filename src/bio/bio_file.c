#include <stdio.h>
#include <config.h>
#include "bio.h"

#if defined(CFG_SYS_UNIX)
#include <unistd.h>
#include <errno.h>

static int unix_close(long * fp)
{
    if(*fp) {
        if(fclose((FILE *)*fp)) {
            dbg_outerr_I(DS_BIO_ERR, "fclose:");
        }
        *fp = (long)NULL;
    }
    return 0;
}
static int unix_open(char * path, const char * mode, long * fp)
{
    unix_close(fp);
    *fp = (long)fopen(path, mode);
    if(*fp == (long)NULL) {
        dbg_outerr_I(DS_BIO_ERR, "fopen:");
        return -1;
    }
    return 0;
}
static int unix_write(void * buf, int len, long fp)
{
    if(fwrite(buf, 1, len, (FILE *)fp) != len) {
        dbg_outerr_I(DS_BIO_ERR, "fwrite:");
        return -1;
    }
    return 0;
}
static int unix_read(void * buf, int len, long fp)
{
    int ret = fread(buf, 1, len, (FILE *)fp);
    if(errno == EOF) {
        dbg_out_W(DS_BIO_ERR, "End of file");
    }
    else if(ret < 0) {
        dbg_outerr_I(DS_BIO_ERR, "fread:");
        return -1;
    }
    return ret;
}
static int unix_seek(long ofs, int whence, long fp)
{
    int ret = fseek((FILE *)fp, ofs, whence);
    if(ret < 0) {
        dbg_outerr_I(DS_BIO_ERR, "fseek:");
    }
    return ret;
}
static int unix_truncate(char * path, long length)
{
    int ret = truncate(path, length);
    if(ret < 0) {
        dbg_outerr_I(DS_BIO_ERR, "truncate:");
    }
    return 0;
}
static BIO_FCTL_T s_bio_fctl_unix = {
    .desc       = "Unix standard I/O",
    .open       = unix_open,
    .close      = unix_close,
    .write      = unix_write,
    .read       = unix_read,
    .seek       = unix_seek,
    .truncate   = unix_truncate,
};
#endif /* defined(CONF_SYS_UNIX) */

static BIO_FCTL_T * s_bio_fctl = NULL;
int bio_fctl_init(void * p)
{
    if(p) {
        s_bio_fctl = (BIO_FCTL_T *)p;
    }
    else {
#if defined(CFG_SYS_UNIX)
        s_bio_fctl = &s_bio_fctl_unix;
#endif /* defined(CONF_SYS_UNIX) */
    }
    return 0;
}
inline BIO_FCTL_T * bio_fctl(void)
{
    return s_bio_fctl;
}

