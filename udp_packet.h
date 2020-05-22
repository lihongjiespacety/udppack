/**
 ********************************************************************************        
 * \brief       UDP文件打包解包相关数据结构和接口描述.
 * \details     Copyright (c) 2020,天仪研究院
 *              All rights reserved.    
 * \file        udp_packet.h 
 * \author      lhj <lihongjie@spacety.cn>
 * \version     0.1 
 * \date        2020年4月8日
 * \note        使用前请参考注释.\n
 *              依赖:.
 * \since       lhj      2020-4-8     0.1    新建 
 * \par 修订记录
 * - 2020年4月8日 初始版本
 * \par 资源说明
 * - RAM:              
 * - ROM:
 ********************************************************************************
 */

#ifndef UDP_PACKET_H
#define UDP_PACKET_H

/********************************************************************************    
 *                                                                              *
 *                             数据结构描述                                      *
 *                                                                              *
 *******************************************************************************/

#define UDP_PACKET_SIZE (1024-12)  /**< 一包数据大小 */
#define UDP_PACKET_BAKNUM 3        /**< 写份数 */
#define UDP_PACKET_MAGIC1  0xA1    /**< 包头标志 */
#define UDP_PACKET_MAGIC2  0xB2    /**< 包头标志 */
#define UDP_PACKET_MAGIC3  0x3C    /**< 包头标志 */
#define UDP_PACKET_MAGIC4  0x4D    /**< 包头标志 */

/**
 * \struct udp_file_pack_st
 * 数据包格式结构体.
 */
typedef struct __attribute__((__packed__))
{
	uint8_t  magic_au8[4];    /* 包标志 A1B23C4D */
	uint32_t index_u32;       /* 包索引从0开始    */ 
	uint8_t  buff_au8[UDP_PACKET_SIZE];  /* 一包数据UDP协议一包数据为1k 所以本结构体总体为1k PACKET_SIZE为1k减去标志 索引和校验      */
	uint32_t crc_u32;         /* 一包数据crc校验包括magic_au8到buff_au8区域  */
}udp_file_pack_st;

/********************************************************************************    
 *                                                                              *
 *                             接口函数描述                                      *
 *                                                                              *
 *******************************************************************************/

/**
 *****************************************************************************
 * \fn          int32_t udp_file_pack(char* srcpath, char* despath)
 * \brief       打包文件
 * \note        .
 * \param[in]   srcpath 源文件
 * \param[out]  despath 保存为文件
 * \retval      1 - 失败
 * \retval      0 - 成功
 *****************************************************************************
 */
int32_t udp_file_pack(char* srcpath, char* despath);
/**
 *****************************************************************************
 * \fn          int32_t udp_file_unpack(char* srcpath, char* despath)
 * \brief       解包文件
 * \note        .
 * \param[in]   srcpath 源文件
 * \param[out]  despath 保存为文件
 * \retval      1 - 失败
 * \retval      0 - 成功
 *****************************************************************************
 */
int32_t udp_file_unpack(char* srcpath, char* despath);

#endif  /* UDP_PACKET_H */