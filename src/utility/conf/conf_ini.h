#ifndef __CONF_INI_H__
#define __CONF_INI_H__

#include <config.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONF_INI_CHAR_SECTION_HEAD '['  //!< 节名前标记符
#define CONF_INI_CHAR_SECTION_END  ']'  //!< 节名后标记符
#define CONF_INI_CHAR_ASSIGN    '='     //!< 赋值符
#define CONF_INI_CHARS_STRING   "'\"`"  //!< 字符串标记符。在最外围配对标记符
                                        //!<    之间的标记符都无效。多种符号
                                        //!<    可用，以防止与有效数据冲突。
                                        //!<    必须要有配对符，否则只作为普通
                                        //!<    字符处理
#define CONF_INI_CHAR_COMMENT   ';'     //!< 注释符，处于配对字符串符之间时无效
#define CONF_INI_CHAR_ESCAPE    '\\'    //!< 转义符，其后的一个字符为普通字符
                                        //!<    处于配对字符串符之间时无效

typedef enum {                          //!< 返回值定义
    CONF_INI_RET = ERR_CODE_CONF_INI,   //!< 起始码
    CONF_INI_RET_READ_ERR,              //!< 读取文件错误
    CONF_INI_RET_MEM,                   //!< 内存申请错误
    CONF_INI_RET_PARAM_ERR,             //!< 参数错误或超出范围
    CONF_INI_RET_UNKNOWN_SEC,           //!< 未知节名
    CONF_INI_RET_UNKNOWN_KEY,           //!< 未知键名
}CONF_INI_RET_E;
typedef struct {                        //!< 键信息表结构体
    char * name;                        //!< 键名，完全匹配
    unsigned int ofs;                   //!< 键表结构体中的对应成员偏移
}CONF_INI_KEYTBL_T;
typedef struct {                        //!< 节信息表结构体
    char * name;                        //!< 节名，匹配部分
    struct list_head * list;            //!< 数据存储链表，用于存储多组键表数据
    CONF_INI_KEYTBL_T * keytbl;         //!< 键信息表指针
    unsigned int keytbl_keynum;         //!< 键信息表键数
    unsigned int keytblst_size;         //!< 键表结构体大小
    unsigned int keytblst_ptr_ofs;      //!< 键表结构体中的链表节点偏移
}CONF_INI_SECTBL_T;
typedef struct {                        //!< 配置记录结构体(单条键值
    char * sec;                         //!< 节
    char * key;                         //!< 键
    char * val;                         //!< 值
    unsigned int len;                   //!< 值长(用于申请存放空间大小)
}CONF_INI_RECORD_T;

/**
 * \brief       加载配置文件
 * \param       fp          文件句柄
 * \param       tbls        节信息表结构体(数组)指针
 * \param       num         节信息表结构体个数
 * \return      0:Success   <0:Error
 * \detail      文件需要在外部打开，并传入句柄。
 *              读取所有节信息表匹配的配置信息，并装载到各个链表中。
 *              可连续加载多个文件。
 *              每行不能超过4096字节，超出部分会被忽略。
 */
int conf_ini_load(long fp, CONF_INI_SECTBL_T * tbls, unsigned int num);
/**
 * \brief       写入多个节表中的所有键表数据到配置文件
 * \param       fp          文件句柄
 * \param       tbl         节信息表结构体(数组)指针
 * \param       num         节信息表结构体个数
 * \return      0:Success   <0:Error
 * \detail      文件需要在外部打开，并传入句柄
 *              通过num控制，可以实现将配置分别写入到多个文件
 */
int conf_ini_save(long fp, CONF_INI_SECTBL_T * tbl, unsigned int num);
/**
 * \brief       清除某个表加载的数据
 * \param       tbl         节信息表结构体指针
 * \return      0:Success   <0:Error
 */
int conf_ini_clean(CONF_INI_SECTBL_T * tbl);
/**
 * \brief       获取某类型的节的链表中，某个结构体指针
 * \param       tbl         节信息表结构体指针
 * \param       idx         结构体下标
 * \param [out] entry       结构体指针
 * \return      0:Success   <0:Error
 */
int conf_ini_get_entry(CONF_INI_SECTBL_T * tbl, unsigned int idx, void ** entry);
/**
 * \brief       获取某类型的节的链表结构体个数
 * \param       tbl         节信息表结构体指针
 * \return      0:Success   <0:Error
 */
int conf_ini_get_size(CONF_INI_SECTBL_T * tbl);
#ifdef __cplusplus
}
#endif

#endif /* __CONF_INI_H__ */

