#include <stm32f30x.h>
#include "rfm73.h"
#include <stdio.h>

#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever is needed */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}

void clocks_init()
{
	//HSE i zegar na 72MHz
	RCC->CR = RCC_CR_HSION;
	while(!(RCC->CR & RCC_CR_HSIRDY)){};
	FLASH->ACR |= FLASH_ACR_LATENCY_0;
	RCC->CFGR = (RCC_CFGR_PLLMULL9 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PLLSRC_HSI_Div2);
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY)){};
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));

	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
	GPIOE->MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;
	GPIOE->ODR |= GPIO_ODR_8;
		
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~GPIO_MODER_MODER1;
	//GPIOB->PUPDR |= GPIO_PUPDR_PUPDR1_0;
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
		
		
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;	
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PB;
	EXTI->FTSR |= EXTI_FTSR_TR1;
	EXTI->IMR |= EXTI_IMR_MR1;	
	NVIC_EnableIRQ(EXTI1_IRQn);
	

	
}

unsigned char length = 0;
unsigned char rx_buf[32];

void EXTI1_IRQHandler(void){
  
	uint8_t status = rfm73_register_read(RFM73_REG_STATUS);
	if(status & (1 << 6)) //RX data
	{
		rfm73_receive(rx_buf, &length);
		rfm73_register_write(RFM73_REG_STATUS, status);
		
		for(int i = 0; i < length; i++)
			printf("%c", rx_buf[i]);
	}
	
	EXTI->PR |= EXTI_PR_PR1;	
	GPIOE->ODR ^= GPIO_ODR_8;
	
	
}


const unsigned char tx_buf[17]={
       0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
       0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};

int main(void)
{
	clocks_init();
	rfm73_init();


	if(rfm73_is_present())
			GPIOE->ODR |= GPIO_ODR_9;
	
	rfm73_mode_receive();


    while(1)
    {
	     
    }
}

