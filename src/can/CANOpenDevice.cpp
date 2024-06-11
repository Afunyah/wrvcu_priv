#include "can/CANOpenDevice.hpp"

namespace wrvcu {

void CANOpenDevice::init(AbstractCANController* ican, uint8_t inodeID, uint32_t task_priority) {
    this->can = ican;
    this->nodeID = inodeID;

    canQueue.init();

    can->subscribe(HEARTBEAT_COB_ID + nodeID, &canQueue);    // sub to heartbeat
    can->subscribe(SDO_RESPONSE_COB_ID + nodeID, &canQueue); // sub to SDO reply

    task.start(
        [this] {
            loop();
        },
        task_priority,
        "CANOpen_Task");
};
void CANOpenDevice::sendNMT(NMTCommand cmd) {
    CANMessage msg = {
        .id = NMT_COB_ID,
        .len = 2
    };

    msg.data[0] = (int)cmd;
    msg.data[1] = nodeID;

    can->send(msg);
};

void CANOpenDevice::sendSDOWrite(uint8_t numBytes, uint16_t index, uint8_t subindex, uint8_t data[]) {
    // if (numBytes == 0 || numBytes > 4)
    // throw std::invalid_argument("received numBytes out of range");;

    uint8_t ccs = 1;          // initiate a download (write)
    uint8_t n = 4 - numBytes; // number of bytes which *do not* contain data
    uint8_t e = 1;            // expedited transfer - all data is in this message. If 0, it is a segmented transfer
    uint8_t s = 1;            // data size is specified

    CANMessage msg = {
        .id = (uint32_t)SDO_REQUEST_COB_ID + nodeID,
        .len = 8
    };

    msg.data[0] = ((ccs & 0b111) << 5) | ((n & 0b11) << 2) | ((e & 1) << 1) | (s & 1);
    msg.data[1] = index & 0xff;
    msg.data[2] = (index & 0xff00) >> 8;
    msg.data[3] = subindex & 0xff;

    memset(msg.data + 4, 0, 4);           // set the 4 data bytes to 0
    memcpy(msg.data + 4, data, numBytes); // copy bytes from input data to the buffer between 4-7

    can->send(msg);
};

void CANOpenDevice::sendSDORead(uint16_t index, uint8_t subindex) {
    uint8_t ccs = 2; // initiate an upload (read)
    uint8_t n = 0;   // ignore other fields
    uint8_t e = 0;
    uint8_t s = 0;

    CANMessage msg = {
        .id = (uint32_t)SDO_REQUEST_COB_ID + nodeID,
        .len = 8
    };

    msg.data[0] = ((ccs & 0b111) << 5) | ((n & 0b11) << 2) | ((e & 1) << 1) | (s & 1);
    msg.data[1] = index & 0xff;
    msg.data[2] = (index & 0xff00) >> 8;
    msg.data[3] = subindex & 0xff;

    memset(msg.data + 4, 0, 4); // set the 4 data bytes to 0

    can->send(msg);
};

void CANOpenDevice::sendPDO(uint32_t cob_id, uint8_t data[8]) {
    CANMessage msg = {
        .id = cob_id + nodeID,
        .len = 8
    };
    memcpy(msg.data, data, 8);

    can->send(msg);
};

void CANOpenDevice::addPDOQueue(Queue<PDOMessage, 256>* ipdoQueue) {
    pdoQueue = ipdoQueue;
};

void CANOpenDevice::addSDOQueue(Queue<SDOMessage, 256>* isdoQueue) {
    sdoQueue = isdoQueue;
};

void CANOpenDevice::subscribePDO(uint32_t cob_id) {
    can->subscribe(cob_id + nodeID, &canQueue);
}

void CANOpenDevice::loop() {

    while (true) {
        CANMessage canMessage = canQueue.dequeue(TIMEOUT_MAX);

        if (canMessage.id == (SDO_REQUEST_COB_ID + nodeID)) { // SDO Message
            uint16_t index = (canMessage.data[2] << 8) + canMessage.data[1];

            SDOMessage sdoMessage = {
                .index = index,
                .subindex = canMessage.data[3]
            };
            memcpy(sdoMessage.data, canMessage.data + 4, 4); // copy last 4 bytes

            sdoQueue->enqueue(sdoMessage, TIMEOUT_MAX);

        } else if (canMessage.id == (HEARTBEAT_COB_ID + nodeID)) { // heartbeat message
            nmtState = static_cast<NMTState>(canMessage.data[0]);
        } else { // PDO message

            PDOMessage pdoMessage = {
                .cobID = canMessage.id - nodeID
            };
            memcpy(pdoMessage.data, canMessage.data, 8);

            pdoQueue->enqueue(pdoMessage, TIMEOUT_MAX);
        }

        // yield -  we block when reading the can queue anyway so could be 0
        Task::delay(1);
    }
}

NMTState CANOpenDevice::getNMTState() {
    return nmtState;
}

}