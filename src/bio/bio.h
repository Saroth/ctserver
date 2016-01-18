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
 * \block:      File control
 * @{ */
/**
 * \brief       打开文件接口函数类型
 * \param       path        文件名
 * \param       mode        模式，同fopen的mode参数
 * \param [out] hdl         文件句柄指针，用于存放文件指针或句柄
 * \return      0:Success   <0:Error
 * \detail      如果不支持C标准库的文件操作，则需要解析mode参数。
 *              需要判断hdl是否有效，如有效需要先关闭文件。
 *              文件句柄由调用函数管理，需注意重入问题。
 */
typedef int (* BIO_FUNC_FOPEN_T)(char * path, const char * mode, long * hdl);
/**
 * \brief       关闭文件接口函数类型
 * \param [i/o] hdl         文件句柄指针
 * \return      0:Success   <0:Error
 * \detail      文件未打开返回0 
 *              关闭后需要设置hdl为无效值
 */
typedef int (* BIO_FUNC_FCLOSE_T)(long * hdl);
/**
 * \brief       写文件接口函数类型
 * \param  [in] buf         写入的数据
 * \param       len         长度
 * \param       hdl         文件句柄
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_FWRITE_T)(void * buf, int len, long hdl);
/**
 * \brief       读文件接口函数类型
 * \param [out] buf         读取数据的缓存
 * \param       len         缓存大小
 * \param       hdl         文件句柄
 * \return      >=0:读取长度    <0:Error
 */
typedef int (* BIO_FUNC_FREAD_T)(void * buf, int len, long hdl);
/**
 * \brief       文件读写指针移动
 * \param       ofs         偏移
 * \param       whence      位置,SEEK_SET,SEEK_CUR,SEEK_END
 * \param       hdl         文件句柄
 * \return      >=0:当前位置    <0:Error
 */
typedef int (* BIO_FUNC_FSEEK_T)(long ofs, int whence, long hdl);
/**
 * \brief       文件截断
 * \param       path        文件名
 * \param       length      截断后的文件大小
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_TRUNCATE_T)(char * path, long length);
typedef struct {                        //!< 文件操作接口配置结构体
    char * desc;                        //!< 接口类型描述
    BIO_FUNC_FOPEN_T open;              //!< 打开文件接口函数
    BIO_FUNC_FCLOSE_T close;            //!< 关闭文件接口函数
    BIO_FUNC_FWRITE_T write;            //!< 写文件接口函数
    BIO_FUNC_FREAD_T read;              //!< 读文件接口函数
    BIO_FUNC_FSEEK_T seek;              //!< 文件指针移动接口函数
    BIO_FUNC_TRUNCATE_T truncate;       //!< 文件截断接口函数
}BIO_FCTL_T;
/**
 * \brief       初始化接口
 * \param       io          自定义接口结构体指针
 * \return      0:Success   <0:Error
 */
int bio_fctl_init(void * io);
/**
 * \brief       获取文件接口结构体
 * \return      接口结构体指针
 */
BIO_FCTL_T * bio_fctl(void);
/** @} */
/**
 * \block:      Semaphore
 * @{ */
#define BIO_SEM_RETRY_WITH_NEW_ID   //!< 信号量已存在时，使用新ID重新申请
/**
 * \brief       创建信号量集
 * \param       name        用于获取key的文件名或信号量名
 * \param       num         信号量数
 * \param       val         初始值
 * \param [out] hdl         句柄指针
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_NEW)(char * name, int num, int val, long * hdl);
/**
 * \brief       删除信号量集
 * \param [i/o] hdl         句柄指针
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_DEL)(long * hdl);
/**
 * \brief       强制删除信号量集
 * \param       name        用于获取key的文件名或信号量名
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_FDEL)(char * name);
/**
 * \brief       挂起
 * \param       num         信号量编号
 * \param       wait        等待时间参数 -1:wait,0:nowait,~:waittime(s)
 * \param       hdl         句柄
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_P)(int num, long wait, long hdl);
/**
 * \brief       释放
 * \param       num         信号量编号
 * \param       hdl         句柄
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_V)(int num, long hdl);
/**
 * \brief       取值
 * \param       num         信号量编号
 * \param       hdl         句柄
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_VAL)(int num, long hdl);
/**
 * \brief       设值
 * \param       num         信号量编号
 * \param       val         值
 * \param       hdl         句柄
 * \return      0:Success   <0:Error
 */
typedef int (* BIO_FUNC_SEM_SETVAL)(int num, int val, long hdl);
typedef struct {                        //!< 信号量操作接口配置结构体
    char * desc;                        //!< 接口类型描述
    BIO_FUNC_SEM_NEW new;               //!< 创建信号量集接口函数
    BIO_FUNC_SEM_DEL del;               //!< 删除信号量集接口函数
    BIO_FUNC_SEM_P p;                   //!< 挂起信号量接口函数
    BIO_FUNC_SEM_V v;                   //!< 释放信号量接口函数
    BIO_FUNC_SEM_VAL val;               //!< 获取信号量值接口函数
    BIO_FUNC_SEM_SETVAL setval;         //!< 设置信号量值接口函数
    BIO_FUNC_SEM_FDEL fdel;             //!< 强制删除信号量集接口函数
}BIO_SEM_T;
/**
 * \brief       初始化接口
 * \param       io          自定义接口结构体指针
 * \return      0:Success   <0:Error
 */
int bio_sem_init(void * io);
/**
 * \brief       获取信号量集操作接口结构体
 * \return      接口结构体指针
 */
BIO_SEM_T * bio_sem(void);
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* __BIO_H__ */

