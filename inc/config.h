
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

#define DS_CONF_INI     (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_CONF_INI_MEM (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
#define DS_CONF_INI_ERR (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/// 模块测试输出
#define DS_TM           (DBG_INFO | DBG_LABEL_FUNC | DBG_LABEL_LINE)
/** @} */
/**
 * \block:      Error code segments
 * @{ */
#define ERR_CODE_BIO        -9900
#define ERR_CODE_CONF_INI   -9800
/** @} */

/**
 * \block:      include
 * @{ */
#include "../src/bio/bio.h"
#include "../src/core/core.h"
#include "../src/utility/conf/conf_ini.h"
/** @} */


