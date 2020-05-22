/**
 ********************************************************************************        
 * \brief       UDP�ļ�������������ݽṹ�ͽӿ�ʵ��.
 * \details     Copyright (c) 2020,�����о�Ժ
 *              All rights reserved.    
 * \file        udp_packet.c 
 * \author      lhj <lihongjie@spacety.cn>
 * \version     0.1 
 * \date        2020��4��8��
 * \note        ʹ��ǰ��ο�ע��.\n
 *              ����:.
 * \since       lhj      2020-4-8     0.1    �½� 
 * \par �޶���¼
 * - 2020��4��8�� ��ʼ�汾
 * \par ��Դ˵��
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
 *                             ���ݽṹ����                                      *
 *                                                                              *
 *******************************************************************************/

uint8_t g_bitmap_au8[((off_t)512*1024*1024)/(UDP_PACKET_SIZE)+1];  /**< ��¼�Ѿ�����İ� �����ظ�д�ļ� ��������ļ���С�޸� Ĭ�����512M*/

/********************************************************************************    
 *                                                                              *
 *                             �ڲ�����ʵ��                                      *
 *                                                                              *
 *******************************************************************************/

/**
 ********************************************************************************
 * \fn          static void bitmap_clr(void)
 * \brief       ���.
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
 * \brief       ����ָ��������Ӧ��bitΪ�Ѿ����.
 * \param[in]   index ����
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
        /*�����д*/
    }
}

/**
 ********************************************************************************
 * \fn          static uint8_t bitmap_get(off_t index)
 * \brief       ��ȡָ��������Ӧ��bit�Ƿ��Ѿ�����.
 * \param[in]   index ����
 * \note        . 
 * \return      1�Ѿ����� 0δ����
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
        /*������Ƿ���δ���� ��֤�жϸð��Ƿ��Ѿ�д��ʱ��Ϊδ����ֱ��д���ļ�*/
        return 0;
    }
}

/**
 ********************************************************************************
 * \fn          off_t uint32_t get_file_size(char *path)
 * \brief       ��ȡ�ļ���С.
 * \param[in]   path �ļ�·����
 * \note        . 
 * \retval      off_t �ļ���С
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
 * \brief       ����һ������ 
 *              �����һ��������ȷ�İ��򷵻�0 ��index����0
 *              ���û��һ��������ȷ�������� ɾ����ͷǰ������ݰѰ�ͷһ�����ʼ�� 
 *              ��index���ش����������ݸ���.
 * \note        .
 * \param[in]   packet ������ ������udp_file_pack_st��С������
 * \param[out]  index ����������Ч���ݸ���
 * \retval      1 - ʧ��
 * \retval      0 - �ɹ�
 *****************************************************************************
 */
static int32_t udp_file_packethandle(uint8_t* packet,uint32_t* index)
{
    /*���Ȳ��Ұ�ͷ��־ ����ͷ��־��֮��������ƶ�����ͷ*/
    uint8_t* p;
    uint32_t i;
    uint32_t tomove=0; /*������Ҫ�ƶ��ĸ���*/
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
        /*��־�Ѿ��ڰ�ͷ ����У��*/
        if(((udp_file_pack_st*)packet)->crc_u32 == csp_crc32_memory((const uint8_t *)packet, UDP_PACKET_SIZE+8))
        {
            res = 0;
            tomove = 0;
            *index = 0;
        }
        else
        {
            res = -1;
            /*У����� ɾ������־ ���������ǰ��*/
            tomove= 4;
            *index = sizeof(udp_file_pack_st) - tomove;
        }
    }
    else
    {
        /*��־���ڰ�ͷ ���� �ȴ���һ�δ���*/
        res = -1;
        tomove= p -packet;
        *index = sizeof(udp_file_pack_st) - tomove;
    }
    if(tomove)
    {
        /*��Ҫ��ǰ�ƶ�tomove������*/
        for(i=0;i<sizeof(udp_file_pack_st)-tomove;i++)
        {
            packet[i] = packet[i+tomove];
        }
    }
    return res;
}

/********************************************************************************    
 *                                                                              *
 *                             �ӿں�������                                      *
 *                                                                              *
 *******************************************************************************/
/**
 *****************************************************************************
 * \fn          int32_t udp_file_pack(char* srcpath, char* despath)
 * \brief       ����ļ�
 * \note        .
 * \param[in]   srcpath Դ�ļ�
 * \param[out]  despath ����Ϊ�ļ�
 * \retval      1 - ʧ��
 * \retval      0 - �ɹ�
 *****************************************************************************
 */
int32_t udp_file_pack(char* srcpath, char* despath)
{
    off_t    filelen = 0;     /*�ļ�����*/
    off_t    roffset = 0;   
    off_t    woffset = 0;  
    uint32_t packet_num = 0;  /*����*/
    uint32_t last_num = 0;    /*���һ����*/
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

    /*��ȡԴ�ļ���С*/   
    filelen = get_file_size(srcpath);
    if(filelen > 0)
    {
        packet_num = filelen/UDP_PACKET_SIZE;
        last_num = filelen%UDP_PACKET_SIZE;
    }

    /*���ļ�*/
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

    /*д*/
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
        /*��*/
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

        /*д*/
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
 * \brief       ����ļ�
 * \note        .
 * \param[in]   srcpath Դ�ļ�
 * \param[out]  despath ����Ϊ�ļ�
 * \retval      1 - ʧ��
 * \retval      0 - �ɹ�
 *****************************************************************************
 */
int32_t udp_file_unpack(char* srcpath, char* despath)
{
    off_t    filelen = 0;     /*�ļ�����*/
    off_t    roffset = 0;   
    int32_t res = 0;
    int32_t index = 0;
    FILE* rfile = NULL;   
    FILE* wfile = NULL;   
    uint32_t toread;   /*������Ҫ��ȡ���ֽ��� ��������������*/
    uint8_t buff[sizeof(udp_file_pack_st)];
    uint32_t buffindex=0;  /*��������ǰ���ݸ���*/
    /*��ȡԴ�ļ���С*/   
    filelen = get_file_size(srcpath);
    if(filelen < 0)
    {
        return -1;
    }
    /*���ļ�*/
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

    /*д*/
    bitmap_clr();
    while(roffset<filelen)
    {
        /*��*/
        if(0 != fseek(rfile,roffset,SEEK_SET))
        {
            res = -1;
            break;
        }

        /*��ָ��������䵽������ �����*/
        toread = sizeof(udp_file_pack_st)-buffindex;  /*����Ҫ��toread���������������*/
        if(toread != fread(&buff[buffindex],1,toread,rfile))
        {
            res = -1;
            break;
        }
        else
        {
            /*����һ������*/
            if(0==udp_file_packethandle(buff,&buffindex))
        {
                /*��һ����Ч���� д���ļ�*/
                index = ((udp_file_pack_st*)buff)->index_u32;
                /*���֮ǰû�иð����� д��*/
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
                /*û��һ����Ч���� ����*/
            }
            roffset += toread;
        }
    }
    fclose(rfile);
    fclose(wfile);    
    printf("\r\n");
    return res;
}