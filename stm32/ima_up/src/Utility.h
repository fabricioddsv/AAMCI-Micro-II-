/*
 * Utility.h
 *
 *  Created on: 8 de jul de 2023
 *      Author: Prof. Dr. Fagner de Araujo Pereira
 */

#ifndef UTILITY_H_
#define UTILITY_H_

//Declara��es de fun��es �teis
void Utility_Init(void);			//inicializa��o de fun��es da biblioteca
void Configure_Clock(void);			//configura��o do sistema de clock
void TIM2_Setup(void);				//configura��o do Timer2 como base de tempo de 1us
void Delay_us(uint32_t delay);		//atraso em us
void Delay_ms(uint32_t delay);		//atraso em ms
void USART1_Init(void);				//configura��o da USART1 para debug com printf


//Inicializa��o de fun��es da biblioteca
void Utility_Init(void)
{
	Configure_Clock();	//inicializa o sistema de clock
	TIM2_Setup();		//configura o Timer2 como base de tempo de 1us
}

//Configura��o do sistema de clock para velocidade m�xima em todos os barramentos
//cristal externo de 8MHz e HCLK de 168 MHz
void Configure_Clock(void)
{
	//Par�metros do PLL principal
	#define PLL_M	4
	#define PLL_N	168
	#define PLL_P	2
	#define PLL_Q	7

	//Reseta os registradores do m�dulo RCC para o estado inicial
	RCC->CIR = 0;				//desabilita todas as interrup��es de RCC
	RCC->CR |= RCC_CR_HSION;	//liga o oscilador HSI
	RCC->CFGR = 0;				//reseta o registrador CFGR
	//Desliga HSE, CSS e o PLL e o bypass de HSE
	RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON |
			   RCC_CR_PLLON | RCC_CR_HSEBYP);
	RCC->PLLCFGR = 0x24003010;	//reseta o registrador PLLCFGR

	//Configura a fonte de clock (HSE), os par�metros do PLL,
	//prescalers dos barramentos AHB, APB
	RCC->CR |= RCC_CR_HSEON;				//habilita HSE
	while(!((RCC->CR) & RCC_CR_HSERDY));	//espera HSE ficar pronto
    RCC->CFGR |= 0x9400;	//HCLK = SYSCLK/1, PCLK2 = HCLK/2, PCLK1 = HCLK/4

    //Configura a fonte de clock e os par�metros do PLL principal
    RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
                   (0x400000)           | (PLL_Q << 24);

    RCC->CR |= RCC_CR_PLLON;			//habilita o PLL
    while(!(RCC->CR & RCC_CR_PLLRDY));	//espera o PLL ficar pronto verificando a flag PLLRDY

    RCC->CFGR |= 0x2;					//seleciona o PLL como fonte de SYSCLK
    while((RCC->CFGR & 0xC) != 0x8);	//espera o PLL ser a fonte de SYSCLK

    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;    //habilita o clock do RNG
    RNG->CR |= RNG_CR_RNGEN;
}

//Configura o timer 2 como base de tempo de 1us
void TIM2_Setup(void)
{
	//O modo padr�o do contador � com contagem crescente
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;	//liga o clock do Timer2
	TIM2->PSC = 83;						//prescaler para incrementos a cada 1uS
	TIM2->EGR = TIM_EGR_UG;				//update event para escrever o valor do prescaler
	TIM2->CR1 |= TIM_CR1_CEN;			//habilita o timer
}

//Cria��o de atraso em us
void Delay_us(uint32_t delay)
{
	TIM2->CNT = 0;				//inicializa o contador com 0
	while(TIM2->CNT < delay);	//aguarda o tempo passar
}

//Cria��o de atraso em ms
void Delay_ms(uint32_t delay)
{
	uint32_t max = 1000*delay;
	TIM2->CNT = 0;				//inicializa o contador com 0
	while(TIM2->CNT < max);		//aguarda o tempo passar
}

//Configura��o b�sica da USART1
void USART1_Init(void)
{
	//configura��o da USART1
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;				//habilita o clock da USART1
	USART1->BRR = 84;									//ajusta baud rate para 1 Mbps (frequ�ncia do perif�rico de 84MHz)
	//O estado default do registrador USART1->CR1 garante:
	//1 stop bit, 8 bits de dados, sem bit de paridade,
	//oversampling de 16 amostras por bit
	USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE |		//habilita o trasmissor e o receptor
					USART_CR1_RXNEIE | USART_CR1_UE);	//habilita interrup��o de RX e a USART1
	//USART1->CR1 |= (USART_CR1_TE | USART_CR1_UE);		//habilita apenas o trasmissor

	//Habilita a interrup��o da USART1 no NVIC
	USART1->CR1 |= USART_CR1_RXNEIE;
	NVIC_SetPriority(USART1_IRQn, 0);	//seta a prioridade da USART1
	NVIC_EnableIRQ(USART1_IRQn);		//habilita a interrup��o da USART1

	//Configura��o dos pinos PA9 (TX) e PA10(RX)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			//habilita o clock do GPIOA
	GPIOA->MODER |= (0b10 << 20) | (0b10 << 18) ;	//pinos PA10 e PA9 como fun��o alternativa
	GPIOA->AFR[1] |= (0b0111 << 8) | (0b0111 << 4);	//fun��o alternativa 7 (USART1)

}

//Redefini��o da fun��o de envio de dados pela USART1
int __io_putchar(int ch)
{
	USART1->DR = ch;						//transmite o dado
	while (!(USART1->SR & USART_SR_TXE));	//espera pelo fim da transmiss�o
	return ch;
}

//Redefini��o da fun��o de recebimento de dados pela USART1
int __io_getchar(void)
{
   return (uint16_t)(USART1->DR);	//l� o dado recebido
}

//ISR da USART1. Todas as ISR's est�o definidas no arquivo startup_stm32.s
/*void USART1_IRQHandler(void)
{
	__io_putchar(__io_getchar());	//l� o dado e reenvia pela USART1
}*/

#endif /* UTILITY_H_ */
