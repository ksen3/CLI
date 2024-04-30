#include "tty.h"
#include "string.h"
#include "shell.h"

#define TTY_RX_BUF_SIZE (128U)		///< Receiver buffer maximum size
#define TTY_TX_BUF_SIZE (128U)		///< Transmitter buffer maximum size

//#define DBG

uint8_t tty_tx_buffer[TTY_TX_BUF_SIZE] = {0,};		// TODO: delete?
uint8_t tty_rx_buffer[TTY_RX_BUF_SIZE] = {0,};		///<Buffer for current message
uint8_t *cmd_string_ptr = tty_rx_buffer - 1;		///< Pointer to first symbol receiver buffer
int8_t tty_rx_buffer_size = 0;						///< Stores size of receiver buffer in bytes

static uint8_t byte_buff = 0;		///< Buffer for one byte
static uint8_t backspace = 0x7F;	///< Code of backspace in ASCII

//static char* greetings =
//	"\r\n\t..######..##.......####\
//	 \r\n\t.##....##.##........##.\
//	 \r\n\t.##.......##........##.\
//	 \r\n\t.##.......##........##.\
//	 \r\n\t.##.......##........##.\
//	 \r\n\t.##....##.##........##.\
//	 \r\n\t..######..########.####\r\n\n";

/**
 * @brief
 * DMA settings for one-byte information exchange
 */
void dma_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;		//Clock on DMA

	//
	//		DMA RX
	//
	DMA1_Channel6->CCR = 0;
	DMA1_Channel6->CPAR = (uint32_t)(&(USART2->DR));
	DMA1_Channel6->CMAR = (uint32_t)(&(byte_buff));
	DMA1_Channel6->CNDTR = 1;	//bytes number
	DMA1_Channel6->CCR = 0x1082;	//TODO: refactor magic numbers


	//
	//	DMA TX
	//
	DMA1_Channel7->CCR = 0;
	DMA1_Channel7->CPAR = (uint32_t)(&(USART2->DR));
	DMA1_Channel7->CMAR = (uint32_t)(&(byte_buff));
	DMA1_Channel7->CNDTR = 1;	//bytes number
	DMA1_Channel7->CCR = 0x1090;	//TODO: refactor magic numbers

	//--------------------------
	DMA1->IFCR = 0xFFFFFFFF;
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);

	DMA1_Channel6->CCR |= 1;		//Enable DMA RX stream

	memset(tty_rx_buffer, 0, TTY_RX_BUF_SIZE);	//clear rx buffer
	printBuffer("> ");
}

/**
 * @brief
 * Waits one byte from keyboard via DMA
 *
 */
void waitChar(void)
{
	DMA1_Channel6->CCR = 0;
	//while((DMA1_Channel6->CCR&DMA_CCR_EN)){;}	//wait until EN bit resets

	DMA1_Channel6->CMAR = (uint32_t)(&(byte_buff));
	DMA1_Channel6->CNDTR = 1;		//bytes number
	DMA1_Channel6->CCR = 0x1082;	//TODO: refactor magic number

	DMA1->IFCR = 0xFFFFFFFF;
	//DMA1->ISR = DMA_ISR_TCIF7;	//RX transfer complete interrupt enable
	DMA1_Channel6->CCR |= 1;		//Enable DMA RX stream
}

/**
 * @brief
 * Transmits one byte to the screen
 */
void printChar(void)
{
	DMA1_Channel7->CCR = 0;
	//while((DMA1_Channel7->CCR&DMA_CCR_EN)){;}	//wait until EN bit resets

	DMA1_Channel7->CMAR = (uint32_t)(&(byte_buff));
	DMA1_Channel7->CNDTR = 1;		//bytes number
	DMA1_Channel7->CCR = 0x1090;	//TODO: refactor magic number

	DMA1_Channel7->CCR |= 1;		//Enable DMA TX stream
}

/**
 * @brief
 * This function deletes certain quantity of symbols on the screen
 *
 * @param[in] uint8_t symbols
 * Defines number of symbols will be deleted
 *
 * @return
 * None
 */
void deleteFromScreen(uint8_t symbols)
{
	DMA1_Channel7->CCR = 0;
	//while((DMA1_Channel7->CCR&DMA_CCR_EN)){;}	//wait until EN bit resets

	DMA1_Channel7->CMAR = (uint32_t)(&(backspace));
	DMA1_Channel7->CNDTR = symbols;	//bytes number
	DMA1_Channel7->CCR = 0x1010;	//TODO: refactor magic number

	DMA1_Channel7->CCR |= 1;		//Enable DMA TX stream
}

/**
 * @brief
 * Print message to the screen
 *
 * @param[in] const char *buffer
 * Pointer to string to be output to the screen
 *
 * @return
 *
 */
int8_t printBuffer(const char *buffer)
{
	uint8_t size = strlen(buffer);
	if(NULL == buffer)
		return -1;	//TODO: Argument error

	if(0 == size)
		return -1;	//TODO: argument error

	DMA1_Channel7->CCR = 0;
	//while((DMA1_Channel7->CCR&DMA_CCR_EN)){;}	//wait until EN bit resets

	DMA1_Channel7->CMAR = (uint32_t)(buffer);
	DMA1_Channel7->CNDTR = size;	//bytes number
	DMA1_Channel7->CCR = 0x1090;	//TODO: refactor magic number

	DMA1_Channel7->CCR |= 1;		//Enable DMA TX stream

	//while((DMA1->ISR & DMA_ISR_TCIF7) == 0){;}	//Wait while DMA1_Channel7 transfer doesn't complete
	for(int z=0; z<30000; z++){__NOP();}	//TODO: refactor delay. Do not remove before the issue isn't solved
	return 0;
}

/**
 * 	@brief DMA RX channel haldler
 *
 * 	It called when one byte from keyborad was received
 */
void DMA1_Channel6_IRQHandler()
{
	DMA1->IFCR = 0xFFFFFFFF;

	if(byte_buff == 0x7F)		//	Backspace was pressed
	{
		if(cmd_string_ptr >= tty_rx_buffer)	//if pointer to the end on string greater of equal than first byte address in rx buffer
		{
			*(cmd_string_ptr--) = '\0';	//delete byte
			tty_rx_buffer_size--;
		}
		printChar();
	}
	else if(byte_buff == '\r')	//	Enter was pressed
	{
		printBuffer("\r\n");

#ifdef TTY_ECHO_MODE
		char answ[200] = {0};
		sprintf(answ,
				"-------------\r\n"
				"ECHO: %s\r\n"
				"LEN: %d\r\n"
				"-------------\r\n",
				tty_rx_buffer,
				strlen(tty_rx_buffer));
		printBuffer(answ);
#endif

		shell_processing(tty_rx_buffer);

		printBuffer("\r\n> ");
		memset(tty_rx_buffer, 0, TTY_RX_BUF_SIZE);	//clear rx buffer
		cmd_string_ptr = tty_rx_buffer-1;	//carrige return to start of buffer

	}
	else if(byte_buff == 'A')	//	'A' or 'arrow up' was pressed
	{

		*(++cmd_string_ptr) = byte_buff;
		tty_rx_buffer_size++;

		if(strstr(tty_rx_buffer, "\e[A") != NULL) // 'arrow up' pressed processing
		{
			memset(tty_rx_buffer, 0, TTY_RX_BUF_SIZE);	//clear rx buffer
			cmd_string_ptr = tty_rx_buffer-1;	//carrige return to start of buffer


			if(tty_rx_buffer_size != 0)
				deleteFromScreen(tty_rx_buffer_size);
			for(int del=0; del<=30000; del++){__NOP();}

			s_getCmd();
		}
		else	// 'A' pressed processing
		{
			printChar();
		}
	}
	else if(byte_buff == 'B')	//	'B' or 'arrow down' was pressed
	{
		//store in buffer
		*(++cmd_string_ptr) = byte_buff;
		tty_rx_buffer_size++;


		if(strstr(tty_rx_buffer, "\e[B") != NULL) // 'arrow down' pressed processing
		{
			//printBuffer("Arrow DOWN");
			memset(tty_rx_buffer, 0, TTY_RX_BUF_SIZE);	//clear rx buffer
			cmd_string_ptr = tty_rx_buffer-1;	//carrige return to start of buffer
		}
		else	// 'B' pressed processing
		{
			printChar();
		}
	}
	else if(byte_buff == '\e')
	{
		*(++cmd_string_ptr) = byte_buff;
		tty_rx_buffer_size++;
	}
	else if(byte_buff == '[')
	{
		*(++cmd_string_ptr) = byte_buff;
		tty_rx_buffer_size++;

		if(strstr(tty_rx_buffer, "\e[") != NULL)
		{
			__NOP();
		}
		else
		{
			printChar();
		}

	}
	else						//	Any key was pressed
	{

		*(++cmd_string_ptr) = byte_buff;
		tty_rx_buffer_size++;
		printChar();
	}

	//printChar();
	waitChar();
}


