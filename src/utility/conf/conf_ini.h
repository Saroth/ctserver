#ifndef __INI_H__
#define __INI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef enum {                          //!< 返回值定义
    INI_OK      = 0x0000,               //!< 成功
    INI_SEC     = 0x0040,               //!< 读到节
    INI_END     = 0x0041,               //!< 文件结束
    INI_SKIP    = 0x0042,               //!< 无效数据，跳过
    INI_ERROR   = 0x0043,               //!< 错误
    INI_WARNING = 0x0044,               //!< 警告，不可解析的数据
}CONF_INI_RET_E;

typedef struct {                        //!< 配置记录结构体(单条键值
    char * section;                     //!< 节
    char * key;                         //!< 键
    char * value;                       //!< 值
    int len;                            //!< 值长(用于申请存放空间大小)
}CONF_INI_T;

/**
 * \brief       修剪字符串，删除首尾不可见字符，传入字符串将被修改
 * \param       buf     字符缓存
 * \return      >=0:    裁剪后的可见字符长度
 */
int ini_str_trim(char * buf);

int str_segment(char * bufin, char * bufout, char symbol, int segnum);
// 
// /*
//  * 解析ini格式文件
//  * @In:     fp:         文件指针，由fopen函数返回
//  *          KeyVal:     键值结构体，类型|ProFmt_t|
//  * @Ret:    ini_ret_t
//  *****************************************************************************/
// int ini_GetRecord(FILE * fp, ini_cfg_t * rec, char * presec);

#ifdef __cplusplus
}
#endif

#endif /* __INI_H__ */

