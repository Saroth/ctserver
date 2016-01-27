#include <stdlib.h>

#include <config.h>

enum INI_ANALYSIS_E{                    //!< 行类型
    INI_GET_UNRECOGNIZED = 0,           //!< 无法解析数据
    INI_GET_EMPTY,                      //!< 空行
    INI_GET_KEYVAL,                     //!< 键值
    INI_GET_SECTION,                    //!< 节
    INI_GET_END,                        //!< 文件末尾
    INI_GET_READ_ERR,                   //!< 错误
};
/**
 * \block:      字符解析
 * @{ */
/** 字符串裁剪，去除首尾不可见字符，将破坏原字符串 */
int str_trim(char * buf)
{
    int len = strlen(buf);
    char * buf_h = buf;
    /// 0x20~0x7E为可见字符，0x80以上通常为多字节字符，排除空格0x20
    /// 去除头部不可见字符
    while(len > 0 && (unsigned char)*buf_h < 0x21) {
        len--;
        buf_h++;
    }
    memmove(buf, buf_h, len + 1);       /// 包含结束符复制
    /// 去除尾部不可见字符
    while(len > 0 && (unsigned char)buf[len - 1] < 0x21) {
        len--;
    }
    buf[len] = 0;
    return len;
}
/** 值解析 */
static int ini_value_analyze(char * buf)
{
    while(1) {
        switch(*buf) {
            case 0: { return 0; }
            case CONF_INI_CHAR_ESCAPE: { buf++; break; }
            case CONF_INI_CHAR_COMMENT: { *buf = 0; buf--; break; }
            default: {
                if(strchr(CONF_INI_CHARS_STRING, *buf)) {
                    char * pair = strchr(buf + 1, *buf);
                    if(pair) {
                        buf = pair;
                    }
                }
                break;
            }
        }
        buf++;
    }
    return 0;
}
/** 判断是否是键值 */
static int ini_line_is_keyval(char * buf)
{
    char * buf_a = strchr(buf, CONF_INI_CHAR_ASSIGN);
    char * buf_c = strchr(buf, CONF_INI_CHAR_COMMENT);
    if(buf_a == NULL || buf_a == buf        // 无赋值符或无键名
            || (buf_c && buf_c < buf_a)) {  // 注释符在赋值符前
        dbg_out_I(DS_CONF_INI, "No assign char or no key name.");
        return INI_GET_UNRECOGNIZED;
    }
    dbg_out_I(DS_CONF_INI, "Get key & value: %s", buf);
    return INI_GET_KEYVAL;
}
/** 判断是否是节标记，并自动移除节名前后标记符 */
static int ini_line_is_section(char * buf)
{
    if(buf[0] != CONF_INI_CHAR_SECTION_HEAD) {
        dbg_out_I(DS_CONF_INI, "No section head char.");
        return INI_GET_UNRECOGNIZED;
    }
    char * buf_s = buf + 1;
    char * buf_c = strchr(buf, CONF_INI_CHAR_COMMENT);
    char * buf_e = strchr(buf, CONF_INI_CHAR_SECTION_END);
    if(buf_e == NULL                        // 无后标记符
            || (buf_c && buf_c < buf_e)) {  // 注释符在后标记符前
        dbg_out_I(DS_CONF_INI, "No section end char.");
        return INI_GET_UNRECOGNIZED;
    }
    *buf_e = 0;
    int len = str_trim(buf_s);
    if(len == 0) {
        dbg_out_W(DS_CONF_INI_ERR, "No section name.");
        return INI_GET_EMPTY;               //!< 非法语句，缓存数据已被修改
    }
    memmove(buf, buf_s, len);
    buf[len] = 0;
    dbg_out_I(DS_CONF_INI, "Get section: %s", buf);
    return INI_GET_SECTION;
}
/** 读取一行字符，并判断类型 */
static int ini_line_read(long fp, char * buf)
{
    int ret;
    char * buf_p = buf;
    /// 读取一行字符
    while(1) {
        ret = bio_fctl()->read(buf_p, 1, fp);
        if(ret < 0 || ret > 1) {
            dbg_out_E(DS_CONF_INI_ERR, "Read file failed, ret:%d", ret);
            return INI_GET_READ_ERR;
        }
        if(ret == 0 || *buf_p == '\n') {
            if(ret == 0 && buf_p == buf) {
                dbg_out_I(DS_CONF_INI, "End of file.");
                return INI_GET_END;     //!< 无数据
            }
            break;                      //!< 读到换行符或文件末尾
        }
        buf_p++;
    }
    dbg_out_I(DS_CONF_INI, "#### buf: %s", buf);
    /// 字符解析
    if(str_trim(buf) == 0) {
        dbg_out_I(DS_CONF_INI, "Empty line.");
        return INI_GET_EMPTY;
    }
    if((ret = ini_line_is_section(buf))) {
        return ret;
    }
    if((ret = ini_line_is_keyval(buf))) {
        return ret;
    }
    dbg_out_W(DS_CONF_INI_ERR, "Unrecognized line: %s", buf);
    return INI_GET_UNRECOGNIZED;
}
/** @} */
/**
 * \block:      配置操作接口
 * @{ */
/**
 * \brief       添加新结构体到链表
 * \param       buf         读取的一行字符
 * \param       tbls        节信息表结构体(数组)指针
 * \param       num         节信息表结构体个数
 * \param [out] addr        新申请的结构体地址
 * \return      >=0:节信息表ID  <0:Error code:CONF_INI_RET_E
 */
static int ini_add_section(char * buf, CONF_INI_SECTBL_T * tbls,
        int num, void ** addr)
{
    int i = 0;
    for(; i < num; i++, tbls++) {
        if(strstr(buf, tbls->name)) {
            break;
        }
    }
    if(i == num) {
        dbg_out_W(DS_CONF_INI_ERR, "Unknown section name: %s", buf);
        return CONF_INI_RET_UNKNOWN_SEC;
    }
    *addr = malloc(tbls->keytblst_size);
    if(*addr == NULL) {
        dbg_outerr_E(DS_CONF_INI_ERR, "malloc");
        return CONF_INI_RET_MEM;
    }
    /// 申请新的结构体
    memset(*addr, 0x00, tbls->keytblst_size);
    struct list_head * ptr = (struct list_head *)((unsigned long)(*addr)
            + tbls->keytblst_ptr_ofs);
    /// 添加结构体到链表
    list_add_tail(ptr, tbls->list);
    dbg_out_I(DS_CONF_INI_MEM, "Add section: %d, malloc:%#x, size:%d, ptr: %#x",
            i, *addr, tbls->keytblst_size, ptr);
    return i;
}
/**
 * \brief       添加新键值到结构体
 * \param       buf         读取的一行字符
 * \param       tbl         节信息表结构体
 * \param       addr        ini_add_section中申请的结构体指针
 * \return      0:Success   <0:Error code:CONF_INI_RET_E
 */
static int ini_add_keyval(char * buf, CONF_INI_SECTBL_T * tbl, void * addr)
{
    char * buf_v = strchr(buf, CONF_INI_CHAR_ASSIGN);
    *buf_v++ = 0;
    ini_value_analyze(buf_v);
    str_trim(buf_v);
    str_trim(buf);
    /// 查找匹配键名
    int i = 0;
    CONF_INI_KEYTBL_T * keytbl = tbl->keytbl;
    for(; i < tbl->keytbl_keynum; i++) {
        if(strcmp((keytbl + i)->name, buf) == 0) {
            break;
        }
    }
    if(i == tbl->keytbl_keynum) {
        dbg_out_W(DS_CONF_INI_ERR, "Unknown key name: %s", buf);
        return CONF_INI_RET_UNKNOWN_KEY;
    }
    /// 申请存储空间
    char * val = (char *)malloc(strlen(buf_v) + 4);
    if(val == NULL) {
        dbg_outerr_E(DS_CONF_INI_ERR, "malloc");
        return CONF_INI_RET_MEM;
    }
    /// 将空间地址保存到结构体
    memset(val, 0x00, sizeof(strlen(buf_v) + 4));
    strcpy(val, buf_v);
    *(char **)((unsigned long)addr + (keytbl + i)->ofs) = val;
    dbg_out_I(DS_CONF_INI_MEM, "\tAdd key value, malloc:%#x, size:%d, ofs:%d",
            val, strlen(buf_v) + 4, (keytbl + i)->ofs);

    return 0;
}

/** 加载配置文件 */
int conf_ini_load(long fp, CONF_INI_SECTBL_T * tbls, unsigned int num)
{
    int ret;
    char buf[4096];
    void * addr = NULL;
    CONF_INI_SECTBL_T * tbl = NULL;
    while(1) {
        memset(buf, 0x00, sizeof(buf));
        switch(ini_line_read(fp, buf)) {
            case INI_GET_SECTION: {
                tbl = NULL;
                ret = ini_add_section(buf, tbls, num, &addr);
                if(ret >= 0 && ret < num) {
                    tbl = tbls + ret;
                }
                else if(ret != CONF_INI_RET_UNKNOWN_SEC) {
                    return ret;         //!< 反馈未知节名之外的所有错误
                }
                break;
            }
            case INI_GET_KEYVAL: {
                if(tbl) {
                    ret = ini_add_keyval(buf, tbl, addr);
                    if(ret && ret != CONF_INI_RET_UNKNOWN_KEY) {
                        return ret;     //!< 反馈未知键名之外的所有错误
                    }
                }
                break;
            }
            case INI_GET_READ_ERR: { return CONF_INI_RET_READ_ERR; }
            case INI_GET_END: { return 0; }
            case INI_GET_UNRECOGNIZED:
            case INI_GET_EMPTY:
            default: break;
        }
    }
    return 0;
}
/** 写入多个节表中的所有键表数据到配置文件 */
int conf_ini_save(long fp, CONF_INI_SECTBL_T * tbl, unsigned int num)
{
    return 0;
}
/** 清除某个表加载的数据 */
int conf_ini_clean(CONF_INI_SECTBL_T * tbl)
{
    struct list_head * ptr = NULL;
    struct list_head * head = tbl->list;
    dbg_out_I(DS_CONF_INI_MEM, "Clean list: %s", tbl->name);
    /// 遍历所有节点
    list_for_each(ptr, head) {
        void * addr = (void *)((unsigned long)ptr - tbl->keytblst_ptr_ofs);
        dbg_out_I(DS_CONF_INI_MEM, "\tGet entry:%#x, ptr:%#x", addr, ptr);
        /// 遍历所有结构体成员
        for(int i = 0; i < tbl->keytbl_keynum; i++) {
            void ** node = (void **)((unsigned long)addr + tbl->keytbl[i].ofs);
            if(*node) {
                free(*node);
            }
            dbg_out_I(DS_CONF_INI_MEM, "\t\tfree: %#x, ofs: %#x", *node, node);
        }
        dbg_out_I(DS_CONF_INI_MEM, "\tfree entry: %#x", addr);
        // 只能删除节点, 不能清空数据或使用list_del, 否则会找不到下一个节点
        __list_del_entry(ptr);
        free(addr);
    }
    return 0;
}
/** 获取某类型的节的链表中，某个结构体指针 */
int conf_ini_get_entry(CONF_INI_SECTBL_T * tbl, unsigned int idx, void ** entry)
{
    int i = 0;
    struct list_head *ptr = NULL;
    list_for_each(ptr, tbl->list) {
        if(idx == i++) {
            *entry = (void *)((unsigned long)ptr - tbl->keytblst_ptr_ofs);
            dbg_out_I(DS_CONF_INI, "Get entry:%#x, idx:%d, ptr:%#x",
                    *entry, idx, ptr);
            return 0;
        }
    }
    dbg_out_E(DS_CONF_INI_ERR, "Param out of range: idx: %d", idx);
    return CONF_INI_RET_PARAM_ERR;
}
/** 获取某类型的节的链表结构体个数 */
int conf_ini_get_size(CONF_INI_SECTBL_T * tbl)
{
    int num = 0;
    struct list_head *ptr = NULL;
    list_for_each(ptr, tbl->list) {
        num++;
    }
    return num;
}

/** @} */

