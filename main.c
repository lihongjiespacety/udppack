#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "udp_packet.h"
int main(void)
{
    char cmdstr[64];
    char src[256];
    char des[256];
    char cmd[512];
    int len;
    while (1)
    {
        /* code */
        memset(cmdstr,0,sizeof(cmdstr));
        memset(src,0,sizeof(src));
        memset(des,0,sizeof(des));
        memset(cmd,0,sizeof(cmd));
        printf("unpack/pack src des\n");
        while(scanf("%[^\r\n]",cmd)>0);
        getchar();
        len = sscanf(cmd,"%s %s %s",cmdstr,src,des);
        if(len == 3)
        {
            if(strcmp(cmdstr,"unpack") == 0)
            {
                udp_file_unpack(src, des);
            }
            else if(strcmp(cmdstr,"pack") == 0)
            {
                udp_file_pack(src, des);
            }
            else
            {

            }
        }
    }
}