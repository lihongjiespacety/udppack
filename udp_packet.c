/**
 ********************************************************************************        
 * \brief       UDP文件打包解包相关数据结构和接口实现.
 * \details     Copyright (c) 2020,天仪研究院
 *              All rights reserved.    
 * \file        udp_packet.c 
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>
#include "csp_crc32.h"
#include "udp_packet.h"

/********************************************************************************    
 *                                                                              *
 *                             数据结构定义                                      *
 *                                                                              *
 *******************************************************************************/

uint8_t g_bitmap_au8[((off_t)512*1024*1024)/(UDP_PACKET_SIZE)+1];  /**< 记录已经解包的包 避免重复写文件 根据最大文件大小修改 默认最大512M*/

/********************************************************************************    
 *                                                                              *
 *                             内部函数实现                                      *
 *                                                                              *
 *******************************************************************************/

/**
 ********************************************************************************
 * \fn          static void bitmap_clr(void)
 * \brief       清除.
 * \note        . 
 ********************************************************************************
 */
static void bitmap_clr(void)
{
    memset(g_bitmap_au8,0,sizeof(g_bitmap_au8));
}

/**
 ********************************************************************************
 * \fn          static bitmap_set(off_t index)
 * \brief       设置指定索引对应的bit为已经解包.
 * \param[in]   index 索引
 * \note        . 
 ********************************************************************************
 */
static void bitmap_set(off_t index)
{
    uint32_t byteindex;
    uint8_t bitindex;
    byteindex = index/8;
    bitindex = index%8;
    if(byteindex<sizeof(g_bitmap_au8))
    {
    g_bitmap_au8[byteindex] = g_bitmap_au8[byteindex] | ((uint8_t)1<<bitindex);
    }
    else
    {
        /*溢出不写*/
    }
}

/**
 ********************************************************************************
 * \fn          static uint8_t bitmap_get(off_t index)
 * \brief       获取指定索引对应的bit是否已经设置.
 * \param[in]   index 索引
 * \note        . 
 * \return      1已经设置 0未设置
 ********************************************************************************
 */
static uint8_t bitmap_get(off_t index)
{
    uint32_t byteindex;
    uint8_t bitindex;
    byteindex = index/8;
    bitindex = index%8;
    if(byteindex<sizeof(g_bitmap_au8))
    {
    if(g_bitmap_au8[byteindex] & ((uint8_t)1<<bitindex))
    {
        return 1;
    }
    else
    {
        return 0;
    }
    }
    else
    {
        /*溢出总是返回未设置 保证判断该包是否已经写入时认为未设置直接写入文件*/
        return 0;
    }
}

/**
 ********************************************************************************
 * \fn          off_t uint32_t get_file_size(char *path)
 * \brief       获取文件大小.
 * \param[in]   path 文件路径名
 * \note        . 
 * \retval      off_t 文件大小
 ********************************************************************************
 */
static off_t get_file_size(char *path)
{
	struct stat statbuff;
	if(stat(path, &statbuff) >= 0)
    {
		return statbuff.st_size;
	}
    else
    {
	    return -1;
    }
}

/**
 *****************************************************************************
 * \fn          static int32_t udp_file_packethandle(uint8_t* packet,uint32_t* index)
 * \brief       处理一包数据 
 *              如果有一包完整正确的包则返回0 且index返回0
 *              如果没有一包完整正确的数据则 删除包头前面的数据把包头一定到最开始处 
 *              且index返回处理后包中数据个数.
 * \note        .
 * \param[in]   packet 包数据 必须是udp_file_pack_st大小缓冲区
 * \param[out]  index 处理后包中有效数据个数
 * \retval      1 - 失败
 * \retval      0 - 成功
 *****************************************************************************
 */
static int32_t udp_file_packethandle(uint8_t* packet,uint32_t* index)
{
    /*首先查找包头标志 将包头标志和之后的数据移动到包头*/
    uint8_t* p;
    uint32_t i;
    uint32_t tomove=0; /*数据需要移动的个数*/
    int32_t res;
    p = packet;
    for(i=0;i<sizeof(udp_file_pack_st)-3;i++)
    {
        if((p[0] != UDP_PACKET_MAGIC1) || (p[1] != UDP_PACKET_MAGIC2) || (p[2] != UDP_PACKET_MAGIC3) || (p[3] != UDP_PACKET_MAGIC4))
        {
            p++;
            continue;  
        }
        else
        {
            break;
	}
    }
    if(p == packet)
    {
        /*标志已经在包头 计算校验*/
        if(((udp_file_pack_st*)packet)->crc_u32 == csp_crc32_memory((const uint8_t *)packet, UDP_PACKET_SIZE+8))
        {
            res = 0;
            tomove = 0;
            *index = 0;
        }
        else
        {
            res = -1;
            /*校验错误 删除包标志 后面的数据前移*/
            tomove= 4;
            *index = sizeof(udp_file_pack_st) - tomove;
        }
    }
    else
    {
        /*标志不在包头 继续 等待下一次处理*/
        res = -1;
        tomove= p -packet;
        *index = sizeof(udp_file_pack_st) - tomove;
    }
    if(tomove)
    {
        /*需要往前移动tomove个数据*/
        for(i=0;i<sizeof(udp_file_pack_st)-tomove;i++)
        {
            packet[i] = packet[i+tomove];
        }
    }
    return res;
}

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
int32_t udp_file_pack(char* srcpath, char* despath)
{
    off_t    filelen = 0;     /*文件长度*/
    off_t    roffset = 0;   
    off_t    woffset = 0;  
    uint32_t packet_num = 0;  /*包数*/
    uint32_t last_num = 0;    /*最后一包数*/
    uint32_t i = 0;
    uint32_t j = 0;
    int32_t res = 0;
    uint32_t toreadlen = 0;
    FILE* rfile = NULL;   
    FILE* wfile = NULL;   
    udp_file_pack_st buff;

    buff.magic_au8[0] = 0xA1;
    buff.magic_au8[1] = 0xB2;
    buff.magic_au8[2] = 0x3C;
    buff.magic_au8[3] = 0x4D;

    /*获取源文件大小*/   
    filelen = get_file_size(srcpath);
    if(filelen > 0)
    {
        packet_num = filelen/UDP_PACKET_SIZE;
        last_num = filelen%UDP_PACKET_SIZE;
    }

    /*打开文件*/
    rfile = fopen(srcpath,"rb");
    if(rfile == NULL)
    {
        return -1;
    }

    wfile = fopen(despath,"wb+");
    if(wfile == NULL)
    {
        fclose(rfile);
        return -1;
    }

    /*写*/
    for(i=0;i<packet_num+1;i++)
    {
        if(packet_num==0)
        {
            toreadlen = last_num;
            memset(buff.buff_au8,0xFF,UDP_PACKET_SIZE);
        }
        else
        {
            if(i == packet_num)
            {
                toreadlen = last_num;
                memset(buff.buff_au8,0xFF,UDP_PACKET_SIZE);
            }
            else
            {
                toreadlen = UDP_PACKET_SIZE;
            }
        }
        if(toreadlen==0)
        {
            break;
        }
        /*读*/
        if(0 != fseek(rfile,roffset,SEEK_SET))
        {
            res = -1;
            break;
        }
        if(toreadlen != fread(buff.buff_au8,1,toreadlen,rfile))
        {
            res = -1;
            break;
        }

        /*写*/
        buff.index_u32 = i;
        buff.crc_u32 = csp_crc32_memory((const uint8_t *)&buff, UDP_PACKET_SIZE+8);
        for(j=0;j<UDP_PACKET_BAKNUM;j++)
        {
            if(0 != fseek(wfile,woffset+(j*(packet_num+1)*sizeof(buff)),SEEK_SET))
            {
                res = -1;
                break;
            }

            if(sizeof(buff) != fwrite(&buff,1,sizeof(buff),wfile))
            {
                res = -1;
                break;
            }

            if(res != 0)
            {
                break;
            }
        }
        printf("%10d/%10d",i,packet_num);
        printf("\r");
        roffset += UDP_PACKET_SIZE;
        woffset += sizeof(udp_file_pack_st);
    }
    fclose(rfile);
    fclose(wfile);     
    printf("\r\n");
    return res;
}

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
int32_t udp_file_unpack(char* srcpath, char* despath)
{
    off_t    filelen = 0;     /*文件长度*/
    off_t    roffset = 0;   
    int32_t res = 0;
    int32_t index = 0;
    FILE* rfile = NULL;   
    FILE* wfile = NULL;   
    uint32_t toread;   /*本次需要读取该字节数 才能填满缓冲区*/
    uint8_t buff[sizeof(udp_file_pack_st)];
    uint32_t buffindex=0;  /*缓冲区当前数据个数*/
    /*获取源文件大小*/   
    filelen = get_file_size(srcpath);
    if(filelen < 0)
    {
        return -1;
    }
    /*打开文件*/
    rfile = fopen(srcpath,"rb");
    if(rfile == NULL)
    {
        return -1;
    }

    wfile = fopen(despath,"wb+");
    if(wfile == NULL)
    {
        fclose(rfile);
        return -1;
    }

    /*写*/
    bitmap_clr();
    while(roffset<filelen)
    {
        /*读*/
        if(0 != fseek(rfile,roffset,SEEK_SET))
        {
            res = -1;
            break;
        }

        /*读指定数据填充到缓冲区 填充满*/
        toread = sizeof(udp_file_pack_st)-buffindex;  /*还需要读toread可以填充满缓冲区*/
        if(toread != fread(&buff[buffindex],1,toread,rfile))
        {
            res = -1;
            break;
        }
        else
        {
            /*处理一包数据*/
            if(0==udp_file_packethandle(buff,&buffindex))
        {
                /*有一包有效数据 写入文件*/
                index = ((udp_file_pack_st*)buff)->index_u32;
                /*如果之前没有该包数据 写入*/
        if(bitmap_get(index) == 0)
        {
                    if(0 != fseek(wfile,index*UDP_PACKET_SIZE,SEEK_SET))
            {
                res = -1;
                break;
            }
                    if(UDP_PACKET_SIZE != fwrite(((udp_file_pack_st*)buff)->buff_au8,1,UDP_PACKET_SIZE,wfile))
            {
                res = -1;
                break;
            }
            bitmap_set(index);
                    printf("%10ld/%10ld(%10d)",roffset,filelen,index);
                    printf("\r\n");
                }
            }
            else
            {
                /*没有一包有效数据 继续*/
            }
            roffset += toread;
        }
    }
    fclose(rfile);
    fclose(wfile);    
    printf("\r\n");
    return res;
}