#pragma once

#include "can/AbstractCANController.hpp"
#include "constants.hpp"
#include "rtos/task.hpp"

#define HEARTBEAT_COB_ID 0x700
#define SDO_REQUEST_COB_ID 0x600
#define SDO_RESPONSE_COB_ID 0x580
#define NMT_COB_ID 0x000

namespace wrvcu {

enum class NMTCommand {
    Operational = 0x1,
    Stopped = 0x2,
    PreOperational = 0x80,
    Reset = 0x81,
    ResetComms = 0x82
};

enum class NMTState {
    Boot = 0x00,
    Stopped = 0x04,
    Operational = 0x05,
    PreOperational = 0x7f,
};

struct SDOMessage {
    uint16_t index;
    uint8_t subindex;
    uint8_t data[4];
};

struct PDOMessage {
    uint32_t cobID;
    uint8_t data[8];
};

class CANOpenDevice {
protected:
    AbstractCANController* can;
    uint8_t nodeID;

    NMTState nmtState = NMTState::Boot;

    Queue<PDOMessage, 256>* pdoQueue;
    Queue<SDOMessage, 256>* sdoQueue;

    Queue<CANMessage, 256> canQueue;

    Task task;

    void loop();

public:
    /**
     * @brief Initialise the CANOpen Device
     *
     * @param ican The CAN controller for the bus the device is on
     * @param inodeID The node ID of the device
     */
    void init(AbstractCANController* ican, uint8_t inodeID, uint32_t task_priority);

    /**
     * @brief Send an NMT command to the device
     *
     * @param cmd The NMT command
     */
    void sendNMT(NMTCommand cmd);

    /**
     * @brief Send an SDO write message
     *
     * @param numBytes The number of bytes in the data parameter
     * @param index The SDO object index
     * @param subindex The SDO object subindex
     * @param data The data to write
     */
    void sendSDOWrite(uint8_t numBytes, uint16_t index, uint8_t subindex, uint8_t data[]);
    void sendSDORead(uint16_t index, uint8_t subindex);

    /**
     * @brief Send a PDO to this node. !! Important !! This is COB ID, not CAN ID, so do not include node ID.
     * e.g COB ID = 0x180, not 0x181
     *
     * @param cob_id The PDO COB ID.
     */
    void sendPDO(uint32_t cob_id, uint8_t data[8]);

    /**
     * @brief Subscribe to PDOs from the device
     *
     * @param pdoQueue The queue of PDO messages
     */
    void addPDOQueue(Queue<PDOMessage, 256>* pdoQueue);

    /**
     * @brief Subscribe to the results from SDO reads
     *
     * @param sdoQueue The queue of SDO messages that will be written to
     */
    void addSDOQueue(Queue<SDOMessage, 256>* sdoQueue);

    /**
     * @brief Add a COB ID to be subscribed to
     *
     * @param cob_id The COB ID to subscribe to
     */
    void subscribePDO(uint32_t cob_id);

    NMTState getNMTState();
};

}
