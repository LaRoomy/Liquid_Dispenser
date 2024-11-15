#include "main.hpp"

uint32_t SystemCoreClock = MCU_CLOCK;

TaskHandle_t flowControlTaskHandle;
TaskHandle_t canTaskHandle;

QueueHandle_t canReceptionQueue;

void flow_control_task(void * params);
void can_task(void * params);

class ApplicationCallback : public IApplicationCallback
{
public:
	void enqueueCanData(void * data) override
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(canReceptionQueue, data, &pxHigherPriorityTaskWoken);
	}
	void startFlowOperationTask() override
	{
		BaseType_t pxHigherPriorityTaskWoken = 0;
		xTaskNotifyFromISR(flowControlTaskHandle, 0, eNoAction, &pxHigherPriorityTaskWoken);
	}
	void handleCriticalError(const char * msg) override
	{
		taskDISABLE_INTERRUPTS(); for( ;; );
	}
};

ApplicationCallback applicationCallback;
Application * appInstance = nullptr;

int main(void)
{
	// enable FPU
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));

	BaseType_t status;

	vInitPrioGroupValue();

	CPU_Clock_Config();

	appInstance = Application::GetInstance();
	if(appInstance == nullptr)
	{
		taskDISABLE_INTERRUPTS(); for( ;; );
	}
	appInstance->enableVerboseLogging();

	// inject the controller instances
	appInstance->init(GetCanControllerInstance(), GetFlowControllerInstance(), GetLoggerInstance());
	appInstance->registerCallback(&applicationCallback);

	canReceptionQueue = xQueueCreate(5, sizeof(uint8_t*));
	configASSERT(canReceptionQueue != NULL);

	status = xTaskCreate(flow_control_task, "fctrl task", 1000, NULL, 2, &flowControlTaskHandle);
	configASSERT(status == pdPASS);

	status = xTaskCreate(can_task, "can task", 2000, NULL, 3, &canTaskHandle);
	configASSERT(status == pdPASS);

	vTaskStartScheduler();

	while(1)
	{}
}

void flow_control_task(void * params)
{
	while(1)
	{
		if(xTaskNotifyWait(0x0, ULONG_MAX, NULL, portMAX_DELAY) == pdTRUE)
		{
			appInstance->processFlowControlOperation();
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




