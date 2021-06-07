/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_midi.h"

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
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
USBD_MIDI_MessageTypeDef exti_msg[3];
extern USBD_HandleTypeDef hUsbDeviceFS;

int module_mode = 1; //0 = master
                     //1 = slave

#define STM32_UUID ((uint32_t *)0x1FFF7590)       // for STM32l412
uint8_t I2C_address = 1;

int slave_number = 0;
int slave_address[16];

int n;
int encoder0PinALast = 0;
uint8_t encoder0Pos = 0;
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
  HAL_Init();

  /* USER CODE BEGIN Init */
  uint32_t idPart1 = (STM32_UUID[0]&0x0008F000)>>12;
    uint32_t idPart2 = (STM32_UUID[1]&0x00FFF000)>>12;
    uint32_t idPart3 = (STM32_UUID[2]&0x00000F00)>>8;

  	uint8_t data[3];

    data[0] = 0;     // 0x0C in your example
    data[1] = 0;    // MSB byte of 16bit data
  	data[2] = 0;

  	int count = 0;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(4000);

//--------------------------------------
//------- MASTER MODE ------------------
//--------------------------------------
	if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED){
		module_mode = 1;
		int i = 0;
	  while(i<2)
		{
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
			HAL_Delay(300);
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
			HAL_Delay(300);
			i++;
		}
		HAL_Delay(500);
		i = 0;
	  while(i<127)
		{
		   if(HAL_I2C_Master_Receive(&hi2c1, i, data, 3,20)==HAL_OK)
		   {
					HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
				  HAL_Delay(300);
				  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
				  HAL_Delay(300);

				  slave_address[slave_number] = i;
				  slave_number++;
			 }
			 i++;
		 }
	}

//--------------------------------------
//-------- SLAVE MODE ------------------
//--------------------------------------
	else
	{
		I2C_address = idPart1;
		MX_I2C1_Init();
		while(HAL_I2C_Slave_Transmit(&hi2c1, data, 3, 100)!=HAL_OK) {}
		int i = 0;
	  while(i<2)
		{
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
			HAL_Delay(300);
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
			HAL_Delay(300);
			i++;
		}

	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  /*		if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED){
	  		  exti_msg[0] = MIDI_MAKE_EVENT(0, MIDI_EVENT_NOTE_ON, 60, 100);
	  		  USBD_MIDI_SendEvent(&hUsbDeviceFS, exti_msg[0]);
	  			HAL_Delay(500);
	  			exti_msg[0] = MIDI_MAKE_EVENT(0, MIDI_EVENT_NOTE_OFF, 60, 0);
	  			USBD_MIDI_SendEvent(&hUsbDeviceFS, exti_msg[0]);
	  			HAL_Delay(1000);
	  		}*/
	  //--------------------------------------
	  //------- MASTER MODE ------------------
	  //--------------------------------------
	  		if(module_mode==0)
	  		{
	  			int i =0;
	  			while (i<slave_number)
	  			{
	  				if(HAL_I2C_Master_Receive(&hi2c1, slave_address[i], data, 3,2)==HAL_OK)
	  		    {
	  /*					HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			    HAL_Delay(300);
	  					HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			    HAL_Delay(300);*/
	  					if(data[0]==1)
	  					{
	  						exti_msg[0] = MIDI_MAKE_EVENT(0, MIDI_EVENT_NOTE_ON, slave_address[i], 100);
	  		        USBD_MIDI_SendEvent(&hUsbDeviceFS, exti_msg[0]);
	  					}
	  				}
	  				i++;
	  			}

	  			n = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	        if ((encoder0PinALast == 0) && (n == 1)) {
	          if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0) {
	            if(encoder0Pos>0)
	  					{
	  						encoder0Pos--;
	  						HAL_Delay(10);
	  					}
	          } else {
	            if(encoder0Pos<127)
	  					{
	  						encoder0Pos++;
	  						HAL_Delay(10);
	  					}
	  					exti_msg[0] = MIDI_MAKE_EVENT(0, MIDI_EVENT_CC, 15, encoder0Pos);
	  		      USBD_MIDI_SendEvent(&hUsbDeviceFS, exti_msg[0]);
	          }
	        }
	        encoder0PinALast = n;
	  			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			HAL_Delay(3000);
	  			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			HAL_Delay(300);
	  		}

	  //--------------------------------------
	  //-------- SLAVE MODE ------------------
	  //--------------------------------------
	  		if(module_mode==1)
	  		{
	  /*			if(count>1000) {

	  				count=0;
	  				data[0] = 1;
	  			}
	  			else {
	  				count++;
	  			}
	  			if(HAL_I2C_Slave_Transmit(&hi2c1, data, 3, 100)==HAL_OK) {
	  				if(data[0]==1) data[0] = 0;
	  			}
	  			HAL_Delay(1);*/
	  			HAL_I2C_Slave_Transmit(&hi2c1, data, 3, 2);
	  			n = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	        if ((encoder0PinALast == 0) && (n == 1)) {
	          if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0) {
	            if(encoder0Pos>0)
	  					{
	  						encoder0Pos--;
	  						HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			      HAL_Delay(300);
	  					  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			      HAL_Delay(300);
	  					}
	          } else {
	            if(encoder0Pos<127)
	  					{
	  						encoder0Pos++;
	  						HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			      HAL_Delay(300);
	  					  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  			      HAL_Delay(300);
	  						HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  				    HAL_Delay(300);
	  				    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
	  				    HAL_Delay(300);
	  					}
	          }
	        }
	        encoder0PinALast = n;

	  		}


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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_USB;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_MSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
