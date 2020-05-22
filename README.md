#编译
gcc udp_packet.c csp_crc32.c main.c -o udppack
#运行
 ./udppack
#打包
pack src.bin des.bin
#解包
unpack des.bin unpack.bin
