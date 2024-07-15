#pragma once

#include "rtos/mutex.hpp"
#include "rtos/task.hpp"
#include <MPU6050_light.h>
#include <Wire.h>

namespace wrvcu {

class IMU {
protected:
    MPU6050 mpu6050;
    Mutex mutex;
    Task task;

    float accX, accY, accZ, gyroX, gyroY, gyroZ;

public:
    void init(TwoWire* wire);
    void read();
    void printData();
};
}