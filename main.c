#include "hw.h"
#include "hal.h"
//#include "mysensor.h"
#include "outsens.h"
#include "sump.h"
#include "ds1820.h"
#include "rfm12b.h"
#include "test.h"
#include "logging.h"
#include "menu2.h"

#define NULL (void*)0

char gVersion[] = "V0.0.6\r\n\000";

void     SystemClock_Config(void);

#define DELAY 100

uint32_t gEvents;
uint8_t gState;
uint8_t gCounter;

#define TEMP_COUNT 3
uint16_t temp[TEMP_COUNT];
rfm12b rfm1;
rfm12b rfm2;

ds1820_search_t search_data;

//------------------------------------------------------------------------------
void state_change()
{
    uint8_t buffer[] = { 'h', 'e', 'l', 'l', 'o' };

    switch (gState) {
    case 0:
	ds1820_measure(PIN_DS1820a);
	//set_LED1;
	break;
    case 2:
	temp[0] = ds1820_read_temp(PIN_DS1820a);
	//clr_LED1;
	break;
    case 3:
	//uart_sends(mysensor_set(1,1, V_TEMP, temp[0]));
	uart_sends(UART, outsens_set(gCounter, temp, TEMP_COUNT));
	gCounter++;
	break;
    case 4:
	rfm12b_send(&rfm1, buffer, sizeof buffer);
	break;
    }
    gState++;
    if (gState > 10)
	gState = 0;
}


void cmdVersion()
{
    uart_sends(UART, gVersion);
    uart_nl(UART);
}

void cmdScan()
{
    uint8_t i;

    // reset the search
    search_data.lastDiscrepancy = 0;
    search_data.lastDeviceFlag = FALSE;
    search_data.lastFamilyDiscrepancy = 0;
    
    while (ds1820_search(PIN_DS1820a, &search_data)) {

	uart_sends(UART, "ROM_CODE: ");
	for (i=0; i<8; i++) {
	    uart_hex8(UART, search_data.romNo[i]);
	    if (i != 7)
		uart_send(UART, ':');
	}
	uart_sends(UART, "\r\n");
    }
}

//------------------------------------------------------------------------------
int main()
{
    uint8_t irq2_state;
    context_t context;
    
    gEvents = 0;
    gState = 0;
    irq2_state = 0;
    SystemClock_Config();

    io_init();
    uart_init(UART);
    timer2_init();
    log_init(UART);
    
    uart_sends(UART, "starting..");

    ds1820_init(PIN_DS1820a);
    rfm12b_init(&rfm1, 1, PIN_RFM12_SEL1, PIN_RFM12_IRQ1);
    rfm12b_tx(&rfm1, 1);
    uart_sends(UART, "rf1..");
    
    rfm12b_init(&rfm2, 2, PIN_RFM12_SEL2, PIN_RFM12_IRQ2);
    rfm12b_tx(&rfm2, 0);
    uart_sends(UART, "rf2");
    
    outsens_init();
    //mysensor_init();

    temp[1] = 2;
    temp[2] = 3;
    
    uart_sends(UART, "r\n");
    //uart_sends(mysensor_present(1,1, S_TEMP));

    /*while (1) {
	io_set(PIN_RFM12_SEL1);
	io_set(PIN_LED1);
	delay_us(2000);
	io_clear(PIN_RFM12_SEL1);
	io_clear(PIN_LED1);
	delay_us(2000);
    }*/

    context.state = menu_root;
    context.action = NULL;

    (*(context.state))(EVENT_PROMPT, &context);
    
    while (1) {
	if (gEvents & EV_TIMER2) {
	    tgl_LED2;
	    //state_change();

	    gEvents &= ~(EV_TIMER2);
	}
	if (gEvents & EV_UART1_RX) {
	    //LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);

	    //sump_handle(gUartRx1);
	    //menu_select(gMainMenu, gUart1Rx);
	    (*(context.state))(gUart1Rx, &context);
	    if (gUart1Rx == EVENT_HELP) {
		uart_nl(UART);
		(*(context.state))(EVENT_PROMPT, &context);
	    }
	    
	    gEvents &= ~(EV_UART1_RX);
	}
	if (gEvents & EV_UART2_RX) {
	    //menu_select(gMainMenu, gUart2Rx);
	    
	    gEvents &= ~(EV_UART2_RX);
	}
	if (gEvents & EV_SPI1_RX) {
	    //command(gSpi1Rx);
	    
	    gEvents &= ~(EV_SPI1_RX);
	}
	if (gEvents & EV_SPI2_RX) {
	    //command(gSpi2Rx);
	    
	    gEvents &= ~(EV_SPI2_RX);
	}
	if ((irq2_state == 0) && (io_read(PIN_RFM12_IRQ2) == 0)) {
	    uart_sends(UART, "irq2");
	    irq2_state = 1;
	}
	else {
	    if (irq2_state) {
		uart_sends(UART, ".\r\n");
	    }
	    irq2_state = 0;
	}
    }
}

//------------------------------------------------------------------------------
void error(error_t code)
{
    uart_sends(UART, "ERROR!");
    
    while (1) {
	set_LED1;
	set_LED2;
	LL_mDelay(20);

	clr_LED1;
	clr_LED2;
	LL_mDelay(20);
    }
}

//------------------------------------------------------------------------------
void SystemClock_Config(void)
{
#ifdef stm32f0
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1);

    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
				LL_RCC_PLL_MUL_12);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    LL_Init1msTick(48000000);

    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

    LL_SetSystemCoreClock(48000000);

    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, 3);
    
#else
    LL_RCC_HSE_EnableBypass();
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE,
				LL_RCC_PLLM_DIV_8,
				400,
				LL_RCC_PLLP_DIV_4);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    SysTick_Config(100000000 / 1000);  // 1ms

    //SystemCoreClock = 168000000;
    SystemCoreClock = 100000000;
#endif
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    uart_sends("ASSERT ERROR in ");
    uart_sends(file);
    uart_send_nl();

    while (1);
}
#endif
