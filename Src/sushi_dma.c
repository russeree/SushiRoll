/**
 * @Auth: Reese Russell
 * @Desc: Sushi Board DMA REQUESTS AND MEM Management
 */

#include "sushi_dma.h"

DMA_HandleTypeDef pulseGenOnDMATimer;
DMA_HandleTypeDef pulseGenOffDMATimer;
DMA_HandleTypeDef sushiUART1rx;
DMA_HandleTypeDef sushiUART1tx;

uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
uint8_t DMA_TX_Buffer[UART_BUFFER_SIZE];

/**
 * @desc: Setups the DMA transfer for pulsing the timer from a Value in Memory
 */
uint32_t swOn[1]  = {0x001B};       //For use by DMA to push this onto the BSSR registers gpio A0,1,3,4 are on
uint32_t swOff[1] = {0x001B << 16}; //For use by DMA to push this onto the BSSR registers gpio A0,1,3,4 are off

extern UART_HandleTypeDef sushiUART; //External instance of UART to link to the  DMA channels 4/5

/**
 * @Desc: Checks for the status of the DMA object to ensure it has been initialized;
 */
SushiStatus SAL_SAFE_DMA_Abort(DMA_HandleTypeDef *DMA_Handler){
	if (HAL_DMA_GetState(DMA_Handler) > HAL_DMA_STATE_BUSY){
		HAL_DMA_Abort(DMA_Handler);
	}
	return SushiSuccess;
}

/* Enable Circular PWM modes, This is not based on the Trigger Style DMA which uses a normal mode */
SushiStatus dmaPWMenableTimer1(void){
	SAL_SAFE_DMA_Abort(&pulseGenOnDMATimer);
	SAL_SAFE_DMA_Abort(&pulseGenOffDMATimer);
	//Lets do Some Fun Stuff Here... DAM CC1 and 2 Events -> send the data to the BSSR registers for a pulse on and off
	pulseGenOnDMATimer.Init.Mode                 = DMA_CIRCULAR;
	//Setup the DMA Turn off timer Parameters
	pulseGenOffDMATimer.Init.Mode                = DMA_CIRCULAR;
	//Fire up the DMA ENGINE
	HAL_DMA_DeInit(&pulseGenOnDMATimer);  //Why de-init? Maybe to make sure all registers are reset
	HAL_DMA_Init(&pulseGenOnDMATimer);    //Init with the DMA Update.....
	HAL_DMA_DeInit(&pulseGenOffDMATimer); //Why de-init? Maybe to make sure all registers are reset
	HAL_DMA_Init(&pulseGenOffDMATimer);   //Init with the DMA Update.....
	HAL_NVIC_DisableIRQ(DMA1_Channel2_3_IRQn); //Now enable the Interupt
	return SushiSuccess;
}

/* Enable Circular PWM modes, This is not based on the Trigger Style DMA which uses a normal mode */
SushiStatus dmaTriggerEnableTimer1(void){
	SAL_SAFE_DMA_Abort(&pulseGenOnDMATimer);
	SAL_SAFE_DMA_Abort(&pulseGenOffDMATimer);
	//Lets do Some Fun Stuff Here... DAM CC1 and 2 Events -> send the data to the BSSR registers for a pulse on and off
	pulseGenOnDMATimer.Init.Mode                 = DMA_NORMAL;
	//Setup the DMA Turn off timer Parameters
	pulseGenOffDMATimer.Init.Mode                = DMA_NORMAL;
	//Fire up the DMA ENGINE
	HAL_DMA_DeInit(&pulseGenOnDMATimer);  //Why de-init? Maybe to make sure all registers are reset
	HAL_DMA_Init(&pulseGenOnDMATimer);    //Init with the DMA Update.....
	HAL_DMA_DeInit(&pulseGenOffDMATimer); //Why de-init? Maybe to make sure all registers are reset
	HAL_DMA_Init(&pulseGenOffDMATimer);   //Init with the DMA Update.....
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn); //Now enable the Interupt
	return SushiSuccess;
}

// DMA Timer Initialize
SushiStatus advTimerDMAinit(void){
	__HAL_RCC_DMA1_CLK_ENABLE();
	SAL_SAFE_DMA_Abort(&pulseGenOnDMATimer);     //This is used for the initial DMA configuration and clearing - DMA starts in a
	SAL_SAFE_DMA_Abort(&pulseGenOffDMATimer);
	//Setup the DMA Turn on  timer Parameters
	pulseGenOnDMATimer.Instance                  = DMA1_Channel2;
	pulseGenOnDMATimer.Init.Direction            = DMA_MEMORY_TO_PERIPH;
	pulseGenOnDMATimer.Init.MemDataAlignment     = DMA_MDATAALIGN_WORD;
	pulseGenOnDMATimer.Init.MemInc               = DMA_MINC_DISABLE;
	pulseGenOnDMATimer.Init.Mode                 = DMA_NORMAL;
	pulseGenOnDMATimer.Init.PeriphDataAlignment  = DMA_PDATAALIGN_WORD;
	pulseGenOnDMATimer.Init.PeriphInc            = DMA_PINC_DISABLE;
	pulseGenOnDMATimer.Init.Priority             = DMA_PRIORITY_HIGH;
	//Setup the DMA Turn off timer Parameters
	pulseGenOffDMATimer.Instance                 = DMA1_Channel3;
	pulseGenOffDMATimer.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	pulseGenOffDMATimer.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
	pulseGenOffDMATimer.Init.MemInc              = DMA_MINC_DISABLE;
	pulseGenOffDMATimer.Init.Mode                = DMA_NORMAL;
	pulseGenOffDMATimer.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	pulseGenOffDMATimer.Init.PeriphInc           = DMA_PINC_DISABLE;
	pulseGenOffDMATimer.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
	//Fire up the DMA ENGINE
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 2, 0); //Second highest priority  level.
	return SushiSuccess;
}

/* This is the DMA interupt configuration for sushiboard UART TX and RX*/
void sushiBoardUARTDMAInit(void){
	__HAL_RCC_DMA1_CLK_ENABLE();
	//Setup the DMA Turn on  timer Parameters
	sushiUART1tx.Instance                  = DMA1_Channel4;
	sushiUART1tx.Init.Direction            = DMA_MEMORY_TO_PERIPH;
	sushiUART1tx.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
	sushiUART1tx.Init.MemInc               = DMA_MINC_ENABLE;
	sushiUART1tx.Init.Mode                 = DMA_NORMAL;
	sushiUART1tx.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
	sushiUART1tx.Init.PeriphInc            = DMA_PINC_DISABLE;
	sushiUART1tx.Init.Priority             = DMA_PRIORITY_LOW;
	//DMA RX SETUP AND CONFIGURATION
	sushiUART1rx.Instance                  = DMA1_Channel5;
	sushiUART1rx.Init.Direction            = DMA_PERIPH_TO_MEMORY;
	sushiUART1rx.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
	sushiUART1rx.Init.MemInc               = DMA_MINC_ENABLE;
	sushiUART1rx.Init.Mode                 = DMA_CIRCULAR;
	sushiUART1rx.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
	sushiUART1rx.Init.PeriphInc            = DMA_PINC_DISABLE;
	sushiUART1rx.Init.Priority             = DMA_PRIORITY_MEDIUM;
	/*Fire Up the DMA Engine for the UART RX and TX channels */
	HAL_DMA_DeInit(&sushiUART1tx);  //Why de-init? Maybe to make sure all registers are reset
	HAL_DMA_Init(&sushiUART1tx);    //Init with the DMA Update.....
	__HAL_DMA_REMAP_CHANNEL_ENABLE(DMA_REMAP_USART1_TX_DMA_CH4);
	HAL_DMA_DeInit(&sushiUART1rx); //Why de-init? Maybe to make sure all registers are reset
	HAL_DMA_Init(&sushiUART1rx);   //Init with the DMA Update.....
	__HAL_DMA_REMAP_CHANNEL_ENABLE(DMA_REMAP_USART1_RX_DMA_CH5);
	__HAL_LINKDMA(&sushiUART, hdmatx, sushiUART1tx);
	__HAL_LINKDMA(&sushiUART, hdmarx, sushiUART1rx);
	/*Enable the Standard Interupts */
	HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);
}
