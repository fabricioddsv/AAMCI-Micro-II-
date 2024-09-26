/*
 * W25Q16.h
 *
 *  Created on: 13 de ago de 2022
 *      Author: Fagner
 */

#ifndef W25Q16_H_
#define W25Q16_H_

//Protótipos de funções de acesso à memória Flash W25Q16
void SPI1_W25Q16_Init(void);				//inicialização da interface SPI1 para o W25Q16
uint8_t Read_Status_Register1(void);		//leitura do registrador de status 1
void Write_Enable(void);					//habilita a escrita
void Sector_Erase(uint32_t address);		//apaga o conteúdo de um setor 4kB
void _32kB_Block_Erase(uint32_t address);	//apagamento de bloco de 32 kB
void _64kB_Block_Erase(uint32_t address);	//apagamento de bloco de 64 kB
void Chip_Erase(void);						//apaga o conteúdo do chip
uint8_t Read_Data(uint32_t address);									//leitura de um byte
void Read_MData(uint32_t address, uint8_t *buffer, uint32_t size);		//leitura de múltiplos bytes
void Page_Program(uint32_t address, uint8_t data);						//escrita de um byte
void MPage_Program(uint32_t address, uint8_t *buffer, uint8_t size);	//escrita de múltiplos bytes
uint64_t Read_Unique_ID_Number(void);									//leitura do ID único de 64 bits

//Declaração de funções de acesso à memória Flash W25Q16
//Inicialização da interface SPI1 para a memória
void SPI1_W25Q16_Init()
{
	//inicializando a interface SPI
	//configurando os pinos PB0 como saída CS e (PB3, PB4 e PB5) da interface SPI no modo de função alternativa
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;	//habilita o clock do GPIOB
	GPIOB->ODR |= 1;						//pino PB0 inicialmente em nível alto
	//modo de saída em PB0(CS) e função alternativa (SPI1) em PB3, PB4 e PB5 em alta velocidade
	GPIOB->MODER |= (0b10 << 10) | (0b10 << 8) | (0b10 << 6) | (0b01) ;
	GPIOB->OSPEEDR |= (0b11 << 10) | (0b11 << 8) | (0b11 << 6) | (0b11);
	GPIOB->AFR[0] |= (0b0101 << 20) | (0b0101 << 16) | (0b0101 << 12);

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	//habilita o clock da interface SPI1
	//modo 0, 8 bits de dados, bit mais significativo transmitido primeiro, baud rate 42 Mbps
	//pino SS controlado por software, modo master e habilitação da interface
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_SPE;
}

//Leitura do registrador de status 1
uint8_t Read_Status_Register1(void)
{
	GPIOB->ODR &= ~1;					//pino CS em nível baixo (inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));  	//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x05;					//comando de leitura do status register 1 (05h)
	while(!(SPI1->SR & SPI_SR_RXNE));	//aguarda o byte fictício do escravo
	(void)SPI1->DR;						//lê o byte fictício
	SPI1->DR = 0xFF;					//envia byte fictício para receber o valor do registrador 1
	while(!(SPI1->SR & SPI_SR_RXNE));	//aguarda o valor ser recebido
	GPIOB->ODR |= 1;					//pino CS em nível alto (encerra o comando)
	return SPI1->DR;					//retorna o valor do registrador de status 1
}

//habilitação de escrita na memória
void Write_Enable(void)
{
	while(Read_Status_Register1() & 1);			//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;							//(inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));			//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x06;							//comando para habilitação de escrita (06h)
	while(!(SPI1->SR & SPI_SR_RXNE));			//aguarda o byte fictício do escravo
	(void)SPI1->DR;								//lê o byte fictício
	GPIOB->ODR |= 1;							//(encerra o comando)
	while(!(Read_Status_Register1() & 0b10));	//aguarda a escrita estar habilitada
}

//apagamento de setor
void Sector_Erase(uint32_t address)
{
	Write_Enable();							//habilita a escrita na memória
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//(inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x20;						//envia comando para apagar o setor (20h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	GPIOB->ODR |= 1;						//(encerra o comando)
}

//apagamento de bloco de 32 kB
void _32kB_Block_Erase(uint32_t address)
{
	Write_Enable();							//habilita a escrita na memória
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//(inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x52;						//envia comando para apagar o bloco (52h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	GPIOB->ODR |= 1;						//(encerra o comando)
}

//apagamento de bloco de 64 kB
void _64kB_Block_Erase(uint32_t address)
{
	Write_Enable();							//habilita a escrita na memória
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//faz o pino CS ir para nível baixo (inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0xD8;						//envia comando para apagar o bloco (D8h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	GPIOB->ODR |= 1;						//(encerra o comando)
}

//apagamento completo do chip
void Chip_Erase(void)
{
	Write_Enable();						//habilita a escrita na memória
	while(Read_Status_Register1() & 1);	//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;					//faz o pino CS ir para nível baixo (inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));	//aguarda o buffer Tx estar vazio
	SPI1->DR = 0xC7;					//envia comando para apagar o chip (C7h)
	while(!(SPI1->SR & SPI_SR_RXNE));	//aguarda o byte fictício do escravo
	(void)SPI1->DR;						//lê o byte fictício
	GPIOB->ODR |= 1;					//(encerra o comando)
}

//leitura de um byte na memória
uint8_t Read_Data(uint32_t address)
{
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//(inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x03;						//comando para ler um dado (03h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	uint8_t RX = SPI1->DR;					//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	RX = SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	RX = SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	RX = SPI1->DR;							//lê o byte fictício

	SPI1->DR = 0xFF;						//envia um byte fictício
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte armazenado na memória ser recebido
	RX = SPI1->DR;							//lê o byte gravado na memória
	GPIOB->ODR |= 1;						//(encerra o comando)

	return RX;								//retorna com o byte armazenado
}

//leitura de múltiplos bytes na memória
void Read_MData(uint32_t address, uint8_t *buffer, uint32_t size)
{
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//(inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x03;						//comando para ler um dado (03h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	uint32_t contador=0;
	while(size)
	{
		SPI1->DR = 0xFF;						//envia um byte fictício
		while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte de dado ser recebido
		buffer[contador] = SPI1->DR;			//lê o byte gravado na memória
		++contador;
		--size;
	}

	GPIOB->ODR |= 1;	//(encerra o comando)
}

//gravação de um byte na memória
void Page_Program(uint32_t address, uint8_t data)
{
	Write_Enable();							//habilita a escrita na memória
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//(inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x02;						//comando para escrever um dado (02h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = data;						//envia o dado a ser gravado
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	GPIOB->ODR |= 1;						//(encerra o comando)
}

//gravação de múltiplos bytes na memória
void MPage_Program(uint32_t address, uint8_t *buffer, uint8_t size)
{
	Write_Enable();							//habilita a escrita na memória
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//pino CS em nível baixo (inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x02;						//envia comando para escrever um dado (02h)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF0000) >> 16;	//envia o byte 3 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF00) >> 8;		//envia o byte 2 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = (address & 0xFF);			//envia o byte 1 do endereço
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	uint8_t contador=0;
	while(size)
	{
		SPI1->DR = buffer[contador];		//envia o byte de dado a ser gravado
		while(!(SPI1->SR & SPI_SR_RXNE));	//recebe o byte fictício
		(void)SPI1->DR;						//lê o byte fictício
		++contador;
		--size;
	}

	GPIOB->ODR |= 1;	//(encerra o comando)
}

//leitura do ID único de 64 bits
uint64_t Read_Unique_ID_Number(void)
{
	while(Read_Status_Register1() & 1);		//aguarda a memória estar disponível
	GPIOB->ODR &= ~1;						//pino CS em nível baixo (inicia o comando)
	while(!(SPI1->SR & SPI_SR_TXE));		//aguarda o buffer Tx estar vazio
	SPI1->DR = 0x4B;						//comando para ler o ID de 64 bits (4Bh)
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício do escravo
	(void)SPI1->DR;							//lê o byte fictício

	SPI1->DR = 0xFF;						//envia o byte fictício 1
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício
	SPI1->DR = 0xFF;						//envia o byte fictício 2
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício
	SPI1->DR = 0xFF;						//envia o byte fictício 3
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício
	SPI1->DR = 0xFF;						//envia o byte fictício 4
	while(!(SPI1->SR & SPI_SR_RXNE));		//aguarda o byte fictício
	(void)SPI1->DR;							//lê o byte fictício

	uint64_t ID = 0;			//variável que vai receber o ID
	uint8_t cont = 1;
	while(cont <= 8)
	{
		SPI1->DR = 0xFF;							//envia um byte fictício
		while(!(SPI1->SR & SPI_SR_RXNE));			//aguarda um byte do ID ser recebido
		ID |= ((uint64_t)SPI1->DR << (8*(8-cont)));	//lê o byte do ID
		++cont;
	}

	GPIOB->ODR |= 1;	//(encerra o comando)
	return ID;			//retorna com o ID
}

#endif /* W25Q16_H_ */
