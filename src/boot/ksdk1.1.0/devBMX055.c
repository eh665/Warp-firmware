/*
	Authored 2016-2018. Phillip Stanley-Marbell. Additional contributors,
	2018-onwards, see git log.

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	*	Redistributions of source code must retain the above
		copyright notice, this list of conditions and the following
		disclaimer.

	*	Redistributions in binary form must reproduce the above
		copyright notice, this list of conditions and the following
		disclaimer in the documentation and/or other materials
		provided with the distribution.

	*	Neither the name of the author nor the names of its
		contributors may be used to endorse or promote products
		derived from this software without specific prior written
		permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdlib.h>

/*
 *	config.h needs to come first
 */
#include "config.h"

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"


extern volatile WarpI2CDeviceState	deviceBMX055accelState;
extern volatile WarpI2CDeviceState	deviceBMX055gyroState;
extern volatile WarpI2CDeviceState	deviceBMX055magState;
extern volatile uint32_t			gWarpI2cBaudRateKbps;
extern volatile uint32_t			gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t			gWarpSupplySettlingDelayMilliseconds;



/*
 *	Bosch Sensortec BMX055.
 */
void
initBMX055accel(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts)
{
	deviceBMX055accelState.i2cAddress					= i2cAddress;
	deviceBMX055accelState.operatingVoltageMillivolts	= operatingVoltageMillivolts;

	return;
}

WarpStatus
writeSensorRegisterBMX055accel(uint8_t deviceRegister, uint8_t payload)
{
	uint8_t			payloadByte[1], commandByte[1];
	i2c_status_t	status;

	if (deviceRegister > 0x3F)
	{
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
		{
		.address = deviceBMX055accelState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	commandByte[0] = deviceRegister;
	payloadByte[0] = payload;

	warpScaleSupplyVoltage(deviceBMX055accelState.operatingVoltageMillivolts);
	warpEnableI2Cpins();
	status = I2C_DRV_MasterSendDataBlocking(
		0 /* I2C instance */,
		&slave,
		commandByte,
		1,
		payloadByte,
		1,
		gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorBMX055accel(uint8_t payloadPMU_RANGE, uint8_t payloadACCD_HBW)
{
	WarpStatus status1, status2;

	warpScaleSupplyVoltage(deviceBMX055accelState.operatingVoltageMillivolts);
	status1 = writeSensorRegisterBMX055accel(kWarpSensorConfigurationRegisterBMX055accelPMU_RANGE /* register address PMU_RANGE */,
											 payloadPMU_RANGE /* payload */
	);

	status2 = writeSensorRegisterBMX055accel(kWarpSensorConfigurationRegisterBMX055accelACCD_HBW /* register address ACCD_HBW */,
											 payloadACCD_HBW /* payload */
	);
	return (status1 | status2);
}

WarpStatus
readSensorRegisterBMX055accel(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t			cmdBuf[1] = {0xFF};
	i2c_status_t	status;


	if (deviceRegister > 0x3F)
	{
		warpPrint("kWarpStatusBadDeviceCommand\n...");
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
		{
		.address = deviceBMX055accelState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	cmdBuf[0] = deviceRegister;

	warpScaleSupplyVoltage(deviceBMX055accelState.operatingVoltageMillivolts);
	warpEnableI2Cpins();
	status = I2C_DRV_MasterReceiveDataBlocking(
		0 /* I2C peripheral instance */,
		&slave,
		cmdBuf,
		1,
		(uint8_t*)deviceBMX055accelState.i2cBuffer,
		numberOfBytes,
		gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
initBMX055mag(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts)
{
	deviceBMX055magState.i2cAddress						= i2cAddress;
	deviceBMX055magState.operatingVoltageMillivolts		= operatingVoltageMillivolts;

	return;
}

WarpStatus
writeSensorRegisterBMX055mag(uint8_t deviceRegister, uint8_t payload)
{
	uint8_t			payloadByte[1], commandByte[1];
	i2c_status_t	status;

	if (deviceRegister > 0x52 || deviceRegister < 0x40)
	{
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
		{
		.address       = deviceBMX055magState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	commandByte[0] = deviceRegister;
	payloadByte[0] = payload;

	warpScaleSupplyVoltage(deviceBMX055magState.operatingVoltageMillivolts);
	warpEnableI2Cpins();
	status = I2C_DRV_MasterSendDataBlocking(
		0 /* I2C instance */,
		&slave,
		commandByte,
		1,
		payloadByte,
		1,
		gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorBMX055mag(uint8_t payloadPowerCtrl, uint8_t payloadOpMode)
{
	WarpStatus	status1, status2;

	warpScaleSupplyVoltage(deviceBMX055magState.operatingVoltageMillivolts);
	status1 = writeSensorRegisterBMX055mag(
		kWarpSensorConfigurationRegisterBMX055magPowerCtrl /* Power and operation modes, self-test, data output rate control registers */,
		payloadPowerCtrl /* payload */
	);

	status2 = writeSensorRegisterBMX055mag(
		kWarpSensorConfigurationRegisterBMX055magOpMode /* Operation mode, output data rate and self-test control register */,
		payloadOpMode /* payload */
	);
	return (status1 | status2);
}

WarpStatus
readSensorRegisterBMX055mag(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t			cmdBuf[1] = {0xFF};
	i2c_status_t	status;


	if (deviceRegister > 0x52 || deviceRegister < 0x40)
	{
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
		{
		.address       = deviceBMX055magState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	cmdBuf[0] = deviceRegister;

	warpScaleSupplyVoltage(deviceBMX055magState.operatingVoltageMillivolts);
	warpEnableI2Cpins();
	status = I2C_DRV_MasterReceiveDataBlocking(
		0 /* I2C peripheral instance */,
		&slave,
		cmdBuf,
		1,
		(uint8_t*)deviceBMX055magState.i2cBuffer,
		numberOfBytes,
		gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
initBMX055gyro(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts)
{
	deviceBMX055gyroState.i2cAddress					= i2cAddress;
	deviceBMX055gyroState.operatingVoltageMillivolts	= operatingVoltageMillivolts;

	return;
}

WarpStatus
writeSensorRegisterBMX055gyro(uint8_t deviceRegister, uint8_t payload)
{
	uint8_t			payloadByte[1], commandByte[1];
	i2c_status_t	status;

	if (deviceRegister > 0x3F)
	{
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
		{
		.address       = deviceBMX055gyroState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	commandByte[0] = deviceRegister;
	payloadByte[0] = payload;

	warpScaleSupplyVoltage(deviceBMX055gyroState.operatingVoltageMillivolts);
	warpEnableI2Cpins();
	status = I2C_DRV_MasterSendDataBlocking(
		0 /* I2C instance */,
		&slave,
		commandByte,
		1,
		payloadByte,
		1,
		gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorBMX055gyro(uint8_t payloadRANGE, uint8_t payloadBW, uint8_t payloadLPM1, uint8_t payloadRATE_HBW)
{
	WarpStatus	status1, status2, status3, status4;


	warpScaleSupplyVoltage(deviceBMX055gyroState.operatingVoltageMillivolts);

	status1 = writeSensorRegisterBMX055gyro(kWarpSensorConfigurationRegisterBMX055gyroRANGE /* register address RANGE */,
											payloadRANGE /* payload */
	);

	status2 = writeSensorRegisterBMX055gyro(kWarpSensorConfigurationRegisterBMX055gyroBW /* register address filter bandwidth */,
											payloadBW /* payload */
	);

	status3 = writeSensorRegisterBMX055gyro(kWarpSensorConfigurationRegisterBMX055gyroLPM1 /* register address LPM1 */,
											payloadLPM1 /* payload */
	);

	status4 = writeSensorRegisterBMX055gyro(kWarpSensorConfigurationRegisterBMX055gyroRATE_HBW /* register address RATE_HBW */,
											payloadRATE_HBW /* payload */
	);
	return (status1 | status2 | status3 | status4);
}

WarpStatus
readSensorRegisterBMX055gyro(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t			cmdBuf[1] = {0xFF};
	i2c_status_t	status;


	USED(numberOfBytes);
	if (deviceRegister > 0x3F)
	{
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
		{
		.address       = deviceBMX055gyroState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	cmdBuf[0] = deviceRegister;

	warpScaleSupplyVoltage(deviceBMX055gyroState.operatingVoltageMillivolts);
	warpEnableI2Cpins();
	status = I2C_DRV_MasterReceiveDataBlocking(
		0 /* I2C peripheral instance */,
		&slave,
		cmdBuf,
		1,
		(uint8_t*)deviceBMX055gyroState.i2cBuffer,
		numberOfBytes,
		gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
printSensorDataBMX055accel(bool hexModeFlag)
{
	uint16_t	readSensorRegisterValueLSB;
	uint16_t	readSensorRegisterValueMSB;
	int16_t		readSensorRegisterValueCombined;
	WarpStatus	i2cReadStatus;


	warpScaleSupplyVoltage(deviceBMX055accelState.operatingVoltageMillivolts);
	i2cReadStatus = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_X_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055accelState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055accelState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 4) | (readSensorRegisterValueLSB >> 4);

	/*
	 *	Sign extend the 12-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 11)) - (1 << 11);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_Y_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055accelState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055accelState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 4) | (readSensorRegisterValueLSB >> 4);

	/*
	 *	Sign extend the 12-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 11)) - (1 << 11);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_Z_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055accelState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055accelState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 4) | (readSensorRegisterValueLSB >> 4);

	/*
	 *	Sign extend the 12-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 11)) - (1 << 11);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_TEMP, 1 /* numberOfBytes */);
	readSensorRegisterValueCombined = deviceBMX055accelState.i2cBuffer[0];

	/*
	 *	Sign extend the 8-bit value based on knowledge that upper 8 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 7)) - (1 << 7);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x,", readSensorRegisterValueCombined);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}
}

void
printSensorDataBMX055gyro(bool hexModeFlag)
{
	uint16_t	readSensorRegisterValueLSB;
	uint16_t	readSensorRegisterValueMSB;
	int16_t		readSensorRegisterValueCombined;
	WarpStatus	i2cReadStatus;


	warpScaleSupplyVoltage(deviceBMX055gyroState.operatingVoltageMillivolts);
	i2cReadStatus = readSensorRegisterBMX055gyro(kWarpSensorOutputRegisterBMX055gyroRATE_X_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055gyroState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055gyroState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 8) | (readSensorRegisterValueLSB & 0xFF);

	/*
	 *	NOTE: Here, we don't need to manually sign extend since we are packing directly into an int16_t
	 */

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055gyro(kWarpSensorOutputRegisterBMX055gyroRATE_Y_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055gyroState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055gyroState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 8) | (readSensorRegisterValueLSB & 0xFF);

	/*
	 *	NOTE: Here, we don't need to manually sign extend since we are packing directly into an int16_t
	 */

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055gyro(kWarpSensorOutputRegisterBMX055gyroRATE_Z_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055gyroState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055gyroState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 8) | (readSensorRegisterValueLSB & 0xFF);

	/*
	 *	NOTE: Here, we don't need to manually sign extend since we are packing directly into an int16_t
	 */

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}
}

void
printSensorDataBMX055mag(bool hexModeFlag)
{
	uint16_t	readSensorRegisterValueLSB;
	uint16_t	readSensorRegisterValueMSB;
	int16_t		readSensorRegisterValueCombined;
	WarpStatus	i2cReadStatus;


	warpScaleSupplyVoltage(deviceBMX055magState.operatingVoltageMillivolts);
	i2cReadStatus = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magX_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 5) | (readSensorRegisterValueLSB >> 3);

	/*
	 *	Sign extend the 13-bit value based on knowledge that upper 3 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 12)) - (1 << 12);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magY_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 5) | (readSensorRegisterValueLSB >> 3);

	/*
	 *	Sign extend the 13-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 12)) - (1 << 12);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magZ_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 7) | (readSensorRegisterValueLSB >> 1);

	/*
	 *	Sign extend the 15-bit value based on knowledge that upper 1 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 14)) - (1 << 14);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magRHALL_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2);

	/*
	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}
}

uint8_t
appendSensorDataBMX055accel(uint8_t* buf)
{
	uint8_t index = 0;

	uint16_t 	readSensorRegisterValueLSB;
	uint16_t 	readSensorRegisterValueMSB;
	int16_t 	readSensorRegisterValueCombined;
	WarpStatus 	i2cReadStatus;

	warpScaleSupplyVoltage(deviceBMX055accelState.operatingVoltageMillivolts);
	i2cReadStatus                   = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_X_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055accelState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055accelState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 4) | (readSensorRegisterValueLSB >> 4);

	/*
	 *	Sign extend the 12-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 11)) - (1 << 11);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_Y_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055accelState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055accelState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 4) | (readSensorRegisterValueLSB >> 4);

	/*
	 *	Sign extend the 12-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 11)) - (1 << 11);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_Z_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055accelState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055accelState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 4) | (readSensorRegisterValueLSB >> 4);

	/*
	 *	Sign extend the 12-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 11)) - (1 << 11);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055accel(kWarpSensorOutputRegisterBMX055accelACCD_TEMP, 1 /* numberOfBytes */);
	readSensorRegisterValueCombined = deviceBMX055accelState.i2cBuffer[0];

	/*
	 *	Sign extend the 8-bit value based on knowledge that upper 8 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 7)) - (1 << 7);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	return index;
}

uint8_t
appendSensorDataBMX055gyro(uint8_t* buf)
{
	uint8_t index = 0;

	uint16_t 	readSensorRegisterValueLSB;
	uint16_t 	readSensorRegisterValueMSB;
	int16_t 	readSensorRegisterValueCombined;
	WarpStatus 	i2cReadStatus;

	warpScaleSupplyVoltage(deviceBMX055gyroState.operatingVoltageMillivolts);
	i2cReadStatus                   = readSensorRegisterBMX055gyro(kWarpSensorOutputRegisterBMX055gyroRATE_X_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055gyroState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055gyroState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 8) | (readSensorRegisterValueLSB & 0xFF);

	/*
	 *	NOTE: Here, we don't need to manually sign extend since we are packing directly into an int16_t
	 */

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055gyro(kWarpSensorOutputRegisterBMX055gyroRATE_Y_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055gyroState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055gyroState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 8) | (readSensorRegisterValueLSB & 0xFF);

	/*
	 *	NOTE: Here, we don't need to manually sign extend since we are packing directly into an int16_t
	 */

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055gyro(kWarpSensorOutputRegisterBMX055gyroRATE_Z_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055gyroState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055gyroState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 8) | (readSensorRegisterValueLSB & 0xFF);

	/*
	 *	NOTE: Here, we don't need to manually sign extend since we are packing directly into an int16_t
	 */

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	return index;
}

uint8_t
appendSensorDataBMX055mag(uint8_t* buf)
{
	uint8_t index = 0;

	uint16_t 	readSensorRegisterValueLSB;
	uint16_t 	readSensorRegisterValueMSB;
	int16_t 	readSensorRegisterValueCombined;
	WarpStatus 	i2cReadStatus;

	warpScaleSupplyVoltage(deviceBMX055magState.operatingVoltageMillivolts);
	i2cReadStatus                   = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magX_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 5) | (readSensorRegisterValueLSB >> 3);

	/*
	 *	Sign extend the 13-bit value based on knowledge that upper 3 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 12)) - (1 << 12);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magY_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 5) | (readSensorRegisterValueLSB >> 3);

	/*
	 *	Sign extend the 13-bit value based on knowledge that upper 4 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 12)) - (1 << 12);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magZ_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 7) | (readSensorRegisterValueLSB >> 1);

	/*
	 *	Sign extend the 15-bit value based on knowledge that upper 1 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 14)) - (1 << 14);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	i2cReadStatus                   = readSensorRegisterBMX055mag(kWarpSensorOutputRegisterBMX055magRHALL_LSB, 2 /* numberOfBytes */);
	readSensorRegisterValueLSB      = deviceBMX055magState.i2cBuffer[0];
	readSensorRegisterValueMSB      = deviceBMX055magState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2);

	/*
	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);

	if (i2cReadStatus != kWarpStatusOK)
	{
		buf[index] = 0;
		index += 1;

		buf[index] = 0;
		index += 1;
	}
	else
	{
		/*
		 * MSB first
		 */
		buf[index] = (uint8_t)(readSensorRegisterValueCombined >> 8);
		index += 1;

		buf[index] = (uint8_t)(readSensorRegisterValueCombined);
		index += 1;
	}

	return index;
}