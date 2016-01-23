#ifndef __CORE_H__
#define __CORE_H__

typedef struct {                        //!< 选项参数结构体
    
}OPTS_T;
extern OPTS_T g_opts;                   //!< 程序选项

/**
 * \brief       程序基本初始化, Must be the first!
 * \return      0:Success   <0:Error
 */
int main_init(void);
/**
 * \brief       选项处理
 * \param       argc        参数个数
 * \param       argv[]      参数数组
 * \return      0:Success   <0:Error
 */
int option_parse(int argc, char * argv[]);
/**
 * \brief       主流程
 * \return      0:Success   <0:Error
 */
int main_process(void);

#endif /* __CORE_H__ */

