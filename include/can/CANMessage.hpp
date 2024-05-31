#pragma once

#include <cstdint>

namespace wrvcu {
struct CANMessage {
    uint32_t id = 0;         // can identifier
    uint8_t len = 8;         // length of data
    uint8_t data[8] = { 0 }; // data

    uint16_t timestamp = 0; // FlexCAN time when message arrived
    struct {
        bool extended = 0; // identifier is extended (29-bit)
        bool remote = 0;   // remote transmission request packet type
        bool overrun = 0;  // message overrun
        bool reserved = 0;
    } flags;
};

}