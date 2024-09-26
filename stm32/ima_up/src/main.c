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


int enderecos = 5000;
int main(void)
{
	Utility_Init();		//inicializa o sistema de clock e fun��es de temporiza��o
	USART1_Init();
	SPI1_W25Q16_Init();
	Delay_ms(2000);
	Chip_Erase();

	USART1->DR = 170;
	//while (!(USART1->SR & USART_SR_TXE));
	while (1) {
		//while (!(USART1->SR & ))
		//Page_Program(enderecos, USART1->DR);
		//enderecos++;

	}
	//for (int i=0; i<691200; i++){
		//while( !( USART1->SR) );
		//printf("%d\n", (int) USART1->DR);
		//Page_Program(i, USART1->DR);
	//}


}

void USART1_IRQHandler(void) {

	//while( !(USART1->SR & USART_SR_RXNE) );
	Page_Program(enderecos, USART1->DR);
	USART1->DR = Read_Data(enderecos);
	while (!(USART1->SR & USART_SR_TXE));
	enderecos++;
	//printf("%d\n", (char) Read_Data(enderecos));

}
