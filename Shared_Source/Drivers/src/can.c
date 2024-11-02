#include <stm32f4xx.h>
#include <malloc.h>
#include "can.h"

#define		GPIOAEN		(1U<<0)
#define		CAN1EN		(1U<<25)

#define		MCR_INRQ	(1U<<0)
#define		MCR_RS		(1U<<15)
#define		MSR_INAK	(1U<<0)
#define		BTR_LBKM	(1U<<30)
#define		MCR_SLEEP	(1U<<1)
#define		MSR_SLAK	(1U<<1)
#define		MSR_ERRI	(1U<<2)
#define		FMR_FINIT	(1U<<0)
#define		TIR_TXRQ	(1U<<0)
#define		IER_LECIE	(1U<<11)
#define		IER_ERRIE	(1U<<15)
#define		IER_TMEIE	(1U<<0)
#define		IER_FMPIE0	(1U<<1)
#define		RF0R_RFOM0	(1U<<5)
#define		TSR_RQCP0	(1U<<0)
#define		TSR_RQCP1	(1U<<8)
#define		TSR_RQCP2	(1U<<16)
#define		MCR_TXFP	(1U<<2)
#define		RFR_FOVR	(1U<<4)

static uint32_t canInitialized = 0;

typedef struct
{
	uint32_t output_transmission_open;
	uint32_t current_transmission_id;
	int current_transmit_data_index;
	uint32_t current_segment_index;
	uint8_t data_holder[256];
	uint32_t current_transmit_data_size;
	uint32_t current_number_of_segments;

}CanOutputTransmissionParams;

typedef struct
{
	uint8_t input_buffer[256];
	uint32_t input_current_data_index;
	uint32_t input_current_tranmission_open;
	uint32_t input_error_flags;
	uint32_t input_buffer_length;
	uint32_t input_message_std_id;

}CanInputTransmissionParams;

static CanCallback * cCallback = NULL;
static CanOutputTransmissionParams outputParams;
static CanInputTransmissionParams inputParams;

static void clear_input_transmission_params();
static void clear_output_transmission_params();
static void can_send_next_data();
static void can_handle_tx_irq();
static void can_handle_rx_irq();
static void can_handle_sce_irq();
static uint32_t can_fill_output_mailbox_with_segment(CAN_TxMailBox_TypeDef* mailbox, uint32_t dest_std_id);

static void can_init_internal_parameter()
{
	outputParams.current_number_of_segments = 0;
	outputParams.current_segment_index = 0;
	outputParams.current_transmission_id = 0;
	outputParams.current_transmit_data_index = -1;
	outputParams.current_transmit_data_size = 0;
	outputParams.output_transmission_open = 0;

	for(int i = 0; i < 256; i++)
	{
		outputParams.data_holder[i] = 0;
	}

	inputParams.input_buffer_length = 0;
	inputParams.input_current_data_index = 0;
	inputParams.input_current_tranmission_open = 0;
	inputParams.input_error_flags = 0;
	inputParams.input_message_std_id = 0;

	for(int i = 0; i < 256; i++)
	{
		inputParams.input_buffer[i] = 0;
	}
}

can_result_t can_init(uint32_t mode, int nFilters, uint32_t* filterIDs)
{
	if(mode != CAN_NORMAL_MODE && mode != CAN_LOOP_BACK_MODE)
	{
		return CAN_ERROR_INVALID_ARGUMENT;
	}
	if(mode == CAN_NORMAL_MODE && filterIDs == NULL)
	{
		return CAN_ERROR_INVALID_ARGUMENT;
	}

	can_init_internal_parameter();

	// config GPIO >>

	// enable clock access to GPIOA
	RCC->AHB1ENR |= (GPIOAEN);

	// PA11 to alternate function mode (CAN_RX)
	GPIOA->MODER |= (1U<<23);
	GPIOA->MODER &= ~(1U<<22);

	// PA12 to alternate function mode (CAN_TX)
	GPIOA->MODER |= (1U<<25);
	GPIOA->MODER &= ~(1U<<24);

	// set PA11 alternate function type (AF9)
	GPIOA->AFR[1] |=  (1U<<12);
	GPIOA->AFR[1] &= ~(1U<<13);
	GPIOA->AFR[1] &= ~(1U<<14);
	GPIOA->AFR[1] |=  (1U<<15);

	// set PA12 alternate function type (AF9)
	GPIOA->AFR[1] |=  (1U<<16);
	GPIOA->AFR[1] &= ~(1U<<17);
	GPIOA->AFR[1] &= ~(1U<<18);
	GPIOA->AFR[1] |=  (1U<<19);

	/***********************************************/
	// config CAN

	// enable clock access to CAN1
	RCC->APB1ENR |= (CAN1EN);

	// enable CAN initialization mode and wait for acknowledgment
	CAN1->MCR |= (MCR_INRQ);
	while (!(CAN1->MSR & (MSR_INAK))){}

	if(mode == CAN_LOOP_BACK_MODE)
	{
		// set loop-back mode for CAN1
		CAN1->BTR |= (BTR_LBKM);
	}

	// set the re-synchronization jump width to 1 - [0:0] = 0 + 1 = 1
	CAN1->BTR &= ~(1U<<24);
	CAN1->BTR &= ~(1U<<25);

	// set the number of time quanta for time segment 2 - [0:1:1] = 3 + 1 = 4
	CAN1->BTR |= ((1U<<20)|(1U<<21));
	CAN1->BTR &= ~(1U<<22);

	// set the number of time quanta for time segment 1 - [0:0:1:0] = 2 + 1 = 3
	CAN1->BTR &= ~((1U<<16)|(1U<<18)|(1U<<19));
	CAN1->BTR |= (1U<<17);

	// set the baudrate prescaler to (16-1)
	CAN1->BTR |= (0x0FU<<0);

	// exit CAN initialization mode and wait for the INAK bit to be cleared
	CAN1->MCR &= ~(MCR_INRQ);
	while (CAN1->MSR & MSR_INAK){}

	// exit sleep mode
	CAN1->MCR &= ~(MCR_SLEEP);
	while (CAN1->MSR & (MSR_SLAK)){}

	/***********************************************/
	// config CAN filters

	// enter filter initialization mode
	CAN1->FMR |= (FMR_FINIT);

	// set the number of filters for CAN1 (and CAN2 respectively) half REL 14:14
	CAN1->FMR |= (14U<<8);

	// select the single 32-bit scale configuration for the first 14 filters of CAN1
	CAN1->FS1R |= (0x3FFFU);

	// set the receive ID
	if(mode == CAN_LOOP_BACK_MODE)
	{
		// set only filter at pos 0
		CAN1->sFilterRegister[0].FR1 = ((CAN_STD_LOOP_BACK_TEST_ID)<<21);
	}
	else
	{
		for(int i = 0; i < nFilters; i++)
		{
			if(i > 13)
			{
				break;
			}
			CAN1->sFilterRegister[i].FR1 = (filterIDs[i]<<21);
		}
	}

	// select identifier list mode for the first 14 filters of CAN1
	CAN1->FM1R |= (0x3FFFU);

	// activate filters
	if(mode == CAN_LOOP_BACK_MODE)
	{
		// activate filter at pos 0
		CAN1->FA1R |= (1U<<0);
	}
	else
	{
		for(uint32_t i = 0; i < ((uint32_t)14); i++)
		{
			if(i < nFilters)
			{
				CAN1->FA1R |= (1U<<i);
			}
			else
			{
				CAN1->FA1R &= ~(1U<<i);
			}
		}
	}

	// exit filter initialization mode
	CAN1->FMR &= ~(FMR_FINIT);

	// activate interrupts
	CAN1->IER |= ((IER_LECIE)/*|(IER_ERRIE)*/|(IER_FMPIE0)|(IER_TMEIE));

	// make sure the interrupt level is not zero and
	// make sure the can rx interrupt is prioritized over the tx interrupt
	NVIC_SetPriority(CAN1_RX0_IRQn, 1U);
	NVIC_SetPriority(CAN1_TX_IRQn, 2U);

	// enable interrupts in nvic
	NVIC_EnableIRQ(CAN1_TX_IRQn);
	NVIC_EnableIRQ(CAN1_RX0_IRQn);
	NVIC_EnableIRQ(CAN1_SCE_IRQn);

	canInitialized = 1;
	return CAN_BUS_OK;
}

can_result_t can_deinit()
{
	if(canInitialized != 0)
	{
		return CAN_ERROR_ALREADY_INITALIZED;
	}
	// diable interrupts in nvic
	NVIC_DisableIRQ(CAN1_TX_IRQn);
	NVIC_DisableIRQ(CAN1_RX0_IRQn);
	NVIC_DisableIRQ(CAN1_SCE_IRQn);

	// deactivate interrupts
	CAN1->IER = 0;

	// enter filter initialization mode
	CAN1->FMR |= (FMR_FINIT);

	// reset scale configuration for the first 14 filters of CAN1
	CAN1->FS1R &= ~(0x3FFFU);

	// reset and deactivate filters
	for(int i = 0; i < 14; i++)
	{
		CAN1->sFilterRegister[i].FR1 = 0;
		CAN1->FA1R &= ~(1U<<i);
	}

	// exit filter initialization mode
	CAN1->FMR &= ~(FMR_FINIT);

	// enable CAN initialization mode and wait for acknowledgment
	CAN1->MCR |= (MCR_INRQ);
	while (!(CAN1->MSR & (MSR_INAK))){}

	CAN1->BTR = 0x01230000; // btr to reset value

	// exit CAN initialization mode and wait for the INAK bit to be cleared
	CAN1->MCR &= ~(MCR_INRQ);
	while (CAN1->MSR & MSR_INAK){}

	// reset mcr reg
	CAN1->MCR |= (MCR_RS);

	// disable clock access to CAN1
	RCC->APB1ENR &= ~(CAN1EN);

	// PA11 to input (reset state) (CAN_RX)
	GPIOA->MODER &= ~(1U<<23);
	GPIOA->MODER &= ~(1U<<22);

	// PA12 to input (reset state) (CAN_TX)
	GPIOA->MODER &= ~(1U<<25);
	GPIOA->MODER &= ~(1U<<24);

	// reset PA11 alternate function type (AF0)
	GPIOA->AFR[1] &= ~(1U<<12);
	GPIOA->AFR[1] &= ~(1U<<13);
	GPIOA->AFR[1] &= ~(1U<<14);
	GPIOA->AFR[1] &= ~(1U<<15);

	// reset PA12 alternate function type (AF0)
	GPIOA->AFR[1] &= ~(1U<<16);
	GPIOA->AFR[1] &= ~(1U<<17);
	GPIOA->AFR[1] &= ~(1U<<18);
	GPIOA->AFR[1] &= ~(1U<<19);

	return CAN_BUS_OK;
}

can_result_t can_send_data(CanTransmissionData * tData)
{
	if(outputParams.output_transmission_open > 0)
	{
		return CAN_ERROR_BUSY;
	}
	if(tData == NULL)
	{
		return CAN_ERROR_INVALID_ARGUMENT;
	}
	if(tData->dataSize == 0 || tData->data == NULL)
	{
		return CAN_ERROR_INVALID_ARGUMENT;
	}
	if(tData->dataSize > 256)
	{
		return CAN_ERROR_DATASIZE_TOO_LARGE;
	}
	/*
	 * Transmission sequence for segmented transmissions:
	 *
	 * [0] 		= number of segments in transmission
	 * [1] 		= current segment index
	 * [2..7] 	= data bytes
	 *
	 * */
	uint32_t mb0_written = 0;
	uint32_t mb1_written = 0;
	uint32_t mb2_written = 0;
	uint32_t data_finished = 0;

	// mark transmission as open
	outputParams.output_transmission_open = 1;

	// set first transmission field index
	outputParams.current_transmit_data_index = 0;

	// save destination id for further usage
	outputParams.current_transmission_id = tData->stdID;

	// save data size
	outputParams.current_transmit_data_size = tData->dataSize;

	// calculate number of segments
	if((tData->dataSize % 6) != 0)
	{
		outputParams.current_number_of_segments = (tData->dataSize / 6) + 1;
	}
	else
	{
		outputParams.current_number_of_segments = (tData->dataSize / 6);
	}

	// set transmit registers in fifo mode
	CAN1->MCR |= (MCR_TXFP);

	// copy the data for further usage (erase buffer before)
	for(uint32_t i = 0; i < 256; i++)
	{
		outputParams.data_holder[i] = 0;
	}

	// copy data
	for(uint32_t i = 0; i < tData->dataSize; i++)
	{
		outputParams.data_holder[i] = tData->data[i];
	}

	// start with mailbox 0
	data_finished =
			can_fill_output_mailbox_with_segment(&CAN1->sTxMailBox[0], tData->stdID);

	mb0_written = 1;

	if(data_finished == 0)
	{
		// mailbox 1
		data_finished =
				can_fill_output_mailbox_with_segment(&CAN1->sTxMailBox[1], tData->stdID);

		mb1_written = 1;

		if(data_finished == 0)
		{
			// mailbox 2
			data_finished =
					can_fill_output_mailbox_with_segment(&CAN1->sTxMailBox[2], tData->stdID);

			mb2_written = 1;
		}
	}

	// request transmission in the right order how the data was set
	if(mb0_written == 1)
	{
		CAN1->sTxMailBox[0].TIR |= (TIR_TXRQ);
	}
	if(mb1_written == 1)
	{
		CAN1->sTxMailBox[1].TIR |= (TIR_TXRQ);
	}
	if(mb2_written == 1)
	{
		CAN1->sTxMailBox[2].TIR |= (TIR_TXRQ);
	}
	return CAN_BUS_OK;
}

static void can_send_next_data()
{
	if(outputParams.output_transmission_open == 1)
	{
		if(outputParams.current_transmit_data_index < outputParams.current_transmit_data_size)
		{
			uint32_t mb0_written = 0;
			uint32_t mb1_written = 0;
			uint32_t mb2_written = 0;
			uint32_t data_finished = 0;

			if(CAN1->TSR & (TSR_RQCP0))
			{
				CAN1->TSR |= (TSR_RQCP0);

				data_finished = can_fill_output_mailbox_with_segment(
					&CAN1->sTxMailBox[0],
					outputParams.current_transmission_id
				);
				mb0_written = 1;
			}

			if(data_finished == 0)
			{
				if(CAN1->TSR & (TSR_RQCP1))
				{
					CAN1->TSR |= (TSR_RQCP1);

					data_finished = can_fill_output_mailbox_with_segment(
						&CAN1->sTxMailBox[1],
						outputParams.current_transmission_id
					);
					mb1_written = 1;
				}

				if(data_finished == 0)
				{
					if(CAN1->TSR & (TSR_RQCP2))
					{
						CAN1->TSR |= (TSR_RQCP2);

						data_finished = can_fill_output_mailbox_with_segment(
							&CAN1->sTxMailBox[2],
							outputParams.current_transmission_id
						);
						mb2_written = 1;
					}
				}
			}
			// request transmission in the right order how the data was set
			if(mb0_written == 1)
			{
				CAN1->sTxMailBox[0].TIR |= (TIR_TXRQ);
			}
			if(mb1_written == 1)
			{
				CAN1->sTxMailBox[1].TIR |= (TIR_TXRQ);
			}
			if(mb2_written == 1)
			{
				CAN1->sTxMailBox[2].TIR |= (TIR_TXRQ);
			}
		}
		else
		{
			// reset data holder params and delete memory space if the last package was sent
			clear_output_transmission_params();
		}
	}
}

static uint32_t can_fill_output_mailbox_with_segment(CAN_TxMailBox_TypeDef* mailbox, uint32_t dest_std_id)
{
	// start with mailbox 0
	mailbox->TIR = 0;
	mailbox->TIR = (dest_std_id<<21);

	uint8_t* array_pos = &outputParams.data_holder[outputParams.current_transmit_data_index]; // get the pointer to the actual array field
	uint32_t act_data_size =										// get the actual size for this segment
			((outputParams.current_transmit_data_size >= (outputParams.current_transmit_data_index + 6)) ? 6: (outputParams.current_transmit_data_size - outputParams.current_transmit_data_index));

	uint32_t tSize = act_data_size + 2;
	mailbox->TDTR &= ~(0xFU);			// clear
	mailbox->TDTR |= (tSize & 0xFU);	// set transmission lenght

	mailbox->TDLR = outputParams.current_number_of_segments;		// number of segments
	mailbox->TDLR |= ((outputParams.current_segment_index<<8) & 0x0000FF00);

	for(uint32_t i = 0; i < act_data_size; i++)
	{
		switch(i)
		{
		case 0:
			mailbox->TDLR |= ((array_pos[0]<<16) & 0x00FF0000);
			break;
		case 1:
			mailbox->TDLR |= ((array_pos[1]<<24) & 0xFF000000);
			break;
		case 2:
			mailbox->TDHR = array_pos[2];
			break;
		case 3:
			mailbox->TDHR |= ((array_pos[3]<<8) & 0x0000FF00);
			break;
		case 4:
			mailbox->TDHR |= ((array_pos[4]<<16) & 0x00FF0000);
			break;
		case 5:
			mailbox->TDHR |= ((array_pos[5]<<24) & 0xFF000000);
			break;
		default:
			break;
		}
	}
	outputParams.current_segment_index++;
	outputParams.current_transmit_data_index += act_data_size;

	// check if the data transmission is complete
	if((act_data_size < 6) || (outputParams.current_transmit_data_index >= outputParams.current_transmit_data_size))
	{
		return 1;
	}
	return 0;
}

static void clear_output_transmission_params()
{
	outputParams.output_transmission_open = 0;
	outputParams.current_transmission_id = 0;
	outputParams.current_transmit_data_index = -1;
	outputParams.current_segment_index = 0;
	outputParams.current_transmit_data_size = 0;
	outputParams.current_number_of_segments = 0;
}

static void clear_input_transmission_params()
{
	inputParams.input_current_data_index = 0;
	inputParams.input_current_tranmission_open = 0;
}

static void clear_input_buffer()
{
	for(uint32_t i = 0; i < 256; i++)
	{
		inputParams.input_buffer[i] = 0;
	}
	inputParams.input_buffer_length = 0;
	inputParams.input_message_std_id = 0;
}

static uint32_t can_receive_data()
{
	uint32_t total_num_segments = 0;
	uint32_t cur_segment_num = 0;
	uint32_t numMsgInFifo = (CAN1->RF0R & 0x03);
	uint32_t bytes_in_frame = 0;

	// reception fifo overflow
	if((CAN1->RF0R & (RFR_FOVR)) != 0)
	{
		inputParams.input_error_flags |= CAN_TMS_ERR_OVF;
		return CAN_TMS_ERR_OVF;
	}

	while(numMsgInFifo > 0)
	{
		// get data out of rx mailbox
		total_num_segments = (CAN1->sFIFOMailBox[0].RDLR & 0x000000FF);
		cur_segment_num = ((CAN1->sFIFOMailBox[0].RDLR & 0x0000FF00)>>8);
		bytes_in_frame = (CAN1->sFIFOMailBox[0].RDTR & 0xF);

		if(cur_segment_num == 0)
		{
			// -> start of transmission
			clear_input_buffer();
			clear_input_transmission_params();
			inputParams.input_error_flags = 0;
			inputParams.input_message_std_id = ((CAN1->sFIFOMailBox[0].RIR & 0xFFE00000)>>21);
		}

		if(bytes_in_frame > 2)
		{
			inputParams.input_buffer[inputParams.input_current_data_index] = ((CAN1->sFIFOMailBox[0].RDLR & 0x00FF0000)>>16);
			inputParams.input_current_data_index++;
		}

		if(bytes_in_frame > 3)
		{
			inputParams.input_buffer[inputParams.input_current_data_index] = ((CAN1->sFIFOMailBox[0].RDLR & 0xFF000000)>>24);
			inputParams.input_current_data_index++;
		}

		if(bytes_in_frame > 4)
		{
			inputParams.input_buffer[inputParams.input_current_data_index] = (CAN1->sFIFOMailBox[0].RDHR & 0x000000FF);
			inputParams.input_current_data_index++;
		}

		if(bytes_in_frame > 5)
		{
			inputParams.input_buffer[inputParams.input_current_data_index] = ((CAN1->sFIFOMailBox[0].RDHR & 0x0000FF00)>>8);
			inputParams.input_current_data_index++;
		}

		if(bytes_in_frame > 6)
		{
			inputParams.input_buffer[inputParams.input_current_data_index] = ((CAN1->sFIFOMailBox[0].RDHR & 0x00FF0000)>>16);
			inputParams.input_current_data_index++;
		}

		if(bytes_in_frame > 7)
		{
			inputParams.input_buffer[inputParams.input_current_data_index] = ((CAN1->sFIFOMailBox[0].RDHR & 0xFF000000)>>24);
			inputParams.input_current_data_index++;
		}

		// release rx mailbox
		CAN1->RF0R |= (RF0R_RFOM0);

		// update number of available messages
		numMsgInFifo = (CAN1->RF0R & 0x03);
	}

	// if last segment -> indicate by returning nonzero
	if(total_num_segments == (cur_segment_num + 1))
	{
		inputParams.input_buffer_length = inputParams.input_current_data_index--;
		clear_input_transmission_params();
		return CAN_TMS_COMPLETE;
	}
	return CAN_TMS_INCOMPLETE;
}

static void can_handle_tx_irq()
{
	// send next data if applicable
	if(outputParams.output_transmission_open == 1)
	{
		can_send_next_data();
	}
	else
	{
		// clear flags
		CAN1->TSR |= (TSR_RQCP0);
		CAN1->TSR |= (TSR_RQCP1);
		CAN1->TSR |= (TSR_RQCP2);
	}
}

can_result_t can_set_callback(CanCallback* callback)
{
	cCallback = callback;
	return CAN_OK;
}

can_result_t can_enable_loop_back_mode()
{
	// enable CAN initialization mode and wait for acknowledgment
	CAN1->MCR |= (MCR_INRQ);
	while (!(CAN1->MSR & (MSR_INAK))){}

	// set loop back mode
	CAN1->BTR |= (BTR_LBKM);

	// exit CAN initialization mode and wait for the INAK bit to be cleared
	CAN1->MCR &= ~(MCR_INRQ);
	while (CAN1->MSR & MSR_INAK){}

	return CAN_OK;
}

static void can_handle_rx_irq()
{
	if(can_receive_data() != 0)
	{
		if(cCallback != NULL)
		{
			if(cCallback->dataReceived != NULL)
			{
				CanTransmissionData tData =
				{
					.data = inputParams.input_buffer,
					.dataSize = inputParams.input_buffer_length,
					.stdID = inputParams.input_message_std_id
				};
				cCallback->dataReceived(&tData);
			}
		}
	}
}

static void can_handle_sce_irq()
{
	// get last error code
	unsigned int errorCode =
			((CAN1->ESR & (1U<<4))<<0) |
			((CAN1->ESR & (1U<<5))<<1) |
			((CAN1->ESR & (1U<<6))<<2);

	//error_code = ((CAN1->ESR >> 4) & 0x07); // the same as above..

	// clear the error bit in MSR register
	CAN1->MSR &= ~(MSR_ERRI);

	// 001 = 1 -> stuff error
	// 010 = 2 -> form error
	// 011 = 3 -> acknowledgment error
	// 100 = 4 -> bit recessive error
	// 101 = 5 -> bit dominant error
	// 110 = 6 -> crc error
	// 111 = 7 -> set by software

	if(cCallback != NULL)
	{
		if(cCallback->errorOccurred != NULL)
		{
			cCallback->errorOccurred(errorCode);
		}
	}
}

void CAN1_TX_IRQHandler()
{
	can_handle_tx_irq();
}

void CAN1_RX0_IRQHandler()
{
	can_handle_rx_irq();
}

void CAN1_SCE_IRQHandler()
{
	can_handle_sce_irq();
}








