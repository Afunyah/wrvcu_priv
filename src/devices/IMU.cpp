#include "devices/IMU.hpp"
#include "constants.hpp"
#include "logging/log.hpp"

namespace wrvcu {

void IMU::init(TwoWire* wire) {
    INFO("Initializing IMU...");
    wire->begin();
    mpu6050 = MPU6050(*wire);

    byte status = mpu6050.begin();
    if (status != 0) {
        ERROR("IMU: Could not connect to MPU6050.");
    }
    mpu6050.calcOffsets(true, true); // gyro and accelero
    mutex.init();
    task.start(
        [this] { read(); }, IMU_TASK_PRIORITY, "IMU_Task");
}

void IMU::read() {
    while (true) {
        mutex.take();
        mpu6050.update();

        accX = mpu6050.getAccX();
        accY = mpu6050.getAccY();
        accZ = mpu6050.getAccZ();

        gyroX = mpu6050.getGyroX();
        gyroY = mpu6050.getGyroY();
        gyroZ = mpu6050.getGyroZ();

        // printData();

        mutex.give();

        Task::delay(10);
    }
}

void IMU::printData() {
    Serial.print("ACC X Y Z: ");
    Serial.print(accX);
    Serial.print("     ");
    Serial.print(accY);
    Serial.print("     ");
    Serial.print(accZ);
    Serial.println("");

    Serial.print("GYRO X Y Z: ");
    Serial.print(gyroX);
    Serial.print("     ");
    Serial.print(gyroY);
    Serial.print("     ");
    Serial.print(gyroZ);
    Serial.println("");

    Serial.println("--------------------------------------------");
}
}