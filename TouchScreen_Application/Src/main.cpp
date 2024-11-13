#include "main.hpp"

uint32_t SystemCoreClock = MCU_CLOCK;

TaskHandle_t displayTaskHandle;
TaskHandle_t canTaskHandle;
TaskHandle_t touchTaskHandle;

QueueHandle_t canReceptionQueue;
QueueHandle_t displayInstructionQueue;

void display_task(void * params);
void touch_task(void * params);
void can_task(void * params);

class ApplicationCallback : public IApplicationCallback
{
public:
	void enqueueDisplayInstruction(void * instruction) override
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(displayInstructionQueue, instruction, &pxHigherPriorityTaskWoken);
	}
	void enqueueCanData(void * data) override
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(canReceptionQueue, data, &pxHigherPriorityTaskWoken);
	}
	void handleCriticalError(const char * msg) override
	{
		taskDISABLE_INTERRUPTS(); for( ;; );
	}
};

Application * appInstance;
ApplicationCallback applicationCallback;

int tempCounter = 0;

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
	appInstance->init(GetCanControllerInstance(), GetDisplayControllerInstance(), GetTouchControllerInstance(), GetLoggerInstance());
	appInstance->registerCallback(&applicationCallback);

	canReceptionQueue = xQueueCreate(5, sizeof(uint8_t*));
	configASSERT(canReceptionQueue != NULL);

	displayInstructionQueue = xQueueCreate(5, sizeof(uint32_t*));
	configASSERT(displayInstructionQueue != NULL);

	status = xTaskCreate(display_task, "dsp task", 4000, NULL, 1, &displayTaskHandle);
	configASSERT(status == pdPASS);

	status = xTaskCreate(touch_task, "touch task", 1000, NULL, 2, &touchTaskHandle);
	configASSERT(status == pdPASS);

	status = xTaskCreate(can_task, "can task", 2000, NULL, 3, &canTaskHandle);
	configASSERT(status == pdPASS);

	vTaskStartScheduler();

	while(1)
	{}
}

void display_task(void * params)
{
	void * instruction;

	while(1)
	{
		if(xQueueReceive(displayInstructionQueue, &instruction, portMAX_DELAY) == pdPASS)
		{
			appInstance->processDisplayTask(instruction);
		}
	}
}

void touch_task(void * params)
{
	while(1)
	{
		appInstance->processTouchTask();
		vTaskDelay(pdMS_TO_TICKS(30));
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




