#ifndef CAN_H_
#define CAN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define 	CAN_NORMAL_MODE			0U
#define		CAN_LOOP_BACK_MODE		1U

#define		CAN_BUS_OK		0U
#define		CAN_OK			0U

#define 	CAN_ERROR_INVALID_ARGUMENT		(-22)
#define		CAN_ERROR_BUSY					(-16)
#define		CAN_ERROR_ALLOC_FAILED			(-56)
#define		CAN_ERROR_ALREADY_INITALIZED	(-58)
#define		CAN_ERROR_DATASIZE_TOO_LARGE	(-75)

#define		CAN_TMS_INCOMPLETE	0U
#define		CAN_TMS_COMPLETE	1U

#define		CAN_TMS_ERR_OVF		0x02U

#define		CAN_STD_LOOP_BACK_TEST_ID	(0xE4U)

typedef int can_result_t;

typedef struct
{
	uint32_t stdID;
	uint8_t * data;
	uint32_t dataSize;

}CanTransmissionData;

typedef struct
{
	void (*dataReceived)(CanTransmissionData * tData);
	void (*errorOccurred)(uint32_t errorCode);

}CanCallback;

can_result_t can_init(uint32_t mode, int nFilters, uint32_t* filterIDs);
can_result_t can_deinit();
can_result_t can_send_data(CanTransmissionData * tData);
can_result_t can_set_callback(CanCallback* callback);
can_result_t can_enable_loop_back_mode();

#ifdef __cplusplus
 } // extern "C"
#endif

#endif /* CAN_H_ */
