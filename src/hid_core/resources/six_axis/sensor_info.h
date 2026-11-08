// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstring>
#include <cstddef>

#include "hid_core/hid_types.h"

namespace Service::HID {

// Keep defaults outside the trivial IPC structure. Zero ranges invalidate guest sensor fusion.
constexpr Core::HID::SixAxisSensorIcInformation DefaultSixAxisSensorIcInformation() {
    return {
        .angular_rate = 2000.0f,
        .unknown_gyro_data1 = {-10.0f, -10.0f, -10.0f, 10.0f, 10.0f, 10.0f},
        .unknown_gyro_data2 = {0.95f, -0.003f, -0.003f, -0.003f, 0.95f, -0.003f,
                              -0.003f, -0.003f, 0.95f},
        .unknown_gyro_data3 = {1.05f, 0.003f, 0.003f, 0.003f, 1.05f, 0.003f,
                              0.003f, 0.003f, 1.05f},
        .acceleration_range = 8.0f,
        .unknown_accel_data1 = {-0.0612f, -0.0612f, -0.0612f, 0.0612f, 0.0612f, 0.0612f},
        .unknown_accel_data2 = {0.95f, -0.003f, -0.003f, -0.003f, 0.95f, -0.003f,
                               -0.003f, -0.003f, 0.95f},
        .unknown_accel_data3 = {1.05f, 0.003f, 0.003f, 0.003f, 1.05f, 0.003f,
                               0.003f, 0.003f, 1.05f},
    };
}

} // namespace Service::HID

namespace Service::HID {

// The 0x744 blob returned by LoadSixAxisSensorCalibrationParameter has never been fully
// reverse-engineered, but its leading 0x18 bytes are believed to mirror the Joy-Con SPI
// factory calibration block (12 little-endian s16 values): accelerometer origin xyz,
// accelerometer sensitivity xyz, gyroscope origin xyz, gyroscope sensitivity xyz.
// The Joy-Con IMU uses 4096 LSB/g at the 8G range and 16 LSB/dps at the 2000dps range.
// An all-zero blob gives the guest scale/offset of zero, which several titles treat as
// a dead sensor (Nintendo Switch Sports boot calibration).
inline Core::HID::SixAxisSensorCalibrationParameter DefaultSixAxisSensorCalibrationParameter() {
    Core::HID::SixAxisSensorCalibrationParameter calibration{};
    constexpr std::array<s16, 12> factory_calibration{
        0,    0,    0,    // accelerometer origin
        4096, 4096, 4096, // accelerometer sensitivity (counts per g)
        0,    0,    0,    // gyroscope origin
        16,   16,   16,   // gyroscope sensitivity (counts per dps)
    };
    static_assert(sizeof(factory_calibration) == 0x18);
    std::memcpy(calibration.unknown_data.data(), factory_calibration.data(),
                sizeof(factory_calibration));
    return calibration;
}

} // namespace Service::HID
