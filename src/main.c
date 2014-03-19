#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"
#include "stm32_p103.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

/* Filesystem includes */
#include "filesystem.h"
#include "fio.h"
#include "romfs.h"

#include "clib.h"
#include "shell.h"

/* _sromfs symbol can be found in main.ld linker script
 * it contains file system structure of test_romfs directory
 */
extern const unsigned char _sromfs;

static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
/* Add for serial input */
volatile xQueueHandle serial_rx_queue = NULL;

/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */
void USART2_IRQHandler()
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;

	/* If this interrupt is for a transmit... */
	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
		/* "give" the serial_tx_wait_sem semaphore to notfiy processes
		 * that the buffer has a spot free for the next byte.
		 */
		xSemaphoreGiveFromISR(serial_tx_wait_sem, &xHigherPriorityTaskWoken);

		/* Diables the transmit interrupt. */
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		/* If this interrupt is for a receive... */
	}else if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		char msg = USART_ReceiveData(USART2);

		/* If there is an error when queueing the received byte, freeze! */
		if(!xQueueSendToBackFromISR(serial_rx_queue, &msg, &xHigherPriorityTaskWoken))
			while(1);
	}
	else {
		/* Only transmit and receive interrupts should be enabled.
		 * If this is another type of interrupt, freeze.
		 */
		while(1);
	}

	if (xHigherPriorityTaskWoken) {
		taskYIELD();
	}
}

void send_byte(char ch)
{
	/* Wait until the RS232 port can receive another byte (this semaphore
	 * is "given" by the RS232 port interrupt when the buffer has room for
	 * another byte.
	 */
	while (!xSemaphoreTake(serial_tx_wait_sem, portMAX_DELAY));

	/* Send the byte and enable the transmit interrupt (it is disabled by
	 * the interrupt).
	 */
	USART_SendData(USART2, ch);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

char recv_byte()
{
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	char msg;
	while(!xQueueReceive(serial_rx_queue, &msg, portMAX_DELAY));
	return msg;
}
void command_prompt(void *pvParameters)
{
	char buf[128];
	char *argv[20];
	fio_printf(1, "\rWelcome to FreeRTOS Shell\r\n");
	while(1){
		fio_printf(1, "\r>>");
		fio_read(0, buf, 127);
	
		int n=parse_command(buf, argv);

		/* will return pointer to the command function */
		cmdfunc *fptr=do_command(argv[0]);
		if(fptr!=NULL)
			fptr(n, argv);
		else
			fio_printf(2, "\r\n\"%s\" command not found.\r\n", argv[0]);
	}

}

int main()
{
	init_rs232();
	enable_rs232_interrupts();
	enable_rs232();
	
	fs_init();
	fio_init();
	
	register_romfs("romfs", &_sromfs);
	
	/* Create the queue used by the serial task.  Messages for write to
	 * the RS232. */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	/* Add for serial input 
	 * Reference: www.freertos.org/a00116.html */
	serial_rx_queue = xQueueCreate(1, sizeof(char));

	/* Create a task to output text read from romfs. */
	xTaskCreate(command_prompt,
	            (signed portCHAR *) "Command Prompt",
	            512 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL);

	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

void vApplicationTickHook()
{
}
