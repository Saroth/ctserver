#ifndef __CONF_H__
#define __CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "conf_ini.h"

typedef struct {                        //!< 键名匹配表结构体
    const char *KeyName;                //!< 键名(忽略大小写)
    unsigned int Offset;                //!< 成员偏移
}KeyTable_t;

// 节名匹配表结构体
typedef struct {
    int num;                            // 序号
    const char *SectionName;            // 节名(包含)
    const KeyTable_t *keytbl;           // 键表
    unsigned int StSize;                // 结构体大小
    unsigned int KeyCount;              // 键数
    unsigned int ptr_offset;            // 结构体偏移
    struct list_head list;              // 链表
}SecTable_t;

int Prof_Init(void);
// 载入配置文件
int Prof_Load(const char * filename);
// 获取链表节点
int Prof_GetEntry(int type, int idx, void ** entry);
// 获取链表长度
int Prof_ListSize(int type);
// 清除所有链表及申请的空间
int Prof_Clean(void);

// 根据类型获取结构体数组下标
int Prof_GetTblIdx(int type);
// 发现新节, 创建结构体并添加到链表
int Prof_AddSection(ini_cfg_t * rec);
// 发现键值, 添加到结构体
int Prof_AddKeyVal(ini_cfg_t * rec);

int profile_test(int arg);

#ifdef __cplusplus
}
#endif

#endif /* __CONF_H__ */

