#include <config.h>

typedef struct {                        //!< 大块结构体
    char * b[BUFFER_BLOCK_NUM];         //!< 块指针数组
}BUF_BIG_BLOCK_T;
typedef struct {                        //!< 缓存结构体
    unsigned int limit;                 //!< 缓存上限，为BUFFER_BLOCK_SIZE倍数
    long long count;                    //!< 数据量计数
    BUF_BIG_BLOCK_T * bb[BUFFER_BIG_BLOCK_NUM]; //!< 大块指针数组
}BUF_T;




