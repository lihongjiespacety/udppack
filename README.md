#����
gcc udp_packet.c csp_crc32.c main.c -o udppack
#����
 ./udppack
#���
pack src.bin des.bin
#���
unpack des.bin unpack.bin
