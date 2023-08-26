#ifndef TESTER_INA219_H
#define TESTER_INA219_H

#define I2C_SCL 41
#define I2C_SDA 40

extern float current_mA;
extern float avgCurrent_mA;

void ina219SetUp();
void getCurrent();

#endif  // TESTER_INA219_H