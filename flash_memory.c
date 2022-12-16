/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

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
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t operation[1]; // choose the operation you want to do: write or read or delete
uint8_t deleteInput[1]; // choose the operation you want to do: write or read or delete

char bootMessage[] = "\nReady\n";
uint8_t writeMessage[] = "\nenter task:";
char writeconfirmMessage[] = "task added";
char deleteconfirmMessage[] = "task deleted";
char deleteMessage[] = "\nenter num of task to delete";
char readMessage[] = "\nYour to do list:\n";
char dayMessage[] = "\nnew day has been started\n";
char maxtaskMessage[] =
		"\ncan't add task.you reached your max number of tasks per day. upgrade to premium to enjoy more tasks per day starting from 50rm per month\n";
char toturialMessage[] =
		"\nw to write. r to read. d to delete. n for new day. max 6 tasks per day. 10 char per task\n";

uint8_t bytes_temp[4];

uint32_t addressInt = 0x08004C10;
uint32_t address = 0x08005C10;
uint32_t address1 = 0x08006C30;
uint32_t address2 = 0x08007C30;
uint32_t address3 = 0x08008C30;
uint32_t address4 = 0x08009C30;
uint32_t address5 = 0x0800AC30;

struct Node {
	char *data;
	struct Node *next;
};

char tx_data[10] = "FFFFFFFFFF";
char header[20] = "\nend of tasks";
char task0[10] = "0";
char task1[20] = "0";
char task2[20] = "0";
char task3[20] = "0";
char task4[20] = "0";
char task5[20] = "0";

char tmpString[10] = "FFFFFFFFFF";
char tmpString2[10] = "FFFFFFFFFF";
char Rx_Data[10] = "FFFFFFFFFF";

int numberOfTasks = 0;

struct Node *head = NULL;
/*
 struct Node *first = NULL;
 struct Node *second = NULL;
 struct Node *third = NULL;
 struct Node *fourth = NULL;
 */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* FLASH_PAGE_SIZE should be able to get the size of the Page according to the controller */
static uint32_t GetPage(uint32_t Address) {
	for (int indx = 0; indx < 128; indx++) {
		if ((Address < (0x08000000 + (FLASH_PAGE_SIZE * (indx + 1))))
				&& (Address >= (0x08000000 + FLASH_PAGE_SIZE * indx))) {
			return (0x08000000 + FLASH_PAGE_SIZE * indx);
		}
	}

	return 0;
}

uint32_t Flash_Write_Data(uint32_t StartPageAddress, uint32_t *Data,
		uint16_t Datalengh) {

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	int sofar = 0;

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Erase the user Flash area*/

	uint32_t StartPage = GetPage(StartPageAddress);
	uint32_t EndPageAdress = StartPageAddress + Datalengh * 4;
	uint32_t EndPage = GetPage(EndPageAdress);

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = StartPage;
	EraseInitStruct.NbPages = ((EndPage - StartPage) / FLASH_PAGE_SIZE) + 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
		/*Error occurred while page erase.*/
		return HAL_FLASH_GetError();
	}

	/* Program the user Flash area word by word*/

	while (sofar < Datalengh) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartPageAddress,
				Data[sofar]) == HAL_OK) {
			StartPageAddress += 4; // use StartPageAddress += 2 for half word and 8 for double word
			sofar++;
		} else {
			/* Error occurred while writing data in Flash memory*/
			return HAL_FLASH_GetError();
		}
	}

	HAL_FLASH_Lock();

	return 0;
}

void Flash_Read_Data(uint32_t StartPageAddress, uint32_t *RxBuf,
		uint16_t Datalengh) {
	while (1) {

		*RxBuf = *(__IO uint32_t*) StartPageAddress;
		StartPageAddress += 4;
		RxBuf++;
		if (!(Datalengh--))
			break;
	}
}

void Flash_Erase_Data(uint32_t StartPageAddress, uint16_t Datalengh) {

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Erase the user Flash area*/

	uint32_t StartPage = GetPage(StartPageAddress);
	uint32_t EndPageAdress = StartPageAddress + Datalengh * 4;
	uint32_t EndPage = GetPage(EndPageAdress);

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = StartPage;
	EraseInitStruct.NbPages = ((EndPage - StartPage) / FLASH_PAGE_SIZE) + 1;

	HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

	HAL_FLASH_Lock();
}

void Convert_To_Str(uint32_t *Data, char *Buf, uint16_t Datalengh) {
	int numberofbytes = Datalengh * 4;

	for (int i = 0; i < numberofbytes; i++) {
		Buf[i] = Data[i / 4] >> (8 * (i % 4));
	}
}
void float2Bytes(uint8_t *ftoa_bytes_temp, float float_variable) {
	union {
		float a;
		uint8_t bytes[4];
	} thing;

	thing.a = float_variable;

	for (uint8_t i = 0; i < 4; i++) {
		ftoa_bytes_temp[i] = thing.bytes[i];
	}

}

float Bytes2float(uint8_t *ftoa_bytes_temp) {
	union {
		float a;
		uint8_t bytes[4];
	} thing;

	for (uint8_t i = 0; i < 4; i++) {
		thing.bytes[i] = ftoa_bytes_temp[i];
	}

	float float_variable = thing.a;
	return float_variable;
}

void Flash_Write_NUM(uint32_t StartSectorAddress, float Num) {

	float2Bytes(bytes_temp, Num);

	Flash_Write_Data(StartSectorAddress, (uint32_t*) bytes_temp, 1);
}

float Flash_Read_NUM(uint32_t StartSectorAddress) {
	uint8_t buffer[4];
	float value;

	Flash_Read_Data(StartSectorAddress, (uint32_t*) buffer, 1);
	value = Bytes2float(buffer);
	return value;
}

/*void saveToDolist(struct Node *n) {
 int i = 0;
 while (n != NULL && i < 5) {
 Flash_Write_Data(address, (uint32_t*) n->data, strlen(n->data));
 i++;
 n = n->next;
 }
 }*/

/*void appendToDoList(struct Node **head_ref, char *new_data) {
 //	 1. allocate node
 struct Node *new_node = (struct Node*) malloc(sizeof(struct Node));

 //	 2. put in the data
 new_node->data = new_data;

 //	 3. Make next of new node as head
 new_node->next = (*head_ref);

 //	 4. move the head to point to the new node
 (*head_ref) = new_node;
 }*/

/*void deleteNode(struct Node **head) {
 //temp is used to freeing the memory
 struct Node *temp;

 //key found on the head node.
 //move to head node to the next and free the head.
 temp = *head;    //backup the head to free its memory
 *head = (*head)->next;
 free(temp);
 }*/

/*
 void displayToDoList(struct Node *n) {

 //	int i=0;
 //	&& i < 5
 while (n != NULL) {
 //		Flash_Read_Data(address[i], (uint32_t*) Rx_Data, strlen(Rx_Data));
 //		Convert_To_Str((uint32_t*) Rx_Data, tmpString, strlen(Rx_Data));
 HAL_UART_Transmit(&huart1, (uint8_t*) n->data, strlen(n->data), 10);
 //		i++;
 n = n->next;
 }
 }
 */

void readToDoList() {
	numberOfTasks = Flash_Read_NUM(addressInt);

	HAL_UART_Transmit(&huart1, (uint8_t*) readMessage, sizeof(readMessage),
			100);

	if (numberOfTasks > 0) {
		Flash_Read_Data(address, (uint32_t*) Rx_Data, 10);
		if (*Rx_Data != (uint8_t) 0xFFFFFFFF) {
			Convert_To_Str((uint32_t*) Rx_Data, tmpString, 10);
			HAL_UART_Transmit(&huart1, (uint8_t*) tmpString, 10, 10);
		}
	}
	if (numberOfTasks > 1) {
		Flash_Read_Data(address1, (uint32_t*) Rx_Data, 10);
		if (*Rx_Data != (uint8_t) 0xFFFFFFFF) {
			Convert_To_Str((uint32_t*) Rx_Data, tmpString, 10);
			HAL_UART_Transmit(&huart1, (uint8_t*) tmpString, 10, 10);
		}
	}
	if (numberOfTasks > 2) {
		Flash_Read_Data(address2, (uint32_t*) Rx_Data, 10);
		if (*Rx_Data != (uint8_t) 0xFFFFFFFF) {
			Convert_To_Str((uint32_t*) Rx_Data, tmpString, 10);
			HAL_UART_Transmit(&huart1, (uint8_t*) tmpString, 10, 10);
		}
	}
	if (numberOfTasks > 3) {
		Flash_Read_Data(address3, (uint32_t*) Rx_Data, 10);
		if (*Rx_Data != (uint8_t) 0xFFFFFFFF) {
			Convert_To_Str((uint32_t*) Rx_Data, tmpString, 10);
			HAL_UART_Transmit(&huart1, (uint8_t*) tmpString, 10, 10);
		}
	}
	if (numberOfTasks > 4) {
		Flash_Read_Data(address4, (uint32_t*) Rx_Data, 10);
		if (*Rx_Data != (uint8_t) 0xFFFFFFFF) {
			Convert_To_Str((uint32_t*) Rx_Data, tmpString, 10);
			HAL_UART_Transmit(&huart1, (uint8_t*) tmpString, 10, 10);
		}
	}
	if (numberOfTasks > 5) {
		Flash_Read_Data(address5, (uint32_t*) Rx_Data, 10);
		if (*Rx_Data != (uint8_t) 0xFFFFFFFF) {
			Convert_To_Str((uint32_t*) Rx_Data, tmpString, 10);
			HAL_UART_Transmit(&huart1, (uint8_t*) tmpString, 10, 10);
		}
	}

}

void deleteToDo(int number_of_task) {
	switch (number_of_task - 1) {
	case 0:
		Flash_Erase_Data(address, 10);
		break;
	case 1:
		Flash_Erase_Data(address1, 10);
		break;
	case 2:
		Flash_Erase_Data(address2, 10);
		break;
	case 3:
		Flash_Erase_Data(address3, 10);
		break;
	case 4:
		Flash_Erase_Data(address4, 10);
		break;
	case 5:
		Flash_Erase_Data(address5, 10);

	}
}

void startNewDay() {
	Flash_Erase_Data(addressInt, 10);
	Flash_Erase_Data(address, 10);
	Flash_Erase_Data(address1, 10);
	Flash_Erase_Data(address2, 10);
	Flash_Erase_Data(address3, 10);
	Flash_Erase_Data(address4, 10);
	Flash_Erase_Data(address5, 10);
	numberOfTasks = 0;
	HAL_UART_Transmit(&huart1, (uint8_t*) dayMessage, strlen(dayMessage), 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart1) { // callback for when we receive an input
	int userInput = (int) operation[0];

	if (userInput == 119) { // write operations
		if (numberOfTasks < 5) {
			HAL_UART_Transmit(huart1, (uint8_t*) writeMessage,sizeof(writeMessage), 10);
//		appendToDoList(&head, tx_data);
//		HAL_UART_Receive(huart1, (uint8_t*) task1, 10, 10000);
//		Flash_Write_Data(numberOfTasks*1023 + address, (uint32_t*) task1, 10);
			if (numberOfTasks == 0) {
				HAL_UART_Receive(huart1, (uint8_t*) task1, 10, 10000);
				Flash_Write_Data(address, (uint32_t*) task1, 10);
			} else if (numberOfTasks == 1) {
				HAL_UART_Receive(huart1, (uint8_t*) task2, 10, 10000);
				Flash_Write_Data(address1, (uint32_t*) task2, 10);
			} else if (numberOfTasks == 2) {
				HAL_UART_Receive(huart1, (uint8_t*) task3, 10, 10000);
				Flash_Write_Data(address2, (uint32_t*) task3, 10);
			} else if (numberOfTasks == 3) {
				HAL_UART_Receive(huart1, (uint8_t*) task4, 10, 10000);
				Flash_Write_Data(address3, (uint32_t*) task4, 10);
			} else if (numberOfTasks == 4) {
				HAL_UART_Receive(huart1, (uint8_t*) task4, 10, 10000);
				Flash_Write_Data(address4, (uint32_t*) task4, 10);
			} else if (numberOfTasks == 5) {
				HAL_UART_Receive(huart1, (uint8_t*) task5, 10, 10000);
				Flash_Write_Data(address5, (uint32_t*) task5, 10);
			}
			numberOfTasks++;
			Flash_Write_NUM(addressInt, numberOfTasks);
			HAL_UART_Transmit(huart1, (uint8_t*) writeconfirmMessage,strlen(writeconfirmMessage), 10);

		} else {
			HAL_UART_Transmit(huart1, (uint8_t*) maxtaskMessage,strlen(maxtaskMessage), 10);
		}
//		saveToDolist(head);

//		Flash_Write_Data(address1, (uint32_t*) task0, strlen(task0));
//		Flash_Read_Data(address1, (uint32_t*) Rx_Data, strlen(Rx_Data));
//		Convert_To_Str((uint32_t*) Rx_Data, tmpString, strlen(Rx_Data));
	} else if (userInput == 114) {
//		displayToDoList(head);
		readToDoList();
//		HAL_UART_Transmit(huart1, (uint8_t*) task1, sizeof(task1),10);

	} else if (userInput == 100) {
		HAL_UART_Transmit(huart1, (uint8_t*) deleteMessage,
				strlen(deleteMessage), 10);
		HAL_UART_Receive(huart1, deleteInput, 1, 10000);
		int taskTODelete = deleteInput[0] - 48;
//		char data[50];
//		sprintf(data, " thus it %d, %d, %c\n", taskTODelete, (int)deleteInput[0], (char)deleteInput[0]);
//		HAL_UART_Transmit(huart1, (uint8_t*) &data,
//				strlen(data), 10);
		deleteToDo(taskTODelete);
		HAL_UART_Transmit(huart1, (uint8_t*) deleteconfirmMessage,
				strlen(deleteconfirmMessage), 10);

	} else if (userInput == 110) {
		startNewDay();
	}
	HAL_UART_Receive_IT(huart1, (uint8_t*) operation, 1);
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
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	head = (struct Node*) malloc(sizeof(struct Node));
	/*first = (struct Node*) malloc(sizeof(struct Node));
	 second = (struct Node*) malloc(sizeof(struct Node));
	 third = (struct Node*) malloc(sizeof(struct Node));
	 fourth = (struct Node*) malloc(sizeof(struct Node));*/

	head->data = header; // assign data in first node
	head->next = NULL;
	/*

	 first->data = task1; // assign data to second node
	 first->next = second;

	 second->data = task2; // assign data to third node
	 second->next = third;

	 third->data = task3; // assign data to fourth node
	 third->next = fourth;

	 fourth->data = task4; // assign data to fourth node
	 fourth->next = NULL;
	 */
	int timer1 = HAL_GetTick();
//	Flash_Read_Data(address1, led1_fl);
//	Flash_Read_Data(address2, led2_fl);
	HAL_UART_Transmit(&huart1, (uint8_t*) bootMessage, strlen(bootMessage),
			100);
	HAL_UART_Transmit(&huart1, (uint8_t*) toturialMessage,
			strlen(toturialMessage), 100);

	HAL_UART_Receive_IT(&huart1, (uint8_t*) operation, 1);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (HAL_GetTick() - timer1 >= 250) {
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			timer1 = HAL_GetTick();
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

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

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
