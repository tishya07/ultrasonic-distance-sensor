/*
 * ECE 153B
 *
 * Name(s):
 * Section:
 * Lab: 3C
 */
 
#include <stdio.h> 
 
#include "stm32l476xx.h"

uint32_t volatile currentValue = 0;
uint32_t volatile lastValue = 0;
uint32_t volatile overflowCount = 0;
uint32_t volatile timeInterval = 0;
uint32_t volatile distance;

void Input_Capture_Setup() {
	//1 set up PB6
	//1.a
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;	//enable gpio port B
	
	//1.b
	GPIOB->MODER &= ~GPIO_MODER_MODER6;
	GPIOB->MODER |= GPIO_MODER_MODER6_1;	//set to alternative function
	
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL6;		//clear alt func for PB6
	GPIOB->AFR[0] |= GPIO_AFRL_AFSEL6_1;	//configure PB6 for TIM4_CH1
	
	//1.c
	GPIOB->PUPDR &=~(1UL<<12);						//set to no pull-up, no pull-down
	
	//2
	RCC->APB1ENR1|= RCC_APB1ENR1_TIM4EN; 	//enable timer 4
	
	//3
	TIM4->PSC = 15;												//prescaler to 15
	
	//4
	TIM4->CR1 |= TIM_CR1_ARPE;						//enable auto relod preload
	TIM4->ARR = 0xFFFF;										//set to max value 2^16
	
	//5
	TIM4->CCMR1 &= ~(TIM_CCMR1_CC1S);
	TIM4->CCMR1 |= TIM_CCMR1_CC1S_0;			//input capture bits mapped to timer input 1;
	
	//6
	TIM4->CCER |= ~TIM_CCER_CC1NP;				//capture both rising/falling edge
	TIM4->CCER |= TIM_CCER_CC1E;					//enable capturing
	
	//7
	TIM4->DIER |= TIM_DIER_UDE;						//enable DMA requests
	TIM4->DIER |= TIM_DIER_UIE;						//enable update interrupt
	
	//8
	TIM4->EGR |= TIM_EGR_UG;							//enable update generation
	
	//9
	TIM4->SR &= ~(TIM_SR_UIF);						//clear update interrupt flag
	
	//10
	TIM1->CR1 |= ~TIM_CR1_DIR;						//set direction to up counter
	TIM1->CR1 |= TIM_CR1_CEN;							//enable counter
	
	//11
	NVIC_EnableIRQ(TIM4_IRQHandler); 
	NVIC_SetPriority(TIM4_IRQHandler, 2);
}

void TIM4_IRQHandler(void) {
	
	uint32_t count=1; //1 if rising edge, 0 if falling edge
	
	if (TIM4->SR & TIM_SR_CC1IF){
		
		if (count){
				lastValue = (TIM4->CCR1); //set rise to the CCR value
				TIM4->SR &=~(TIM_SR_CC1IF); //clearing flag
				count = 0;
			
			} else {
				currentValue = (TIM4->CCR1); //set fall to the CCR value
				TIM4->SR &=~(TIM_SR_CC1IF); //clearing flag
				timeInterval = (currentValue + overflowCount*(0xFFFF+1) - lastValue); 
				count = 1;
				
			}		
		}
	
		if (count == 0){
			
			if (TIM4->SR & TIM_SR_UIF){
				overflowCount ++;
				TIM4->SR &=~(TIM_SR_UIF); //clearing flag
				
			}
		} else {
				overflowCount = 0;
				
		}
}

void Trigger_Setup() {
	//1 set up PA9
	//1.a
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;	//enable gpio port A
	
	//1.b
	GPIOA->MODER &= ~GPIO_MODER_MODER9;
	GPIOA->MODER |= GPIO_MODER_MODER9_1;	//set to alternative function
	
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFRH1;		//clear alt func for PA9
	GPIOA->AFR[1] |= GPIO_AFRH_AFSEL9_0;	//configure PA9 for TIM1_CH2
	
	//1.c
	GPIOA->PUPDR &=~(1UL<<18);						//set to no pull-up, no pull-down
	
	//1.d
	GPIOA->OTYPER &= ~(1UL<<9);						//set output type to push-pull
	
	//1.e
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED9);
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED9;	//set to very high output speed
	
	//2
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; 	//enable timer 1
	
	//3
	TIM1->PSC = 15;												//prescaler to 15
	
	//4
	TIM1->CR1 |= TIM_CR1_ARPE;						//enable auto relod preload
	TIM1->ARR = 0xFFFF;										//set to max value 2^16
	
	//5
	TIM1->CCR2 = 10;											//clock period = 1us, 10us needed to activate sensor
	
	//6
	TIM1->CCMR1 &= ~(TIM_CCMR1_OC2M);
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;	//set to 110 PWM Mode 1
	
	TIM1->CCMR1 |= TIM_CCMR1_OC2PE;				//enable output compare preload
	
	//7
	TIM1->CCER |= TIM_CCER_CC2E;					//enable output in CCER
	
	//8
	TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI;	//set main output enable, off-state selection for run mode, and off-state selection for idle mode
	
	//9
	TIM1->EGR |= TIM_EGR_UG;							//enable update generation 
	
	//10
	TIM1->DIER |= TIM_DIER_UIE;						//enable update interrupt
	TIM1->SR &= ~(TIM_SR_UIF);						//clear update interrupt flag
	
	//11
	TIM1->CR1 |= TIM_CR1_DIR;							//set direction to down counter
	TIM1->CR1 |= TIM_CR1_CEN;							//enable counter
}

int main(void) {	
	// Enable High Speed Internal Clock (HSI = 16 MHz)
	RCC->CR |= RCC_CR_HSION;
	while ((RCC->CR & RCC_CR_HSIRDY) == 0); // Wait until HSI is ready
	
	// Select HSI as system clock source 
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	while ((RCC->CFGR & RCC_CFGR_SWS) == 0); // Wait until HSI is system clock source
  
	// Input Capture Setup
	Input_Capture_Setup();
	
	// Trigger Setup
	Trigger_Setup();

	
	while(1) {
		// [TODO] Store your measurements on Stack
		distance = timeInterval / 58;
		
	}
}
