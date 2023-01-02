/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
//static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void initTim2(void) {
	//ENABLE TIM2 CLOCK
	RCC->APB1ENR |= (1u << 0); // timer2
	RCC->APB1ENR |= (1u << 1); // timer3

	//LEAVE THE COUNTER FREQUENCY UNCHANGED
	TIM2->PSC = 16800;
	TIM3->PSC = 16800;

	//SET TIMER RELOAD VALUE
	TIM2->ARR = (uint32_t) 10000;
	TIM3->ARR = (uint32_t) 10000;

	//SET INITIAL COUNTER VALUE
	TIM2->CNT = 0;
	TIM3->CNT = 0;

	//ENABLE TIM2 COUNTER
	TIM2->CR1 |= (1u << 0);
	TIM3->CR1 |= (1u << 0);
}

void initGPIO(void) {
	RCC->AHB1ENR |= (1 << 0);  // Enable the GPIOA clock

	GPIOA->MODER &= ~((1 << 2) | (1 << 3));  // pin PA1(bits 3:2) as input (00)
	GPIOA->MODER &= ~((1 << 4) | (1 << 5));  // pin PA2(bits 5:4) as input (00)

	GPIOA->PUPDR |= (1 << 2);  // Pin PA1 (bits 3:2) are 0:1 --> PULL-UP
	GPIOA->PUPDR |= (1 << 4);  // Pin PA2 (bits 5:4) are 0:1 --> PULL-UP

	GPIOA->MODER |= (1 << 10);  // pin PA5(bits 11:10) as Output (01)
	GPIOA->MODER |= (1 << 12);  // pin PA6(bits 13:12) as Output (01)
	GPIOA->MODER |= (1 << 14);  // pin PA7(bits 15:14) as Output (01)

	GPIOA->OTYPER &= ~(1 << 5);  // bit 5=0 --> Output push pull
	GPIOA->OTYPER &= ~(1 << 6);  // bit 6=0 --> Output push pull
	GPIOA->OTYPER &= ~(1 << 7);  // bit 7=0 --> Output push pull

	GPIOA->OSPEEDR |= (1 << 11);  // Pin PA5 (bits 11:10) as Fast Speed (1:0)
	GPIOA->OSPEEDR |= (1 << 13);  // Pin PA6 (bits 13:12) as Fast Speed (1:0)
	GPIOA->OSPEEDR |= (1 << 15);  // Pin PA7 (bits 15:14) as Fast Speed (1:0)

	GPIOA->PUPDR &= ~((1 << 10) | (1 << 11)); // Pin PA5 (bits 11:10) are 0:0 --> no pull up or pulldown
	GPIOA->PUPDR &= ~((1 << 12) | (1 << 13)); // Pin PA6 (bits 13:12) are 0:0 --> no pull up or pulldown
	GPIOA->PUPDR &= ~((1 << 14) | (1 << 15)); // Pin PA6 (bits 15:14) are 0:0 --> no pull up or pulldown

}

void initClock(void){

	//ENABLE HSE and wait for the HSE to become Ready
	RCC->CR |= 1<<16;
	while (!(RCC->CR & (1<<17)));

	//Set the POWER ENABLE CLOCK
	RCC->APB1ENR |= 1<<28;

	//Set the VOLTAGE REGULATOR
	PWR->CR |= 3<<14;

	//Configure the FLASH PREFETCH and the LATENCY Related Settings
	FLASH->ACR = (1<<8) | (1<<9)| (1<<10)| (5<<0);

	// AHB PR
	RCC->CFGR |= RCC_SYSCLK_DIV1;

	// APB1 PR
	RCC->CFGR |= RCC_HCLK_DIV4;

	// APB2 PR
	RCC->CFGR |= RCC_HCLK_DIV2;

	//Configure the MAIN PLL
	RCC->PLLCFGR = (25 <<0) | (336 << 6) | (2 <<16) | (1<<22);

	//Enable the PLL and wait for it to become ready
	RCC->CR |= (1<<24);
	while (!(RCC->CR & (1<<25)));

	//Select the Clock Source and wait for it to be set
	RCC->CFGR |= (2<<0);
	while (!(RCC->CFGR & (2<<2)));

}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//	HAL_Init();

	/* USER CODE BEGIN Init */
	initTim2();
	initGPIO();
	initClock();
	/* USER CODE END Init */

	/* Configure the system clock */
//	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
//	MX_GPIO_Init();
	/* USER CODE BEGIN 2 */
	int timer2 = TIM2->CNT;
	int timer3 = TIM3->CNT;
	int temp = 250;
	int bt1Debounce = 0;
	int bt2Debounce = 0;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (TIM2->CNT - timer2 >= 500) {
			GPIOA->ODR ^= GPIO_PIN_5;
			timer2 = TIM2->CNT;
		}

		if ((GPIOA->IDR & (1 << 1)) && bt1Debounce == 0) {
			bt1Debounce = 1;
		}
		if (!(GPIOA->IDR & (1 << 1)) && bt1Debounce == 1) {
			GPIOA->ODR ^= GPIO_PIN_6;
			bt1Debounce = 0;
		}
		if ((GPIOA->IDR & (1 << 2)) && bt2Debounce == 0) {
			bt2Debounce = 1;
		}
		if (!(GPIOA->IDR & (1 << 2)) && bt2Debounce == 1) {
			if (temp >= 1000)
				temp = 250;
			else
				temp += 250;
			bt2Debounce = 0;
		}
		if (TIM2->CNT - timer3 >= temp) {
			GPIOA->ODR ^= GPIO_PIN_7;
			timer3 = TIM2->CNT;
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
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
/*
static void MX_GPIO_Init(void) {

	 GPIO Ports Clock Enable
	__HAL_RCC_GPIOH_CLK_ENABLE();

}
*/

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
