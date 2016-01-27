#include <config.h>

int test_str_trim(void *p)
{
    extern int str_trim(char * buf);
    char buf[3][32] = {
        { "\t \x01\x1F\x10 test = 123 \x19\t", },
        { "中文测试", },
        { "\x02 中 文 测 试 \t  ", },
    };
    int ret;
    int i = 0;
    for(; i < 3; i++) {
        dbg_out_I(DS_TM, "String before trim:");
        dbg_dmp_HC(DS_TM, buf[i], 32);
        dbg_out_I(DS_TM, "Trim...");
        ret = str_trim(buf[i]);
        dbg_out_I(DS_TM, "ret: %d", ret);
        dbg_out_I(DS_TM, "String after trim:");
        dbg_dmp_HC(DS_TM, buf[i], 32);
    }
    return 0;
}

// 烧片文件基本信息结构体
typedef struct {
    char *Version;              // 版本号
    char *BuildTime;            // 烧片文件创建时间
    char *FileType;             // 安装包类型
    char *Process;              // 烧片流程控制

    struct list_head ptr;       // 链表节点
}PKGINFO_T;
// 分区信息结构体
//      烧片时的分区设置, 非电力系统分区;
//      暂不支持自定义分区(需要特殊驱动);
//      以下分区设置必须和环境变量中的定义相同;
typedef struct {
    char *ParNum;               // 分区序号, [0, ~)
    char *ParName;              // 分区标识
    char *ParSize;              // 分区大小
    char *Offset;               // 分区在主MTD空间的偏移量

    struct list_head ptr;       // 链表节点
}PARINFO_T;
// 烧片文件信息结构体
typedef struct{
    char *FileName;             // 烧片文件名;
    char *FileType;             // 烧片文件类型, 同时作为最终待烧写文件
                                // 的文件名, 必填, 不可重复;
    char *ParNum;               // 文件所在的分区序号, 必填;
    char *CreateMode;           // 最终生成烧片文件处理方式, 使用','分割:
                                // fdl:     解包fdl;
                                // gz:      gzip解压
                                // cmd:     通过执行CreateCmd处理;
                                // none:    不处理;
                                // env:     自动生成环境变量文件,
                                //          需要依赖某些文件;
                                // flg:     自动生成标记文件,
                                //          需要依赖某些文件;
    char *CreateCmd;            // 处理脚本;
    char *CreateData;           // 预留处理数据;
    char *Priority;             // 烧写优先级[0, 255], 未定义则默认100,
                                // 数值越低表示优先级越高;
    char *WriteMode;            // 烧片方式, 使用','分割
                                // bin:     MTD方式写到对应地址;
                                // tar:     挂载方式解压到对应路径;
                                // cmd:     挂载后执行脚本, 完成后自动卸载;
                                // 注:  bin烧写后如果再执行cmd烧写,
                                //      挂载磁盘操作可能会破坏
                                //      非文件系统磁盘的数据;
    char *WriteOffset;          // 烧写地址;
    char *WriteRange;           // 烧写范围, 未定义默认为分区大小;
    char *WriteCmd;             // 烧写命令;
    char *WriteData;            // 预留烧写数据;

    struct list_head ptr;       // 链表节点
}FILEINFO_T;
// 烧片文件基本信息表
static CONF_INI_KEYTBL_T s_pkginfo_keytbl[] = {
    { "Version",        (unsigned long)&((PKGINFO_T *)0)->Version,      },
    { "BuildTime",      (unsigned long)&((PKGINFO_T *)0)->BuildTime,    },
    { "FileType",       (unsigned long)&((PKGINFO_T *)0)->FileType,     },
    { "Process",        (unsigned long)&((PKGINFO_T *)0)->Process,      },
};
// 分区信息表
static CONF_INI_KEYTBL_T s_parinfo_keytbl[] = {
    { "ParNum",         (unsigned long)&((PARINFO_T *)0)->ParNum,       },
    { "ParName",        (unsigned long)&((PARINFO_T *)0)->ParName,      },
    { "ParSize",        (unsigned long)&((PARINFO_T *)0)->ParSize,      },
    { "ParOffset",      (unsigned long)&((PARINFO_T *)0)->Offset,       },
};
// 烧片文件信息表
static CONF_INI_KEYTBL_T s_fileinfo_keytbl[] = {
    { "FileName",       (unsigned long)&((FILEINFO_T *)0)->FileName,    },
    { "FileType",       (unsigned long)&((FILEINFO_T *)0)->FileType,    },
    { "ParNum",         (unsigned long)&((FILEINFO_T *)0)->ParNum,      },
    { "CreateMode",     (unsigned long)&((FILEINFO_T *)0)->CreateMode,  },
    { "CreateCmd",      (unsigned long)&((FILEINFO_T *)0)->CreateCmd,   },
    { "CreateData",     (unsigned long)&((FILEINFO_T *)0)->CreateData,  },
    { "Priority",       (unsigned long)&((FILEINFO_T *)0)->Priority,    },
    { "WriteMode",      (unsigned long)&((FILEINFO_T *)0)->WriteMode,   },
    { "WriteOffset",    (unsigned long)&((FILEINFO_T *)0)->WriteOffset, },
    { "WriteRange",     (unsigned long)&((FILEINFO_T *)0)->WriteRange,  },
    { "WriteCmd",       (unsigned long)&((FILEINFO_T *)0)->WriteCmd,    },
    { "WriteData",      (unsigned long)&((FILEINFO_T *)0)->WriteData,   },
};

// 配置文件信息存储链表
LIST_HEAD(s_pkginfo_list);
LIST_HEAD(s_parinfo_list);
LIST_HEAD(s_fileinfo_list);
// 配置文件信息匹配表
CONF_INI_SECTBL_T g_test_sectbl[] = {
    { "BaseInfo", &s_pkginfo_list, s_pkginfo_keytbl,
        (unsigned int)(sizeof(s_pkginfo_keytbl) / sizeof(CONF_INI_KEYTBL_T)),
        sizeof(PKGINFO_T), (unsigned long)&((PKGINFO_T *)0)->ptr,  },
    { "Par", &s_parinfo_list, s_parinfo_keytbl,
        (unsigned int)(sizeof(s_parinfo_keytbl) / sizeof(CONF_INI_KEYTBL_T)),
        sizeof(PARINFO_T), (unsigned long)&((PARINFO_T *)0)->ptr,  },
    { "File", &s_fileinfo_list, s_fileinfo_keytbl,
        (unsigned int)(sizeof(s_fileinfo_keytbl) / sizeof(CONF_INI_KEYTBL_T)),
        sizeof(FILEINFO_T), (unsigned long)&((FILEINFO_T *)0)->ptr, },
};
// 配置文件节类型序号
typedef enum {
    ST_PKG_INFO,                        // 文件基本信息
    ST_PAR_INFO,                        // 分区信息
    ST_FILE_INFO,                       // 文件信息
    ST_MAX,
}ST_SECTYPE_NUM_t;

int test_ini_parse(void *p)
{
    long fp = NULL;
    char * test_ini_file = "../test/test_ini_syntax.ini";
    dbg_out_I(DS_TM, "Open file: %s", test_ini_file);
    int ret = bio_fctl()->open(test_ini_file, "rb+", &fp);
    if(ret) {
        dbg_out_E(DS_TM, "Open file failed: %s, ret: %d", test_ini_file, ret);
        return -1;
    }
    dbg_out_I(DS_TM, "Analyze ini...");
    ret = conf_ini_load(fp, g_test_sectbl, ST_MAX);
    if(ret) {
        dbg_out_E(DS_TM, "ini load, ret: %d", ret);
        return -1;
    }
    bio_fctl()->close(&fp);

    return 0;
}
int test_ini_load(void *p)
{
    long fp = NULL;
    char * test_ini_file = "../test/test_template.ini";
    dbg_out_I(DS_TM, "Open file: %s", test_ini_file);
    int ret = bio_fctl()->open(test_ini_file, "rb+", &fp);
    if(ret) {
        dbg_out_E(DS_TM, "Open file failed: %s, ret: %d", test_ini_file, ret);
        return -1;
    }
    dbg_out_I(DS_TM, "Analyze ini...");
    ret = conf_ini_load(fp, g_test_sectbl, ST_MAX);
    if(ret) {
        dbg_out_E(DS_TM, "ini load, ret: %d", ret);
        return -1;
    }
    bio_fctl()->close(&fp);

    return 0;
}
int test_clean(void *p)
{
    int i = 0;
    CONF_INI_SECTBL_T * tbl = &g_test_sectbl[0];
    for(; i < ST_MAX; i++, tbl++) {
        conf_ini_clean(tbl);
    }
    return 0;
}
int test_size(void *p)
{
    int i = 0;
    int size;
    CONF_INI_SECTBL_T * tbl = &g_test_sectbl[0];
    for(; i < ST_MAX; i++, tbl++) {
        size = conf_ini_get_size(tbl);
        dbg_out_I(DS_TM, "Get size: %d", size);
    }
    return 0;
}
int test_getentry(void *p)
{
    int i = 0;
    CONF_INI_SECTBL_T * tbl = &g_test_sectbl[0];
    for(; i < ST_MAX; i++, tbl++) {
        int size = conf_ini_get_size(tbl);
        int j = 0;
        for(; j < size; j++) {
            void * entry = 0;
            conf_ini_get_entry(tbl, j, &entry);
            dbg_out_I(DS_TM, "Get entry %d-%d: %p", i, j, entry);
        }
    }
    return 0;
}

int test_ini(void *p)
{
    dbg_test_setlist(
        { "string trim",    NULL,   test_str_trim,  },
        { "syntax parse",   0,      test_ini_parse, },
        { "load(use nandprogrammer cfg file", 0, test_ini_load, },
        { "clean",          0,      test_clean,     },
        { "Size",           0,      test_size,      },
        { "Get entry",      0,      test_getentry,  },
        )
    return 0;
}

