/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "i2c.h"
#include "mpu.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define BUFFER_SIZE 64
#define CMD_SIZE 32

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t receive[1];
uint8_t buffer[BUFFER_SIZE];
uint8_t led_state;
uint8_t streaming;
volatile uint8_t head;
volatile uint8_t tail;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
void ring_write(uint8_t byte);
uint8_t ring_read(void);
uint8_t ring_empty(void);
void process_command(uint8_t* cmd);
void print_mpu_data();

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
  uint8_t cmd_buff[CMD_SIZE];
  uint8_t cmd_index = 0;

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  i2c1_init();
  mpu_init();

  /* USER CODE BEGIN 2 */
  head = 0;
  tail = 0;
  streaming = 0;

  char greeting[16];
  snprintf(greeting, sizeof(greeting), "> Welcome\r\n");
  HAL_UART_Transmit(&huart2, greeting, strlen(greeting), HAL_MAX_DELAY);

  HAL_UART_Receive_IT(&huart2, receive, 1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if(!ring_empty())
	{
		uint8_t byte = ring_read();

		// New line
		if(byte == '\r' || byte == '\n')
		{
			cmd_buff[cmd_index] = '\0';
			process_command(cmd_buff);
			cmd_index = 0;
			memset(cmd_buff, 0, CMD_SIZE);
		}
		// Backspace
		else if(byte == '\b' && cmd_index != 0)
		{
			cmd_index--;
			cmd_buff[cmd_index] = 0;
			uint8_t bs_seq[] = "\b \b";
			HAL_UART_Transmit(&huart2, bs_seq, 3, HAL_MAX_DELAY);
		}
		// Normal character, buffer isn't full
		else if(cmd_index < CMD_SIZE - 1)
		{
			cmd_buff[cmd_index] = byte;
			cmd_index++;
			HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
		}
	}
	// BLOCKING
	// need timers to create non-blocking
	if(streaming == 1)
	{
		print_mpu_data();
		HAL_Delay(1000);
	}
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void process_command(uint8_t* cmd)
{
	uint8_t newline[] = "\r\n";
	HAL_UART_Transmit(&huart2, newline, 2, HAL_MAX_DELAY);

	// turns led on
	if(strcmp((char*)cmd, "led on") == 0)
	{
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		led_state = 1;
		uint8_t msg[] = "> LED ON\r\n";
		HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
	}
	// turns led off
	else if(strcmp((char*)cmd, "led off") == 0)
	{
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		led_state = 0;
		uint8_t msg[] = "> LED OFF\r\n";
		HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
	}
	// fetches led status
	else if(strcmp((char*)cmd, "status") == 0)
	{
		if(led_state)
		{
			uint8_t msg[] = "> LED is ON\r\n";
			HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
		}
		else
		{
			uint8_t msg[] = "> LED is OFF\r\n";
			HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);
		}
	}
	// returns MPU's identity
	else if(strcmp((char*)cmd, "whoami") == 0)
	{
		uint8_t result = 0;
		char msg[32];
		uint8_t err = i2c_read(0x68, 0x75, &result);
		if(err != I2C_OK)
		{
			snprintf(msg, sizeof(msg), "> Error\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
		}
		else
		{
			snprintf(msg, sizeof(msg), "> WHO_AM_I: 0x%02X\r\n", result);
			HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
		}
	}
	// reads accel, temp, and gyro data from MPU
	else if(strcmp((char*)cmd, "read") == 0)
	{
		print_mpu_data();
	}
	// constantly streams MPU sensor data
	else if(strcmp((char*)cmd, "stream") == 0)
	{
		streaming = 1;
		char msg[64];
		snprintf(msg, sizeof(msg), "> Streaming MPU Data\r\n");
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
	// stops stream of MPU sensor data
	else if(strcmp((char*)cmd, "stop") == 0)
	{
		streaming = 0;
		char msg[64];
		snprintf(msg, sizeof(msg), "> Stopping Data Stream\r\n");
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
	// unknown command entered
	else
	{
		uint8_t msg[] = "> unknown cmd\r\n";
		HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, HAL_MAX_DELAY);

	}
}
void ring_write(uint8_t byte)
{
	// checks if buffer is not full
	if (head != (tail - 1 + BUFFER_SIZE) % BUFFER_SIZE)
	{
		// writes to head of ring buffer
		buffer[head] = byte;
		head = (head + 1) % BUFFER_SIZE; // wrap-around
	}
}

uint8_t ring_read(void)
{
	// checks if empty buffer
	if (head == tail)
	{
		return 0;
	}
	// reads current byte (tail) and advances
	else
	{
		uint8_t byte = buffer[tail];
		tail = (tail + 1) % BUFFER_SIZE; // wrap-around
		return byte;
	}
}

uint8_t ring_empty(void)
{
	// returns true if ring is empty
	if (head == tail)
		return 1;
	else
		return 0;
}

// callback function
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// checks for USART 2
	if(huart->Instance == USART2)
	{
		ring_write(receive[0]);
		HAL_UART_Receive_IT(huart, receive, 1);
	}
}

// print sensor data
void print_mpu_data()
{
	int16_t data[7];
	char msg[64];
	uint8_t err = mpu_read(data);
	if(err != I2C_OK)
	{
		snprintf(msg, sizeof(msg), "> Error\r\n");
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
	else
	{
		MPU_Data mpu_data;
		mpu_convert(data, &mpu_data);

		snprintf(msg, sizeof(msg), "> Accel X: %.2f\r\n", mpu_data.accel_x);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Accel Y: %.2f\r\n", mpu_data.accel_y);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Accel Z: %.2f\r\n", mpu_data.accel_z);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Temp (C): %.2f\r\n", mpu_data.temp_c);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Gyro X: %.2f\r\n", mpu_data.gyro_x);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Gyro Y: %.2f\r\n", mpu_data.gyro_y);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

		snprintf(msg, sizeof(msg), "> Gyro Z: %.2f\r\n", mpu_data.gyro_z);
		HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
