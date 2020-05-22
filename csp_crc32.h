/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2012 GomSpace ApS (http://www.gomspace.com)
Copyright (C) 2012 AAUSAT3 Project (http://aausat3.space.aau.dk)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _CSP_CRC32_H_
#define _CSP_CRC32_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculate checksum for a given memory area
 * @param data pointer to memory
 * @param length length of memory to do checksum on
 * @return return uint32_t checksum
 */
uint32_t csp_crc32_memory(const uint8_t * data, uint32_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _CSP_CRC32_H_ */
