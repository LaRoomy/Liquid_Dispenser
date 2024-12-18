#ifndef CAN_COMMUNICATION_CODES_H_
#define CAN_COMMUNICATION_CODES_H_

constexpr uint32_t OUTPUT_CONFIRMED_NOTIFICATON_ID = 0xD0U;
constexpr uint32_t OUTPUT_CANCELLED_NOTIFICATION_ID = 0xD1U;
constexpr uint32_t OUTPUT_STOPPED_NOTIFICATION_ID = 0xD2U;
constexpr uint32_t OUTPUT_COMPLETE_NOTIFICATION_ID = 0xD3U;
constexpr uint32_t OUTPUT_OPERATION_INITIATED_NOTIFICATION_ID = 0xD4U;
constexpr uint32_t SYSTEM_ERROR_OCCURRED_NOTIFICATION_ID = 0xD9U;

constexpr uint32_t INVALID_NOTIFICATION_ID = 0xFFFFFF;

#endif /* CAN_COMMUNICATION_CODES_H_ */
