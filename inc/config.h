
/**
 * \block:      System
 * @{ */
// #define CFG_SYS_WIN                     //!< Windows平台
#define CFG_SYS_UNIX                    //!< 类Unix平台

#define PROGRAM_NAME        "TLS Server"
/*
 * 版本号结构:
 * MMNNPP00
 * Major version | Minor version | Patch version
 */
#define PROGRAM_VERSION     0x00010000
/** @} */
/**
 * \block:      DEBUG
 * @{ */
#define DS_DEBUG_MAIN 1                 //!< 调试总开关

/// 主要信息输入输出
#define MIO             DBG_INFO
/// 主模块调试
#define DS_MAIN         (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_MAIN_ERR     (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 标准接口模块调试
#define DS_BIO          0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_BIO_ERR      (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 信号量模块调试
#define DS_SEM          0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_SEM_ERR      (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 共享内存模块调试
#define DS_SHM          0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_SHM_ERR      (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 配置文件解析模块调试
#define DS_CONF_INI     0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_CONF_INI_MEM 0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_CONF_INI_ERR (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 缓存模块调试
#define DS_BUF_QUEUE        0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_BUF_QUEUE_ERR    (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 池模块调试
#define DS_POOL         0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_POOL_ERR     (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 服务端套接口模块调试
#define DS_SERVER       (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_SERVER_ERR   (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 模块测试输出
#define DS_TM           (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/** @} */
/**
 * \block:      Error code segments
 * @{ */
#define ERR_CODE_BIO            -9900
#define ERR_CODE_CONF_INI       -9800
#define ERR_CODE_BUFFER         -9700
#define ERR_CODE_POOL_THREAD    -9600
#define ERR_CODE_PROCESSOR      -9500
#define ERR_CODE_SOCK           -9400
#define ERR_CODE_MAIN           -5400
/** @} */
/**
 * \block:      include
 * @{ */
#include <stdio.h>
#include <string.h>
/// Base modules
#include <libdebug.h>
#include "list/list.h"
#include "../src/bio/bio.h"
#include "../src/pool/pool.h"
#include "../src/processor/processor.h"
#include "../src/sock/sock.h"
#include "../memwatch/memwatch.h"
/// Utility modules
#include "../src/utility/conf/conf_ini.h"
#include "../src/utility/crc/crc32.h"
#include "../src/utility/buffer/buffer.h"
/// Main modules
#include "../src/core/core.h"
/** @} */


