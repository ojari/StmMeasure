#include "hw.h"
#include "hal.h"

#define NULL (void*)0

uint8_t gUartRx;

typedef struct IoPinTable {
    GPIO_TypeDef *port;
    int pin;
} IoPinTable_t;

IoPinTable_t PINMAP[] = {
    [PIN_LED1]      = { GPIOC, LL_GPIO_PIN_8 },
    [PIN_LED2]      = { GPIOC, LL_GPIO_PIN_9 },
    [PIN_DS1820a]   = { GPIOB, LL_GPIO_PIN_3 },
    [PIN_DS1820b]   = { GPIOB, LL_GPIO_PIN_4 },
    [PIN_DS1820c]   = { GPIOB, LL_GPIO_PIN_5 },
    [PIN_RFM12_SEL] = { GPIOA, LL_GPIO_PIN_4 },
};

#define MAX_PINMAP sizeof PINMAP / sizeof PINMAP[0]


//------------------------------------------------------------------------------
void delay_us(uint16_t time)
{
    volatile uint16_t counter = 2 * time;
    while(counter--) {
	asm("nop");
    }
}

void pin_alt(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t alternate, uint32_t pull)
{
    LL_GPIO_SetPinMode(GPIOx, pin, LL_GPIO_MODE_ALTERNATE);
    if (pin < LL_GPIO_PIN_8)
	LL_GPIO_SetAFPin_0_7(GPIOx, pin, alternate);
    else
	LL_GPIO_SetAFPin_8_15(GPIOx, pin, alternate);

    if (pull != LL_GPIO_PULL_NO)
	LL_GPIO_SetPinOutputType(GPIOx, pin, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinSpeed(GPIOx, pin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinPull(GPIOx, pin, pull);
}

//------------------------------------------------------------------------------
void io_init()
{
#ifdef stm32f0
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
     
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);

    pin_alt(GPIOA, LL_GPIO_PIN_5,  LL_GPIO_AF_0, LL_GPIO_PULL_NO);  // SPI1 SCK
    pin_alt(GPIOA, LL_GPIO_PIN_6,  LL_GPIO_AF_0, LL_GPIO_PULL_NO);  // SPI1 MISO
    pin_alt(GPIOA, LL_GPIO_PIN_7,  LL_GPIO_AF_0, LL_GPIO_PULL_NO);  // SPI1 MOSI

    pin_alt(GPIOB, LL_GPIO_PIN_13, LL_GPIO_AF_0, LL_GPIO_PULL_NO);  // SPI2 SCK
    pin_alt(GPIOB, LL_GPIO_PIN_14, LL_GPIO_AF_0, LL_GPIO_PULL_NO);  // SPI2 MISO
    pin_alt(GPIOB, LL_GPIO_PIN_15, LL_GPIO_AF_0, LL_GPIO_PULL_NO);  // SPI2 MOSI
 
    pin_alt(GPIOA, LL_GPIO_PIN_9,  LL_GPIO_AF_1, LL_GPIO_PULL_UP);  // UART1 TX
    pin_alt(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_1, LL_GPIO_PULL_UP);  // UART1 RX
 
    pin_alt(GPIOA, LL_GPIO_PIN_2,  LL_GPIO_AF_1, LL_GPIO_PULL_UP);  // UART2 TX
    pin_alt(GPIOA, LL_GPIO_PIN_3,  LL_GPIO_AF_1, LL_GPIO_PULL_UP);  // UART2 RX
#endif
#ifdef stm32f4
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_14, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_15, LL_GPIO_MODE_OUTPUT);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    pin_alt(GPIOA, LL_GPIO_PIN_2,  LL_GPIO_AF_7, LL_GPIO_PULL_UP);  // UART TX
    pin_alt(GPIOA, LL_GPIO_PIN_3,  LL_GPIO_AF_7, LL_GPIO_PULL_UP);  // UART RX

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    pin_alt(GPIOB, LL_GPIO_PIN_3,  LL_GPIO_AF_5, LL_GPIO_PULL_NO);  // SPI SCK
    pin_alt(GPIOB, LL_GPIO_PIN_4,  LL_GPIO_AF_5, LL_GPIO_PULL_NO);  // SPI MISO
    pin_alt(GPIOB, LL_GPIO_PIN_5,  LL_GPIO_AF_5, LL_GPIO_PULL_NO);  // SPI MOSI
#endif

#ifdef stm32f7
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_OUTPUT);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

    pin_alt(GPIOD, LL_GPIO_PIN_8,  LL_GPIO_AF_7, LL_GPIO_PULL_UP);  // UART TX
    pin_alt(GPIOD, LL_GPIO_PIN_9,  LL_GPIO_AF_7, LL_GPIO_PULL_UP);  // UART RX
#endif
}

void io_mode(pin_t pin, uint8_t mode)
{
    if (mode == IO_MODE_INPUT) {
	LL_GPIO_SetPinMode(PINMAP[pin].port,
			   PINMAP[pin].pin,
			   LL_GPIO_MODE_INPUT);
    }
    else {
	LL_GPIO_SetPinMode(PINMAP[pin].port,
			   PINMAP[pin].pin,
			   LL_GPIO_MODE_OUTPUT);
    }
}

void io_set(pin_t pin)
{
    LL_GPIO_SetOutputPin(PINMAP[pin].port, PINMAP[pin].pin);
}

void io_clear(pin_t pin)
{
    LL_GPIO_ResetOutputPin(PINMAP[pin].port, PINMAP[pin].pin);
}

uint8_t io_read(pin_t pin)
{
    return LL_GPIO_IsInputPinSet(PINMAP[pin].port, PINMAP[pin].pin);
}

    



#ifdef stm32f0
#define UART_PORT USART1
#endif
#ifdef stm32f4
#define UART_PORT USART2
#endif
#ifdef stm32f7
#define UART_PORT ((USART_TypeDef *) USART3_BASE)
#endif

//------------------------------------------------------------------------------
USART_TypeDef *uart_get(uint8_t port)
{
    USART_TypeDef *ret = NULL;
    
    switch (port) {
    case 1:
	ret = USART1;
	break;
    case 2:
	ret = USART2;
	break;
#ifdef stm32f7
    case 3:
	ret = USART3;
	break;
    case 4:
	ret = UART4;
	break;
#endif
    default:
	error(ERR_UART_PORT);
	break;
    }
    return ret;
}

//------------------------------------------------------------------------------
void USART1_IRQHandler()
//void USART3_IRQHandler()
{
  if (LL_USART_IsActiveFlag_RXNE(UART_PORT) && LL_USART_IsEnabledIT_RXNE(UART_PORT)) {
    /* RXNE flag will be cleared by reading of RDR register (done in call) */
    /* Call function in charge of handling Character reception */
    //USART_CharReception_Callback();
      gUartRx = LL_USART_ReceiveData8(UART_PORT);

      //uart_send(ch);
      
      gEvents |= EV_UART_RX;
  }
}


//------------------------------------------------------------------------------
void uart_init(uint8_t port)
{
    //uint32_t pclk;
    
#ifdef stm32f0
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
    //NVIC_SetPriority(USARTx_IRQn, 0);  
    //NVIC_EnableIRQ(USARTx_IRQn);
    NVIC_SetPriority(USART1_IRQn, 0);  
    NVIC_EnableIRQ(USART1_IRQn);
#endif
#ifdef stm32f4
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
#endif
#ifdef stm32f7
    NVIC_SetPriority(USART3_IRQn, 0);  
    NVIC_EnableIRQ(USART3_IRQn);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
#endif

    LL_USART_SetTransferDirection(UART_PORT,
				  LL_USART_DIRECTION_TX_RX);

    LL_USART_ConfigCharacter(UART_PORT,
			     LL_USART_DATAWIDTH_8B,
			     LL_USART_PARITY_NONE,
			     LL_USART_STOPBITS_1);

    //pclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART1_CLKSOURCE);

    LL_USART_SetBaudRate(UART_PORT,
			 SystemCoreClock/1,       // for stm32f0 1 others 2
			 LL_USART_OVERSAMPLING_16,
//			 9600);
			 115200); 

    LL_USART_Enable(UART_PORT);

    LL_USART_ConfigAsyncMode(USART1); // for stm32f0
    
    LL_USART_EnableIT_RXNE(UART_PORT);
    LL_USART_EnableIT_ERROR(UART_PORT);
}

//------------------------------------------------------------------------------
void uart_send(uint8_t port, char ch)
{
    LL_USART_TransmitData8(UART_PORT, ch);
}

//------------------------------------------------------------------------------
void uart_sends(uint8_t port, char *buf)
{
    for (uint8_t i = 0; buf[i] != 0; i++) {
	while (!LL_USART_IsActiveFlag_TXE(UART_PORT));
	
	LL_USART_TransmitData8(UART_PORT, buf[i]);	
    }
}

//------------------------------------------------------------------------------
void uart_send_nl(uint8_t port)
{
    uart_send(port, '\r');
    //uart_send('\n');
}

//------------------------------------------------------------------------------
void TIM2_IRQHandler(void)
{
    if(LL_TIM_IsActiveFlag_UPDATE(TIM2) == 1) {
	LL_TIM_ClearFlag_UPDATE(TIM2);
    }

    //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_14);
    gEvents |= EV_TIMER2;
}

//------------------------------------------------------------------------------
void timer2_init()
{
    uint32_t clock, reload;
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    LL_TIM_SetPrescaler(TIM2, __LL_TIM_CALC_PSC(SystemCoreClock, 10000));

    clock = SystemCoreClock/1;

    reload = __LL_TIM_CALC_ARR(clock, LL_TIM_GetPrescaler(TIM2), 1);
    LL_TIM_SetAutoReload(TIM2, reload);

    LL_TIM_EnableIT_UPDATE(TIM2);
    
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);

    LL_TIM_EnableCounter(TIM2);
    LL_TIM_GenerateEvent_UPDATE(TIM2);
}

//------------------------------------------------------------------------------
SPI_TypeDef *spi_port(uint8_t port)
{
    SPI_TypeDef *ret = NULL;

    switch (port) {
    case 1:
	ret = SPI1;
	break;
    case 2:
	ret = SPI2;
	break;
    default:
	error(ERR_SPI_PORT);
	break;
    }
    return ret;
}


//------------------------------------------------------------------------------
void spi_init(uint8_t port)
{
    SPI_TypeDef *spi = spi_port(port);

    switch (port) {
    case 1:
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SPI1);
	break;
    case 2:
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
	break;
    }

    LL_SPI_Enable(spi);
    LL_SPI_SetMode(spi, LL_SPI_MODE_MASTER);
    LL_SPI_SetClockPhase(spi, LL_SPI_PHASE_1EDGE);
    LL_SPI_SetClockPolarity(spi, LL_SPI_POLARITY_LOW);
    LL_SPI_SetBaudRatePrescaler(spi, LL_SPI_BAUDRATEPRESCALER_DIV4);
    LL_SPI_SetTransferBitOrder(spi, LL_SPI_MSB_FIRST);
    LL_SPI_SetTransferDirection(spi, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_16BIT);
    LL_SPI_SetNSSMode(spi, LL_SPI_NSS_HARD_INPUT);
    
    LL_SPI_Enable(spi);
}

void spi_write(uint8_t port, uint16_t data)
{
    LL_SPI_TransmitData16(spi_port(port), data);
}

uint16_t spi_read(uint8_t port)
{
    return LL_SPI_ReceiveData16(spi_port(port));
}

//------------------------------------------------------------------------------
/** Hard Fault exception
 */
void HardFault_Handler(void)
{
    error(0);
}
