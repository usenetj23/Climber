
/*
 * Copyright (c) 2017, STMicroelectronics - All Rights Reserved
 *
 * This file is part of VL53L1 Core and is dual licensed,
 * either 'STMicroelectronics
 * Proprietary license'
 * or 'BSD 3-clause "New" or "Revised" License' , at your option.
 *
 ********************************************************************************
 *
 * 'STMicroelectronics Proprietary license'
 *
 ********************************************************************************
 *
 * License terms: STMicroelectronics Proprietary in accordance with licensing
 * terms at www.st.com/sla0081
 *
 * STMicroelectronics confidential
 * Reproduction and Communication of this document is strictly prohibited unless
 * specifically authorized in writing by STMicroelectronics.
 *
 *
 ********************************************************************************
 *
 * Alternatively, VL53L1 Core may be distributed under the terms of
 * 'BSD 3-clause "New" or "Revised" License', in which case the following
 * provisions apply instead of the ones mentioned above :
 *
 ********************************************************************************
 *
 * License terms: BSD 3-clause "New" or "Revised" License.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 ********************************************************************************
 *
 */

#include "vl53l1_platform.h"
// #include "vl53l1_platform_log.h"
#include "vl53l1_api.h"

// #include "stm32xxx_hal.h"
#include <string.h>
// #include <time.h>
// #include <math.h>
#include "i2c.h"
#include "millis.h"
#include "segger_wrapper.h"

#undef I2C_READ_REG
#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr >> 1, p_reg_addr, 2, NRF_TWI_MNGR_NO_STOP), \
		NRF_TWI_MNGR_READ (addr >> 1, p_buffer, byte_cnt, 0)

#undef I2C_READ_REG_REP_STOP
#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr >> 1, p_reg_addr, 2, 0), \
		NRF_TWI_MNGR_READ (addr >> 1, p_buffer, byte_cnt, 0)

#undef I2C_WRITE
#define I2C_WRITE(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr >> 1, p_data, byte_cnt, 0)

//NRF_TWI_SENSOR_SEND_BUF_SIZE
static uint8_t buffer[256];

static int32_t _vl53l1_i2c_read_reg(uint8_t addr, uint16_t reg, uint8_t *p_data, uint32_t length) {

	uint8_t buffer[2];
	buffer[0]=(uint8_t) (reg >> 8);
	buffer[1]=(uint8_t) (reg & 0xFF);

	nrf_twi_mngr_transfer_t const xfer[] =
	{
			I2C_READ_REG_REP_STOP(addr, buffer, p_data, length)
	};

	return i2c_perform(NULL, xfer, sizeof(xfer) / sizeof(xfer[0]), NULL);
}

static int32_t _vl53l1_i2c_write_reg(uint8_t addr, uint16_t reg, uint8_t *data, uint32_t length) {

	buffer[0]=(uint8_t) ((reg >> 8) & 0xFF);
	buffer[1]=(uint8_t) (reg & 0xFF);

	memcpy(&buffer[2], data, length);

	nrf_twi_mngr_transfer_t const xfer[] =
	{
			I2C_WRITE(addr, buffer, length + 2),
	};

	return i2c_perform(NULL, xfer, sizeof(xfer) / sizeof(xfer[0]), NULL);
}

VL53L1_Error VL53L1_WriteMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count) {
	VL53L1_Error Status = VL53L1_ERROR_NONE;

	_vl53l1_i2c_write_reg(Dev->I2cDevAddr, index, pdata, count);

	return Status;
}

VL53L1_Error VL53L1_ReadMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count) {
	VL53L1_Error Status = VL53L1_ERROR_NONE;

	_vl53l1_i2c_read_reg(Dev->I2cDevAddr, index, pdata, count);

	return Status;
}

VL53L1_Error VL53L1_WrByte(VL53L1_DEV Dev, uint16_t index, uint8_t data) {
	int  status;

	status = _vl53l1_i2c_write_reg(Dev->I2cDevAddr, index, &data, 1);
	return status;
}

VL53L1_Error VL53L1_WrWord(VL53L1_DEV Dev, uint16_t index, uint16_t data) {
	int  status;
	uint8_t buffer[2];

	buffer[0] = data >> 8;
	buffer[1] = data & 0x00FF;
	status = _vl53l1_i2c_write_reg(Dev->I2cDevAddr, index, (uint8_t *)buffer, 2);
	return status;
}

VL53L1_Error VL53L1_WrDWord(VL53L1_DEV Dev, uint16_t index, uint32_t data) {
	int  status;
	uint8_t buffer[4];

	buffer[0] = (data >> 24) & 0xFF;
	buffer[1] = (data >> 16) & 0xFF;
	buffer[2] = (data >>  8) & 0xFF;
	buffer[3] = (data >>  0) & 0xFF;
	status = _vl53l1_i2c_write_reg(Dev->I2cDevAddr, index, (uint8_t *)buffer, 4);
	return status;
}

VL53L1_Error VL53L1_UpdateByte(VL53L1_DEV Dev, uint16_t index, uint8_t AndData, uint8_t OrData) {
	int  status;
	uint8_t buffer = 0;

	/* read data direct onto buffer */
	status = _vl53l1_i2c_read_reg(Dev->I2cDevAddr, index, &buffer, 1);
	if (!status)
	{
		buffer = (buffer & AndData) | OrData;
		status = _vl53l1_i2c_write_reg(Dev->I2cDevAddr, index, &buffer, (uint16_t)1);
	}
	return status;
}

VL53L1_Error VL53L1_RdByte(VL53L1_DEV Dev, uint16_t index, uint8_t *data) {
	int  status;

	status = _vl53l1_i2c_read_reg(Dev->I2cDevAddr, index, data, 1);

	if(status)
		return -1;

	return 0;
}

VL53L1_Error VL53L1_RdWord(VL53L1_DEV Dev, uint16_t index, uint16_t *data) {
	int  status;
	uint8_t buffer[2] = {0,0};

	status = _vl53l1_i2c_read_reg(Dev->I2cDevAddr, index, buffer, 2);
	if (!status)
	{
		*data = (buffer[0] << 8) + buffer[1];
	}
	return status;
}

VL53L1_Error VL53L1_RdDWord(VL53L1_DEV Dev, uint16_t index, uint32_t *data) {
	int status;
	uint8_t buffer[4] = {0,0,0,0};

	status = _vl53l1_i2c_read_reg(Dev->I2cDevAddr, index, buffer, 4);
	if(!status)
	{
		*data = ((uint32_t)buffer[0] << 24) + ((uint32_t)buffer[1] << 16) + ((uint32_t)buffer[2] << 8) + (uint32_t)buffer[3];
	}
	return status;
}

VL53L1_Error VL53L1_GetTickCount(
		uint32_t *ptick_count_ms)
{
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	*ptick_count_ms = millis();
	return status;
}

VL53L1_Error VL53L1_GetTimerFrequency(int32_t *ptimer_freq_hz)
{
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	return status;
}

extern void delay_ms(uint32_t);

VL53L1_Error VL53L1_WaitMs(VL53L1_Dev_t *pdev, int32_t wait_ms){
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	//w_task_delay(wait_ms);
	delay_ms(wait_ms);
	return status;
}

VL53L1_Error VL53L1_WaitUs(VL53L1_Dev_t *pdev, int32_t wait_us){
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	//w_task_delay(1);
	delay_ms(1);
	return status;
}

VL53L1_Error VL53L1_WaitValueMaskEx(
		VL53L1_Dev_t *pdev,
		uint32_t      timeout_ms,
		uint16_t      index,
		uint8_t       value,
		uint8_t       mask,
		uint32_t      poll_delay_ms)
{

	/*
	 * Platform implementation of WaitValueMaskEx V2WReg script command
	 *
	 * WaitValueMaskEx(
	 *          duration_ms,
	 *          index,
	 *          value,
	 *          mask,
	 *          poll_delay_ms);
	 */

	VL53L1_Error status         = VL53L1_ERROR_NONE;
	uint32_t     start_time_ms = 0;
	uint32_t     current_time_ms = 0;
	uint32_t     polling_time_ms = 0;
	uint8_t      byte_value      = 0;
	uint8_t      found           = 0;



	/* calculate time limit in absolute time */

	VL53L1_GetTickCount(&start_time_ms);

	/* remember current trace functions and temporarily disable
	 * function logging
	 */


	/* wait until value is found, timeout reached on error occurred */

	while ((status == VL53L1_ERROR_NONE) &&
			(polling_time_ms < timeout_ms) &&
			(found == 0))
	{

		if (status == VL53L1_ERROR_NONE)
			status = VL53L1_RdByte(
					pdev,
					index,
					&byte_value);

		if ((byte_value & mask) == value)
			found = 1;

		if (status == VL53L1_ERROR_NONE  &&
				found == 0 &&
				poll_delay_ms > 0)
			status = VL53L1_WaitMs(
					pdev,
					poll_delay_ms);

		/* Update polling time (Compare difference rather than absolute to
	      negate 32bit wrap around issue) */
		VL53L1_GetTickCount(&current_time_ms);
		polling_time_ms = current_time_ms - start_time_ms;

	}

	if (found == 0 && status == VL53L1_ERROR_NONE)
		status = VL53L1_ERROR_TIME_OUT;

	return status;
}


