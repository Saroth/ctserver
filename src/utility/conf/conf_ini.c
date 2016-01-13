#include <string.h>

#include <config.h>

int ini_str_trim(char * buf)
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



// // 读取一行字符
// int _read_line(FILE * fp, char * str)
// {
//     char ch = 0;
//     int ret = 0;
//     int len = 0;
// 
//     while(1) {
//         ret = fread(&ch, 1, 1, fp);
//         if(ch == '\n') {
//             break;
//         }
//         if(feof(fp) || ret == 0) {
//             // 没有换行符的最后一行记录
//             if(len > 0) {
//                 break;
//             }
//             dbg_out_I(DS_INI, "End of file");
//             return -INI_END;
//         }
//         str[len++] = ch;
//     }
//     if(len == 0) {
//         dbg_out_I(DS_INI, "Empty line");
//         return -INI_SKIP;
//     }
//     return len;
// }
// 
// // 判断是否是节标记
// int _chk_section(char * buf, char * sec, char * presec)
// {
//     char * buf_start = NULL;
//     char * buf_end = NULL;
// 
//     if((buf_start = strchr(buf, '[')) == NULL
//             || (buf_end = strchr(buf, ']')) == NULL) {
//         if(presec == NULL || presec[0] == 0) {
//             // 跳过无效记录
//             dbg_out_I(DS_INI, "No section before, Skip");
//             return -INI_SKIP;
//         }
//         else {
//             // 不是节, 使用前一次的节名
//             strcpy(sec, presec);
//         }
//         return 0;
//     }
//     else {
//         // 处理节名
//         char * buf_p = NULL;
//         buf_start++;
//         if(buf_start == buf_end) {
//             // 节名为空
//             dbg_out_I(DS_INI, "Section name is empty");
//             return -INI_WARNING;
//         }
//         if((buf_p = strchr(buf, ';')) > 0 && buf_p < buf_end) {
//             // 注释符出现在节名之中
//             dbg_out_I(DS_INI, "Invalid section name");
//             return -INI_WARNING;
//         }
//         memmove(sec, buf_start, buf_end - buf_start);
//         if(str_trim(sec) <= 0) {
//             // 去除不可见字符后无内容
//             dbg_out_I(DS_INI, "Section name is empty");
//             return -INI_WARNING;
//         }
//         // 获取到节
//         strcpy(presec, sec);
//         return -INI_SEC;
//     }
// 
//     return 0;
// }
// 
// // 值字符串转换
// int _transform_value(char * bufin, char * bufout)
// {
//     char ch = 0;
// 
//     while(1) {
//         if(ch) {
//             // 字符串引号匹配
//             if(*bufin == ch) {
//                 // 匹配到第二个符号
//                 ch = 0;
//                 bufin++;
//                 continue;
//             }
//         }
//         else if(*bufin == '\\') {
//             // 转义符 保留引号内的转义符
//             bufin++;
//         }
//         else if(*bufin == '"' || *bufin == '\'' || *bufin == '`') {
//             if(strchr(bufin + 1, *bufin)) {
//                 // 检查发现存在第二个符号
//                 ch = *bufin;
//                 bufin++;
//                 continue;
//             }
//         }
//         else if(*bufin == ';') {
//             // 注释符
//             *bufout = 0;
//             break;
//         }
//         else if(*bufin == '\0') {
//             // 结束
//             *bufout = *bufin;
//             break;
//         }
//         *bufout++ = *bufin++;
//     }
// 
//     return 0;
// }
// 
// // 解析键值
// int _get_keyval(char * buf, ini_cfg_t * rec)
// {
//     char * buf_p = NULL;
//     char * buf_c = NULL;
// 
//     // 获取键
//     if((buf_p = strchr(buf, '=')) == NULL) {
//         // 没有等号, 非法语句
//         dbg_out_I(DS_INI, "Invalid config");
//         return -INI_WARNING;
//     }
//     if((buf_c = strchr(buf, ';')) && buf_c < buf_p) {
//         // 等号前有注释符
//         dbg_out_I(DS_INI, "Invalid key name");
//         return -INI_WARNING;
//     }
//     memmove(rec->Key, buf, buf_p - buf);
//     if(str_trim(rec->Key) == 0) {
//         // 没有键内容, 非法语句
//         dbg_out_I(DS_INI, "No keyname.");
//         return -INI_WARNING;
//     }
//     buf_p++;
//     // 获取值
//     _transform_value(buf_p, rec->Value);
//     rec->ValLen = str_trim(rec->Value);
// 
//     return 0;
// }
// 
// /*
//  * 获取一行记录
//  * @In:     fp:     配置文件的文件指针
//  *          presec: 前一次读到的节名, 没有则传入NULL
//  * @Out:    rec:    配置记录结构体
//  * @Ret:    ini_ret_t
//  */
// int ini_GetRecord(FILE * fp, ini_cfg_t * rec, char * presec)
// {
//     char buf[BUFFER_SIZE] = { 0 };
//     int ret = 0;
// 
//     if(NULL == fp || NULL == rec) {
//         dbg_out_I(DS_INI_ERR, "param error!");
//         return -INI_ERROR;
//     }
//     // 读取一行数据
//     if((ret = _read_line(fp, buf)) < 0) {
//         return ret;
//     }
//     memset(rec, 0x00, sizeof(ini_cfg_t));
//     // 判断节
//     if((ret = _chk_section(buf, rec->Section, presec)) < 0) {
//         return ret;
//     }
//     // 处理读到的键值
//     if((ret = _get_keyval(buf, rec)) < 0) {
//         return ret;
//     }
// 
//     return INI_OK;
// }
// 
