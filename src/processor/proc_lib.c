#include <config.h>

extern PROC_T g_proc_null;
PROC_T *g_processors[PROC_METHOD_TYPES_MAX] = {  //!< 数据处理方法列表
    &g_proc_null,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
int g_processor_type_cur = 0;           //!< 当前使用的数据处理器类型

