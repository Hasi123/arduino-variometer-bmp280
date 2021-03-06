
#include <I2CHelper.h>
#include <MPU6050.h>
#include <ms5611.h>
#include <kalmanvert.h>
#include <beeper.h>
#include <marioSounds.h>
#include <varioPower.h>


VarioPower varioPower;
kalmanvert kalmanvert;


void setup() {
  //init varioPower
  varioPower.init();

  //Serial.begin(57600);
  //Serial.println("Start");

  /**************************/
  /* init Two Wires devices */
  /**************************/
  I2C::begin();

  //ms5611
  ms.init();

  //MPU6050
  mpu.init(); // load dmp and setup for normal use
  attachInterrupt(digitalPinToInterrupt(MPU6050_INTERRUPT_PIN), getSensors, RISING);

  //play sound and check if need to update
  marioSounds.bootUp();
  varioPower.updateFW();
  if (mpu.calibrate()) //run calibration if up side down
    varioPower.reset(); //reset to load calibration data and dmp again

  //init kalman filter
  I2C::newData = 0;
  while(!I2C::newData); //wait for fresh data
  ms.update();
  float firstAlti = ms.getAltitude();
  Serial.println(firstAlti);
  kalmanvert.init(firstAlti,
                  0.0,
                  POSITION_MEASURE_STANDARD_DEVIATION,
                  ACCELERATION_MEASURE_STANDARD_DEVIATION,
                  millis());
}

void loop() {
  //new sensors ready
  if (I2C::newData) { // read interrupt status register
    I2C::newData = false;
    //static unsigned long lastTime;
    //unsigned long now = micros();
    //Serial.print(now - lastTime); Serial.print("\t");
    //lastTime = now;


    ms.update();
    float alt = ms.getAltitude();
    //Serial.print(alt); Serial.print("\t");


    float vertAccel = mpu.getVertaccel();
    //Serial.print(vertAccel, 5); Serial.print(" \t");
    //Serial.print(newaccel[0]); Serial.print("\t");
    //Serial.print(newaccel[1]); Serial.print("\t");
    //Serial.print(newaccel[2]); Serial.print("\t");


    kalmanvert.update1(vertAccel, millis());
    kalmanvert.update2(alt);
    //Serial.print(kalmanvert.getVelocity()); Serial.print("\t");


    // set beeper
    beeper::setVelocity(kalmanvert.getVelocity());

    //Serial.println();
  }

  varioPower.update();
  beeper::update();

}

void getSensors() {
  I2C::intHandler();
}
