
/**
 * \block:      System
 * @{ */
// #define CFG_SYS_WIN                     //!< Windows平台
#define CFG_SYS_UNIX                    //!< 类Unix平台
/** @} */
/**
 * \block:      DEBUG
 * @{ */
#define DS_DEBUG_MAIN 1                 //!< 调试总开关
#include <libdebug.h>

#define DS_SVR          (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_SVR_ERR      (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)

#define DS_BIO          (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_BIO_ERR      (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)

#define DS_SEM          (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_SEM_ERR      (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)

#define DS_CONF_INI     0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_CONF_INI_MEM 0   // (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_CONF_INI_ERR (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)

#define DS_BUFFER       (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_BUFFER_ERR   (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)

/// 模块测试输出
#define DS_TM           (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/** @} */
/**
 * \block:      Error code segments
 * @{ */
#define ERR_CODE_BIO        -9900
#define ERR_CODE_CONF_INI   -9800
#define ERR_CODE_BUFFER     -9700
/** @} */

/**
 * \block:      include
 * @{ */
#include "list/list.h"

#include "../src/bio/bio.h"
#include "../src/pool/pool.h"

#include "../src/utility/conf/conf_ini.h"
#include "../src/utility/buffer/buffer.h"

#include "../src/core/core.h"
/** @} */


