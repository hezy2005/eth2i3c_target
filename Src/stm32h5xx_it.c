/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    eth2i3c_target/Src/stm32h5xx_it.c
 * @author  MCD Application Team
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
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
#include "stm32h5xx_it.h"
#include <assert.h>
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1)
    {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
    /* USER CODE BEGIN BusFault_IRQn 0 */

    /* USER CODE END BusFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
    /* USER CODE BEGIN UsageFault_IRQn 0 */

    /* USER CODE END UsageFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
    /* USER CODE BEGIN SVCall_IRQn 0 */

    /* USER CODE END SVCall_IRQn 0 */
    /* USER CODE BEGIN SVCall_IRQn 1 */

    /* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
    /* USER CODE BEGIN PendSV_IRQn 1 */

    /* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    /* USER CODE BEGIN SysTick_IRQn 0 */

    /* USER CODE END SysTick_IRQn 0 */

    /* USER CODE BEGIN SysTick_IRQn 1 */

    /* USER CODE END SysTick_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#define TARGET_MEMORY_BANK_SIZE 128U
#define TARGET_TX_WINDOW_SIZE 128U
#define I2C_SLAVE_RX_BUFFER_LEN 512

enum
{
    I3CS_STATE_IDLE,
    I3CS_STATE_DATA_WRITE,
    I3CS_STATE_DATA_READ,
    I3CS_STATE_STOP,
};

uint8_t lower_memory[TARGET_MEMORY_BANK_SIZE] = {0};
uint8_t high_memory[TARGET_MEMORY_BANK_SIZE] = {0};
__IO uint8_t ubOwnDynAddr = 0;

typedef struct
{
    uint8_t _state;
    uint8_t _lastState;
    uint8_t _regAddr;
    uint8_t _recvBuf[I2C_SLAVE_RX_BUFFER_LEN];
    __IO uint16_t _recvBufLen;
    __IO uint16_t _sendBufIdx;
    uint8_t *_pSendBuf;
    uint8_t _currentByteAddr;
} I3CS_Ctrl_t;

static I3CS_Ctrl_t I3CS_Ctrl, *const self = &I3CS_Ctrl;

static inline uint8_t *SelectMemoryBank(uint8_t reg_addr)
{
    return (reg_addr < TARGET_MEMORY_BANK_SIZE) ? lower_memory : high_memory;
}

static inline uint8_t NextByteAddr(uint8_t addr)
{
    if (addr < TARGET_MEMORY_BANK_SIZE)
    {
        return (uint8_t)((addr + 1U) & 0x7FU);
    }

    return (uint8_t)(0x80U | ((addr + 1U) & 0x7FU));
}

static inline uint8_t AdvanceByteAddr(uint8_t addr, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++)
    {
        addr = NextByteAddr(addr);
    }
    return addr;
}

static void OverwriteMemory(uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint8_t addr = reg_addr;
    uint8_t *pMem = SelectMemoryBank(addr);

    for (i = 0U; i < len; ++i)
    {
        pMem[addr & 0x7f] = data[i];
        addr = NextByteAddr(addr);
    }
}

static inline void I3CS_FillTxFifo(void)
{
    while (LL_I3C_IsActiveFlag_TXFNF(I3C1) && self->_sendBufIdx < TARGET_TX_WINDOW_SIZE)
    {
        uint8_t index = (self->_regAddr & 0x7FU) + self->_sendBufIdx;
        uint8_t regVal = self->_pSendBuf[index & 0x7FU];

        LL_I3C_TransmitData8(I3C1, regVal);
        self->_sendBufIdx++;
    }
}

static inline void I3CS_PreloadFromRegAddr(void)
{
    self->_pSendBuf = SelectMemoryBank(self->_regAddr);
    self->_sendBufIdx = 0;
    LL_I3C_RequestTxFIFOFlush(I3C1);
    while (!LL_I3C_IsActiveFlag_TXFE(I3C1))
        ;
    LL_I3C_ConfigTxPreload(I3C1, TARGET_TX_WINDOW_SIZE);

    I3CS_FillTxFifo();
}

void I3CS_Init()
{
    memset(self, 0, sizeof(I3CS_Ctrl_t));
    ubOwnDynAddr = 0U;
    self->_state = I3CS_STATE_IDLE;
    self->_pSendBuf = lower_memory;
    I3CS_PreloadFromRegAddr();
}

/*
Register Access Methods in CMIS 5.4 Management Register Access Layer (RAL):
    VALUE = READ( [ByteAddress,] N), 1 ≤ N ≤ Nmax
    SUCCESS = WRITE( ByteAddress, VALUE, [VALUE, …])
    ACCESSIBLE = TEST( )

PC -> HOST command format:
    *twi <len>                          // read N bytes from current-address internal maintained
    *twi <len> <addr>                   // read N bytes from start address
    *twi <len> <addr> <data...>         // write N bytes to start address
*/
void I3C1_EV_IRQHandler(void)
{
    if (LL_I3C_IsActiveFlag_RXFNE(I3C1))
    {
        while (LL_I3C_IsActiveFlag_RXFNE(I3C1))
        {
            assert(self->_recvBufLen < sizeof(self->_recvBuf));

            uint8_t data = LL_I3C_ReceiveData8(I3C1);
            if (self->_recvBufLen == 0U)
            {
                /*
                 * The first byte of a private write is treated as the register
                 * address. Prepare the TX FIFO immediately so a following
                 * repeated-start read can be served without waiting for FCF.
                 */
                self->_regAddr = data;
                self->_currentByteAddr = self->_regAddr;
                I3CS_PreloadFromRegAddr();
            }
            self->_recvBuf[self->_recvBufLen++] = data;
        }
    }

    if (LL_I3C_IsActiveFlag_FC(I3C1))
    {
        uint32_t XDCNT = LL_I3C_GetXferDataCount(I3C1);
        uint32_t DIR = LL_I3C_GetMessageDirection(I3C1);

        LL_I3C_ClearFlag_FC(I3C1);

        if (DIR == LL_I3C_MESSAGE_DIRECTION_WRITE)
        {
            if (XDCNT == 1U) /* must be a repeat-start before reading or a STOP of Register-address-only write */
            {
                /*
                 * Register-address-only write.
                 * Do not flush/preload here: for a repeated-start read, the
                 * read phase may already be starting. RXFNE already prepared
                 * the TX FIFO from this register address.
                 */
                self->_recvBufLen = 0;
                LL_I3C_RequestRxFIFOFlush(I3C1);

                self->_currentByteAddr = self->_regAddr;
            }
            else if (XDCNT > 1U) /* must be a STOP for writing */
            {
                self->_regAddr = self->_recvBuf[0];

                /* update current byte addr */
                self->_currentByteAddr = AdvanceByteAddr(self->_regAddr, (uint16_t)XDCNT - 1U);

                /* overwrite memory with received data */
                OverwriteMemory(self->_regAddr, &self->_recvBuf[1], self->_recvBufLen - 1U);
                self->_recvBufLen = 0;
                LL_I3C_RequestRxFIFOFlush(I3C1);

                /* prepare preload TXFIFO from currentByteAddr */
                self->_regAddr = self->_currentByteAddr;
                I3CS_PreloadFromRegAddr();
            }
            else
            {
                assert(false); /* this is a protocol violation, should never happen */
            }
        }
        else /* must be a STOP of reading message */
        {
            self->_recvBufLen = 0;
            LL_I3C_RequestRxFIFOFlush(I3C1);

            /* update current byte addr */
            self->_currentByteAddr = AdvanceByteAddr(self->_regAddr, (uint16_t)XDCNT);

            /* prepare preload TXFIFO from currentByteAddr for current-address-read */
            self->_regAddr = self->_currentByteAddr;
            I3CS_PreloadFromRegAddr();
        }
    }

    if (LL_I3C_IsActiveFlag_TXFNF(I3C1))
    {
        I3CS_FillTxFifo();
    }

    if (LL_I3C_IsActiveFlag_DAUPD(I3C1))
    {
        LL_I3C_ClearFlag_DAUPD(I3C1);
        ubOwnDynAddr = LL_I3C_GetOwnDynamicAddress(I3C1);
    }
}

void I3C1_ER_IRQHandler(void)
{
    // assert(false);
    Error_Handler();
    if (LL_I3C_IsActiveFlag_ERR(I3C1))
    {
        /* If TX/RX FIFO overrun is reported */
        if (LL_I3C_IsActiveFlag_DOVR(I3C1))
        {
        }
        LL_I3C_ClearFlag_ERR(I3C1);
    }
}
