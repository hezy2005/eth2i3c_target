/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    eth2i3c_target/Src/main.c
 * @author  MCD Application Team
 * @brief   This sample code shows how to use STM32H5xx I3C LL API as Target
 *          to receive and transmit a data buffer
 *          with a communication process based on Interrupt transfer.
 *          The communication is done using 2 Boards.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I3C1_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

    /* System interrupt init*/
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_I3C1_Init();
    MX_ICACHE_Init();
    /* USER CODE BEGIN 2 */
    I3CS_Init();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5)
    {
    }

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
    while (LL_PWR_IsActiveFlag_VOS() == 0)
    {
    }
    LL_RCC_HSE_EnableBypass();
    LL_RCC_HSE_SetExternalClockType(LL_RCC_HSE_DIGITAL_TYPE);
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1)
    {
    }

    LL_RCC_PLL1_SetSource(LL_RCC_PLL1SOURCE_HSE);
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
    LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    LL_RCC_PLL1_SetM(4);
    LL_RCC_PLL1_SetN(250);
    LL_RCC_PLL1_SetP(2);
    LL_RCC_PLL1_SetQ(2);
    LL_RCC_PLL1_SetR(2);
    LL_RCC_PLL1P_Enable();
    LL_RCC_PLL1_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL1_IsReady() != 1)
    {
    }

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
    {
    }

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

    LL_Init1msTick(250000000);

    LL_SetSystemCoreClock(250000000);
}

/**
 * @brief I3C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I3C1_Init(void)
{

    /* USER CODE BEGIN I3C1_Init 0 */

    /* USER CODE END I3C1_Init 0 */

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_RCC_SetI3CClockSource(LL_RCC_I3C1_CLKSOURCE_PCLK1);

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I3C1);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    /**I3C1 GPIO Configuration
    PB8   ------> I3C1_SCL
    PB9   ------> I3C1_SDA
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_8 | LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

    GPIO_InitStruct.Alternate = LL_GPIO_AF_3;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I3C1 interrupt Init */
    NVIC_SetPriority(I3C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(I3C1_EV_IRQn);
    NVIC_SetPriority(I3C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(I3C1_ER_IRQn);

    /* USER CODE BEGIN I3C1_Init 1 */

    /* USER CODE END I3C1_Init 1 */

    /** I3C Initialization
     */
    LL_I3C_SetMode(I3C1, LL_I3C_MODE_TARGET);
    LL_I3C_SetAvalTiming(I3C1, 0x000000f8);

    /** Configure FIFO
     */
    LL_I3C_SetRxFIFOThreshold(I3C1, LL_I3C_RXFIFO_THRESHOLD_1_4);
    LL_I3C_SetTxFIFOThreshold(I3C1, LL_I3C_TXFIFO_THRESHOLD_1_4);
    LL_I3C_DisableControlFIFO(I3C1);
    LL_I3C_DisableStatusFIFO(I3C1);

    /** Configure Target
     */
    LL_I3C_SetDeviceCharacteristics(I3C1, 0xC6);
    LL_I3C_SetMIPIInstanceID(I3C1, DEVICE_ID);
    LL_I3C_DisableControllerRoleReq(I3C1);
    LL_I3C_DisableHotJoin(I3C1);
    LL_I3C_DisableIBI(I3C1);
    LL_I3C_SetDeviceIBIPayload(I3C1, LL_I3C_IBI_NO_ADDITIONAL_DATA);
    LL_I3C_ConfigNbIBIAddData(I3C1, LL_I3C_PAYLOAD_1_BYTE);

    /* Let the Controller negotiate MWL and MRL without a Target-side transfer limit. */
    LL_I3C_SetMaxReadLength(I3C1, 0);
    LL_I3C_SetMaxWriteLength(I3C1, 0);

    LL_I3C_SetDeviceCapabilityOnBus(I3C1, LL_I3C_DEVICE_ROLE_AS_TARGET);
    LL_I3C_SetGrpAddrHandoffSupport(I3C1, LL_I3C_HANDOFF_GRP_ADDR_NOT_SUPPORTED);
    LL_I3C_SetDataTurnAroundTime(I3C1, LL_I3C_TURNAROUND_TIME_TSCO_LESS_12NS);
    LL_I3C_SetMiddleByteTurnAround(I3C1, 0);
    LL_I3C_SetDataSpeedLimitation(I3C1, LL_I3C_NO_DATA_SPEED_LIMITATION);
    LL_I3C_SetMaxDataSpeedFormat(I3C1, LL_I3C_GETMXDS_FORMAT_1);
    LL_I3C_SetHandoffActivityState(I3C1, LL_I3C_HANDOFF_ACTIVITY_STATE_0);
    LL_I3C_SetControllerHandoffDelayed(I3C1, LL_I3C_HANDOFF_NOT_DELAYED);
    LL_I3C_SetPendingReadMDB(I3C1, LL_I3C_MDB_NO_PENDING_READ_NOTIFICATION);

    /** Enable the selected I3C peripheral
     */
    LL_I3C_Enable(I3C1);
    /* USER CODE BEGIN I3C1_Init 2 */
    LL_I3C_EnableIT_DAUPD(I3C1);
    LL_I3C_EnableIT_FC(I3C1);
    LL_I3C_EnableIT_RXFNE(I3C1);
    LL_I3C_EnableIT_TXFNF(I3C1);
    LL_I3C_EnableIT_ERR(I3C1);
    /* USER CODE END I3C1_Init 2 */
}

/**
 * @brief ICACHE Initialization Function
 * @param None
 * @retval None
 */
static void MX_ICACHE_Init(void)
{

    /* USER CODE BEGIN ICACHE_Init 0 */

    /* USER CODE END ICACHE_Init 0 */

    /* USER CODE BEGIN ICACHE_Init 1 */

    /* USER CODE END ICACHE_Init 1 */

    /** Enable instruction cache (default 2-ways set associative cache)
     */
    LL_ICACHE_Enable();
    /* USER CODE BEGIN ICACHE_Init 2 */

    /* USER CODE END ICACHE_Init 2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the LL error return state */
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
