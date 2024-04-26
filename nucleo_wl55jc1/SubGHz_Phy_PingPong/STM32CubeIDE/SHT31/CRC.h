/*
 * CRC.h
 *
 *  Created on: Apr 3, 2024
 *      Author: user
 */

#ifndef SHT31_CRC_H_
#define SHT31_CRC_H_

#include <stdint.h>

// CRC 테이블 정의
static const uint32_t crc_table[256];

// CRC 계산 함수
uint32_t CalcCRC(uint8_t * pData, uint32_t DataLength);


#endif /* SHT31_CRC_H_ */
