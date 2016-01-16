#ifndef __CRC32_H__
#define __CRC32_H__

#define CRC32_GENTBL    0               //!< 1:生成列表(较慢) 0:查表(占空间)

#define CRC32_POLY      0xEDB88320
#define CRC32_INIT      0xFFFFFFFF
#define CRC32_XOROUT    0xFFFFFFFF

/**
 * \brief       CRC初始化
 * \param [out] crc         int指针
 */
void crc32_init(unsigned int * crc);
/**
 * \brief       累加计算buffer的CRC
 * \param [out] crc         int指针
 * \param       buffer      数据
 * \param       size        数据长度
 */
void crc32_update(unsigned int * crc, unsigned char * buffer, unsigned int size);
/**
 * \brief       结束CRC运算
 * \param [out] crc         int指针
 */
void crc32_finish(unsigned int * crc);

/**
 * \brief       计算buffer的CRC并返回
 * \param       buffer      数据
 * \param       size        数据长度
 * \return      CRC校验码
 */
unsigned int crc32_calc(unsigned char * buffer, unsigned int size);

#endif /* __CRC32_H__ */

