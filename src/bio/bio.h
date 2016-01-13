#ifndef __BIO_H__
#define __BIO_H__

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \block:      Base I/O control
 * @{ */
/**
 * \brief       接口初始化函数
 * \param       io          NULL:使用默认接口; ~:使用自定义接口
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_INIT_T)(void * io);
typedef void * (* BIO_CHECK)(void);
typedef struct {                        //!< 接口初始化列表结构体
    char * type;                        //!< 接口描述
    void * io;                          //!< 指定的接口结构体指针
    BIO_INIT_T init;                    //!< 接口初始化
    BIO_CHECK check;                    //!< 接口初始化结果
    unsigned long desc_ofs;             //!< 接口类型描述(结构体偏移)
}BIO_INITLIST_T;
/**
 * \brief       基本接口初始化和检查
 * \return      0: Success      <0: Error
 */
int bio_init(void);
/**
 * \brief       调试模块基本接口初始化(暂未)
 * \return      0: Success      <0: Error
 */
int bio_debug_init(void);
/** @} */

/**
 * \block:      Input/Output
 * @{ */
/**
 * \brief       输出接口函数类型
 * \param       buf         输出数据的缓存
 * \param       len         输出长度
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_OUTPUT_T)(char * buf, int len);
/**
 * \brief       输入接口函数类型
 * \param       buf         获取输入数据的缓存
 * \param       len         缓存大小
 * \return      0:Success   <0:Error
 * \detail      输入过程不立即返回，直到收到回车才退出
 */
typedef int (* BIO_FUNC_INPUT_T)(char * buf, int len);
typedef struct {                        //!< 输入输出接口配置结构体
    char * desc;                        //!< 接口类型描述
    BIO_FUNC_OUTPUT_T output;           //!< 打开文件接口函数
    BIO_FUNC_INPUT_T input;             //!< 关闭文件接口函数
}BIO_IO_T;
/**
 * \brief       初始化接口
 * \param       p           自定义接口结构体指针
 * \return      0:Success   <0:Error
 */
int bio_io_init(void * p);
/**
 * \brief       获取输入输出接口结构体
 * \return      接口结构体指针
 */
BIO_IO_T * bio_io(void);
/** @} */
/**
 * \block:      文件操作
 * @{ */
/**
 * \brief       打开文件接口函数类型
 * \param       path        文件名
 * \param       mode        模式，同fopen的mode参数
 * \param [out] fp          文件指针或句柄
 * \return      0: Success      <0: Error
 * \detail      如果不支持C标准库的文件操作，则需要解析mode参数。
 *              需要判断fp是否有效，如有效需要先关闭文件。
 *              函数返回的指针或句柄由调用函数管理，需注意重入问题。
 */
typedef int (* BIO_FUNC_FOPEN_T)(char * path, const char * mode, long * fp);
/**
 * \brief       关闭文件接口函数类型
 * \param  [in] fp          文件指针或句柄
 * \return      0: Success      <0: Error
 * \detail      文件未打开返回0 
 *              关闭后需要设置fp为无效值
 */
typedef int (* BIO_FUNC_FCLOSE_T)(long * fp);
/**
 * \brief       写文件接口函数类型
 * \param  [in] buf         写入的数据
 * \param       len         长度
 * \param       fp          文件指针或句柄
 * \return      0: Success      <0: Error
 */
typedef int (* BIO_FUNC_FWRITE_T)(void * buf, int len, long fp);
/**
 * \brief       读文件接口函数类型
 * \param [out] buf         读取数据的缓存
 * \param       len         缓存大小
 * \param       fp          文件指针或句柄
 * \return      >=0: 读取长度   <0: Error
 */
typedef int (* BIO_FUNC_FREAD_T)(void * buf, int len, long fp);
/**
 * \brief       文件读写指针移动
 * \param       ofs         偏移
 * \param       whence      位置,SEEK_SET,SEEK_CUR,SEEK_END
 * \param       fp          文件指针或句柄
 * \return      >=0: 当前偏移   <0: Error
 */
typedef int (* BIO_FUNC_FSEEK_T)(long ofs, int whence, long fp);
typedef struct {                        //!< 文件操作接口配置结构体
    char * desc;                        //!< 接口类型描述
    BIO_FUNC_FOPEN_T open;              //!< 打开文件接口函数
    BIO_FUNC_FCLOSE_T close;            //!< 关闭文件接口函数
    BIO_FUNC_FWRITE_T write;            //!< 写文件接口函数
    BIO_FUNC_FREAD_T read;              //!< 读文件接口函数
    BIO_FUNC_FSEEK_T seek;              //!< 文件指针
}BIO_FCTL_T;
/**
 * \brief       初始化接口
 * \param       io           自定义接口结构体指针
 * \return      0:Success   <0:Error
 */
int bio_fctl_init(void * io);
/**
 * \brief       获取文件接口结构体
 * \return      接口结构体指针
 */
BIO_FCTL_T * bio_fctl(void);
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* __BIO_H__ */

