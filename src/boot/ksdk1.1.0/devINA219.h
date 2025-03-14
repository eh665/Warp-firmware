/*
made using devMMA8451Q.h as a template
06 March 2025 Elizabeth Hawke

From http://www.adafruit.com/datasheets/ina219.pdf we know that 
'The Power Register and Current Register default to '0' because the Calibration Register defaults to '0', yielding a zero current value until
the Calibration Register is programmed'


https://www.nxp.com/docs/en/data-sheet/MMA8451Q.pdf

*/

void		initINA219(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts);
WarpStatus	readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes);
WarpStatus	writeSensorRegisterINA219(uint8_t deviceRegister, uint8_t payloadBtye);
WarpStatus 	configureSensorINA219(uint8_t payloadConfig, uint8_t payloadCalib); //ok so F_SETUP is the name of a MMA8451Q register, so not gonna need it/need to change it
void		printSensorDataINA219(bool hexModeFlag);
uint8_t		appendSensorDataINA219(uint8_t* buf);

const uint8_t bytesPerMeasurementINA219            = 2;
const uint8_t bytesPerReadingINA219                = 2;
const uint8_t numberOfReadingsPerMeasurementINA219 = 1;