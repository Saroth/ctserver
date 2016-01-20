#include <string.h>
#include <config.h>

static long s_hdl = 0;

int test_buffer_new(void *p)
{
    int ret = buf_new(76, &s_hdl);
    dbg_out_I(DS_TM, "Create new buffer: %d", ret);
    return 0;
}

int test_buffer_del(void *p)
{
    int ret = buf_del(&s_hdl);
    dbg_out_I(DS_TM, "Delete buffer: %d", ret);
    return 0;
}

int test_buffer_append(void *p)
{
    char buf[256] = { 0 };
    dbg_out_I(DS_TM, " >> Input append length:");
    int len = dbg_in();
    if(len < 0 || len > 255) {
        dbg_out_E(DS_TM, "Bad length: %d", len);
        return -1;
    }
    dbg_out_I(DS_TM, " << Get length:%d", len);
    dbg_out_I(DS_TM, " >> Input fill char:");
    dbg_in_S(buf, 256);
    dbg_out_I(DS_TM, " << Get char: %x", buf[0]);
    memset(buf, buf[0], len);
    int ret = buf_append(buf, len, s_hdl);
    dbg_out_I(DS_TM, "Append buffer: %d", ret);
    return 0;
}

int test_buffer_read(void *p)
{
    char buf[256] = { 0 };
    dbg_out_I(DS_TM, " >> Input read offset:");
    int ofs = dbg_in();
    dbg_out_I(DS_TM, " << Get offset: %d(%x)", ofs, ofs);
    dbg_out_I(DS_TM, " >> Input read length:");
    int len = dbg_in();
    dbg_out_I(DS_TM, " << Get length: %d(%x)", len, len);
    int ret = buf_read(buf, len, ofs, s_hdl);
    dbg_out_I(DS_TM, "Read buffer, ret:%d", ret);
    if(ret >= 0) {
        dbg_dmp_HC(DS_TM, buf, ret);
    }
    return 0;
}

int test_buffer_size(void *p)
{
    int ret = buf_size(s_hdl);
    dbg_out_I(DS_TM, "Get size: %d(%x)", ret, ret);
    return 0;
}

int test_buffer(void * p)
{
    dbg_test_setlist(
        { "new",    NULL,   test_buffer_new, },
        { "del",    NULL,   test_buffer_del, },
        { "append", NULL,   test_buffer_append, },
        { "read",   NULL,   test_buffer_read, },
        { "size",   NULL,   test_buffer_size, },
        )
    return 0;
}


