/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "Utility.h"
#include "W25Q16.h"

int enderecos = 0;

int main(void)
{
	Utility_Init();		//inicializa o sistema de clock e fun��es de temporiza��o
	USART1_Init();
	SPI1_W25Q16_Init();
	Delay_ms(2000);
	USART1->DR = 170;

	for (int i=5000; i<696200; i++){
		USART1->DR = Read_Data(i);
		while (!(USART1->SR & USART_SR_TXE));
	}

	/*USART1->DR = 170;
	while (!(USART1->SR & USART_SR_TXE));

	//for (int i=0;i<tam; i++){
	while(1){
		USART1->DR = Read_Data(enderecos);
		while (!(USART1->SR & USART_SR_TXE));
		enderecos++;
		/*USART1->DR = 218;
		while (!(USART1->SR & USART_SR_TXE));
		USART1->DR = 165;
		while (!(USART1->SR & USART_SR_TXE));
		USART1->DR = 32;
		while (!(USART1->SR & USART_SR_TXE));*/
		/*if (enderecos==691200) break;

	}*/

    return 0;
}
