#include "main.hpp"

uint32_t SystemCoreClock = MCU_CLOCK;

TaskHandle_t nfcTaskHandle;
TaskHandle_t canTaskHandle;

QueueHandle_t canReceptionQueue;
QueueHandle_t nfcReceptionQueue;

void nfc_task(void * params);
void can_task(void * params);

class ApplicationCallback : public IApplicationCallback
{
public:
	void enqueueCanData(void * data) override
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(canReceptionQueue, data, &pxHigherPriorityTaskWoken);
	}
	void enqueueNfcData(void * data) override
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(nfcReceptionQueue, data, &pxHigherPriorityTaskWoken);
	}
	void onCriticalError(const char * msg) override
	{
		taskDISABLE_INTERRUPTS(); for( ;; );
	}
};

Application * appInstance;
ApplicationCallback applicationCallback;

int main(void)
{
	// enable FPU
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));

	BaseType_t status;

	vInitPrioGroupValue();

	CPU_Clock_Config();
	init_board_led();

	appInstance = Application::GetInstance();
	if(appInstance == nullptr)
	{
		taskDISABLE_INTERRUPTS(); for( ;; );
	}
	appInstance->enableVerboseLogging();

	// inject the controller instances
	appInstance->init(GetNfcControllerInstance(), GetCanControllerInstance(), GetLoggerInstance());
	appInstance->registerCallback(&applicationCallback);


	canReceptionQueue = xQueueCreate(5, sizeof(uint8_t*));
	configASSERT(canReceptionQueue != NULL);

	nfcReceptionQueue = xQueueCreate(5, sizeof(char*));
	configASSERT(nfcReceptionQueue != NULL);

	status = xTaskCreate(nfc_task, "nfc task", 2000, NULL, 3, &nfcTaskHandle);
	configASSERT(status == pdPASS);

	status = xTaskCreate(can_task, "can task", 2000, NULL, 3, &canTaskHandle);
	configASSERT(status == pdPASS);

	vTaskStartScheduler();

	while(1)
	{}
}

void nfc_task(void * params)
{
	void * data;

	while(1)
	{
		if(xQueueReceive(nfcReceptionQueue, &data, portMAX_DELAY) == pdPASS)
		{
			appInstance->processNfcTask(data);
		}
	}
}

void can_task(void * params)
{
	void * data;

	while(1)
	{
		if(xQueueReceive(canReceptionQueue, &data, portMAX_DELAY) == pdPASS)
		{
			appInstance->processCanTask(data);
		}
	}
}



