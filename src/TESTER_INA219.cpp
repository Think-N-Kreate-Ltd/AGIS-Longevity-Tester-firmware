#include <TESTER_INA219.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Tester_common.h>

#define ARRAYLENGTH 33

Adafruit_INA219 ina219;

void ina219SetUp() {
  Wire.setPins(I2C_SDA, I2C_SCL);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }

  if (!ina219.begin()) {
    ESP_LOGE("INA219", "Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  ESP_LOGD("INA219", "Init INA219 success");
}

void getCurrent() {
  for (int x=0; x<ARRAYLENGTH; x++) { // collect data with 25 times 1 set
    vTaskDelay(30);         // wait for I2C response

    // get the data from INA219
    current_mA = ina219.getCurrent_mA();
    // mark as power failure if V<11
    if (!powerFail && ina219.getBusVoltage_V()<11) {
      powerFail = true;
    } else if (powerFail && ina219.getBusVoltage_V()>11) {
      powerFail = false;
    }

    // calculate the average current in mA
    static float current[ARRAYLENGTH];  // save data with `ARRAYLENGTH` times 1 set
    static float total_current;
    total_current -= current[x];  // delete the value 25 times before
    current[x] = current_mA;
    total_current += current[x];  // update the total value
    avgCurrent_mA = total_current/ARRAYLENGTH;  // calculate the average value
  }
}