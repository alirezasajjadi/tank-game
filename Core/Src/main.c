/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "LiquidCrystal.h"
#include "string.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
enum Direction {
	RIGHT, DOWN, LEFT, UP
};

typedef struct {
	int first;
	int second;
} Pair;

typedef struct {
	int health;
	int numBullet;
	int soundEfect;
} DefaultSetting;

typedef unsigned char byte;

typedef struct {
	int col;
	int row;
} CursorLocation;

typedef struct {
	int flag;
	int pageExecuted;
} MenuInfo;

typedef struct {
	char *name;
	int health;
	int numBullet;
	enum Direction direction;
	int col;
	int row;
	int isAlive; //binary
	int score;
} Player;

typedef struct {
	int col;
	int row;
	enum Direction direction;
	int isAlive;
} Bullet;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WALL_1_COL 1
#define WALL_1_ROW 1
#define WALL_2_COL 18
#define WALL_2_ROW 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
//const byte pointerChar[] = { 0x00, 0x00, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00 };
const byte tankRightChar[] = { 0x18, 0x18, 0x18, 0x1F, 0x1F, 0x18, 0x18, 0x18 };
const byte tankLeftChar[] = { 0x03, 0x03, 0x03, 0x1F, 0x1F, 0x03, 0x03, 0x03 };
const byte tankDownChar[] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E };
const byte tankUpChar[] = { 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F };

const byte wallChar[] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F };
// amirali
const byte healthChar[] = { 0x00, 0x1B, 0x1B, 0x1F, 0x0E, 0x04, 0x00, 0x00 };
const byte barrierChar[] = { 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F };
const byte bulletChar[] = { 0x08, 0x1C, 0x08, 0x01, 0x03, 0x06, 0x0C, 0x18 };
const byte questionmarkChar[] =
		{ 0x1F, 0x11, 0x15, 0x1D, 0x1B, 0x1B, 0x1F, 0x1B };

// amirali

DefaultSetting dfSetting = { 3, 5, 0 };

MenuInfo menuOption[] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };

Player player[2];

char teamMembers[2][10] = { "Alireza", "Ali" };

int initPageFlag = 0, menuFlag = 0, gameStartFlag = 0, settingFlag = 0,
		aboutUsFlag = 0, clearLcdFlag = 0;

int initPageExecuted = 0, menuPageExecuted = 0, settingPageExecuted = 0,
		aboutPageExecuted = 0, gameStartPageExecuted = 0;

int dfsettingFlags = 0; /* range [1,2,3] 1: dfHealth 2:dfNumBullet 3:dfSoundEffect*/

int menuPointerLocation = 0, settingPointerLocation = 0;

RTC_TimeTypeDef mytime;
RTC_DateTypeDef mydate;

int num_shooted_bullet = 0;
Bullet bullets[100];

int array[20][4] = { 0 };
int prevarray[20][4] = { 0 };
int shootarray[20][4] = { 0 };
int shootflag = 0;
//0:empty  1:tankRight 2:downTank 3:lefrTank 4: RightRank 5:bullet  6:health  7:barrier  8:wall
//9:questionMark 10:shootedBullet

// Input pull down rising edge trigger interrupt pins:
// Row1 PD1, Row2 PD2, Row3 PD3, Row4 PD4
GPIO_TypeDef *const Row_ports[] = { GPIOD, GPIOD, GPIOD, GPIOD };
const uint16_t Row_pins[] = { GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4 };
// Output pins: Column1 PD5, Column2 PD6, Column3 PD7, Column4 PB4
GPIO_TypeDef *const Column_ports[] = { GPIOD, GPIOD, GPIOD, GPIOB };
const uint16_t Column_pins[] =
		{ GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_4 };
volatile uint32_t last_gpio_exti;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void begin_page();
void play_song();
void aboutUs_page();
void setting_page();
void menuDisplay();
void setRtcTime();
void customCharsCreate();
void setSettingValues(int val);
void changePointer(int *ptrLoc, int *pageExecuted, char *dir);

void initPlayers();
void initGamePage();
void rotateDirection90(Player *player);
int isDuplicate(Pair arr[], int index, int first, int second, int duringGame);
void generatePairs(Pair arr[], int n, int duringGame);
void generateQM();
void move(Player *player);
int checkMovePossible(int col, int row);
void checkItemExist(Player *player);
void delElement(int num, int *arr, int index);
void copyArray();
void refresh();
void emptyArray();

void shoot(Player *player);
void moveBullets();

int hasBullet(Player *player);

void increaseBullet(Player *player);
void decreaseBullet(Player *player);
void increaseHealth(Player *player);
void decreaseHealth(Player *player);

void finaltime();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rx_byte;
int idx = 0;
uint8_t inArray[9];
uint8_t inputHours[3];
uint8_t inputMin[3];
uint8_t inputSec[3];
int hour, minute, second;

uint8_t inputYear[3];
uint8_t inputMonth[3];
uint8_t inputDay[3];
int year, month, day;

int timeFlag = 0;

int uarttime = 1;
int uartdate = 0;

void setScore(int score) {
	char buffer[50];
	sprintf(buffer, "\nScore: %d", score);
	HAL_UART_Transmit(&huart3, (uint8_t*) buffer, strlen(buffer),
			HAL_MAX_DELAY);
}

void parser() {
	if (uarttime) {
		hour = 0;
		minute = 0;
		second = 0;

		for (int i = 0; i < 8; i++) {
			if (i == 1) {
				inputHours[0] = inArray[0];
				inputHours[1] = inArray[1];
				inputHours[2] = '\0';
				hour = atoi(inputHours);
			} else if (i == 3) {
				inputMin[0] = inArray[3];
				inputMin[1] = inArray[4];
				inputMin[2] = '\0';
				minute = atoi(inputMin);
			} else if (i == 6) {
				inputSec[0] = inArray[6];
				inputSec[1] = inArray[7];
				inputSec[2] = '\0';
				second = atoi(inputSec);
			}
		}
	}

	if (uartdate) {
		year = 0;
		month = 0;
		day = 0;

		for (int i = 0; i < 8; i++) {
			if (i == 1) {
				inputYear[0] = inArray[0];
				inputYear[1] = inArray[1];
				inputYear[2] = '\0';
				year = atoi(inputYear);
			} else if (i == 3) {
				inputMonth[0] = inArray[3];
				inputMonth[1] = inArray[4];
				inputMonth[2] = '\0';
				month = atoi(inputMonth);
			} else if (i == 6) {
				inputDay[0] = inArray[6];
				inputDay[1] = inArray[7];
				inputDay[2] = '\0';
				day = atoi(inputDay);
			}
		}
		uartdate = 0;
	}

	timeFlag = 1;

//	memset(inArray, '\0', sizeof(inArray));
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
		memset(inArray, '\0', sizeof(inArray));
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
		inArray[idx] = rx_byte;
		idx++;
		if (idx == 9) {
			idx = 0;
			parser();
			uarttime = 0;
			uartdate = 1;
		}
		HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (last_gpio_exti + 300 > HAL_GetTick()) // Simple button debouncing
			{
		return;
	}
	last_gpio_exti = HAL_GetTick();

	int8_t row_number = -1;
	int8_t column_number = -1;

	if (GPIO_Pin == GPIO_PIN_0) {
		// blue_button_pressed = 1;
		// return;
	}

	for (uint8_t row = 0; row < 4; row++) // Loop through Rows
			{
		if (GPIO_Pin == Row_pins[row]) {
			row_number = row;
		}
	}

	HAL_GPIO_WritePin(Column_ports[0], Column_pins[0], 0);
	HAL_GPIO_WritePin(Column_ports[1], Column_pins[1], 0);
	HAL_GPIO_WritePin(Column_ports[2], Column_pins[2], 0);
	HAL_GPIO_WritePin(Column_ports[3], Column_pins[3], 0);

	for (uint8_t col = 0; col < 4; col++) // Loop through Columns
			{
		HAL_GPIO_WritePin(Column_ports[col], Column_pins[col], 1);
		if (HAL_GPIO_ReadPin(Row_ports[row_number], Row_pins[row_number])) {
			column_number = col;
		}
		HAL_GPIO_WritePin(Column_ports[col], Column_pins[col], 0);
	}

	HAL_GPIO_WritePin(Column_ports[0], Column_pins[0], 1);
	HAL_GPIO_WritePin(Column_ports[1], Column_pins[1], 1);
	HAL_GPIO_WritePin(Column_ports[2], Column_pins[2], 1);
	HAL_GPIO_WritePin(Column_ports[3], Column_pins[3], 1);

	if (row_number == -1 || column_number == -1) {
		return; // Reject invalid scan
	}
	//   C0   C1   C2   C3
	// +----+----+----+----+
	// | 1  | 2  | 3  | 4  |  R0
	// +----+----+----+----+
	// | 5  | 6  | 7  | 8  |  R1
	// +----+----+----+----+
	// | 9  | 10 | 11 | 12 |  R2
	// +----+----+----+----+
	// | 13 | 14 | 15 | 16 |  R3
	// +----+----+----+----+
	const uint8_t button_number = row_number * 4 + column_number + 1;
	switch (button_number) {
	case 1:
//		HAL_UART_Transmit(&huart3, "11", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(1);

		if (menuOption[0].flag)
			rotateDirection90(&player[0]);

		break;
	case 2:
//		HAL_UART_Transmit(&huart3, "12", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(2);

		break;
	case 3:
//		HAL_UART_Transmit(&huart3, "13", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(3);

		if (menuOption[0].flag)
			rotateDirection90(&player[1]);

		/* code */
		break;
	case 4:
//		HAL_UART_Transmit(&huart3, "14", 2, HAL_MAX_DELAY);

		/* change needed flags to display menu */
		if (initPageFlag == 1) {
			initPageFlag = 0;
			menuFlag = 1;
			menuPageExecuted = 0;
			clearLcdFlag = 1;
		}

		break;
	case 5:
//		HAL_UART_Transmit(&huart3, "21", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(4);

		if (menuOption[0].flag)
			move(&player[0]);

		/* code */
		break;
	case 6:
//		HAL_UART_Transmit(&huart3, "22", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(5);

		/* code */
		break;
	case 7:
//		HAL_UART_Transmit(&huart3, "23", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(6);

		if (menuOption[0].flag)
			move(&player[1]);

		/* code */
		break;
	case 8:
//		HAL_UART_Transmit(&huart3, "24", 2, HAL_MAX_DELAY);

		/* upward key on menu */
		if (menuFlag) {
			changePointer(&menuPointerLocation, &menuPageExecuted, "up");
		}

		/* upward key on setting */
		if (menuOption[1].flag) {
			changePointer(&settingPointerLocation, &menuOption[1].pageExecuted,
					"up");
		}

		break;
	case 9:
//		HAL_UART_Transmit(&huart3, "31", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(7);

		if (menuOption[0].flag)
			shoot(&player[0]);
		/* code */
		break;
	case 10:
//		HAL_UART_Transmit(&huart3, "32", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(8);

		/* code */
		break;
	case 11:
//		HAL_UART_Transmit(&huart3, "33", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(9);

		if (menuOption[0].flag)
			shoot(&player[1]);
		/* code */
		break;
	case 12:
//		HAL_UART_Transmit(&huart3, "34", 2, HAL_MAX_DELAY);

		/* downward key on menu */
		if (menuFlag)
			changePointer(&menuPointerLocation, &menuPageExecuted, "down");

		/* downward key on setting */
		if (menuOption[1].flag)
			changePointer(&settingPointerLocation, &menuOption[1].pageExecuted,
					"down");

		break;
	case 13:
//		HAL_UART_Transmit(&huart3, "41", 2, HAL_MAX_DELAY);
		/* code */
		break;
	case 14:
//		HAL_UART_Transmit(&huart3, "42", 2, HAL_MAX_DELAY);

		if (menuOption[1].flag)
			setSettingValues(0);

		/* code */
		break;
	case 15:

//		HAL_UART_Transmit(&huart3, "43", 2, HAL_MAX_DELAY);

		/* back key to menu */
		menuOption[0].flag = 0, menuOption[1].flag = 0, menuOption[2].flag = 0;
//		menuOption[0].pageExecuted = 0, menuOption[1].pageExecuted = 0, menuOption[2].pageExecuted= 0;
		menuFlag = 1;
		menuPageExecuted = 0;
		initPageFlag = 0;
		clearLcdFlag = 1;
		break;
	case 16:
//		HAL_UART_Transmit(&huart3, "44", 2, HAL_MAX_DELAY);

		/* enter key for menu */
		if (menuFlag) {
			if (menuPointerLocation >= 0) {
				menuOption[menuPointerLocation].flag = 1;
				if (menuOption[menuPointerLocation].pageExecuted != NULL) {
					menuOption[menuPointerLocation].pageExecuted = 0;
				}
			}
			menuFlag = 0;
			initPageFlag = 0;
			clearLcdFlag = 1;
		}
		break;

	default:
//		HAL_UART_Transmit(&huart3, "b", 1, HAL_MAX_DELAY);

		break;
	}
}

int counter = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM1) {
		int x = (player[1].numBullet * 1000) + (player[1].health * 100)
				+ (player[0].numBullet * 10) + (player[0].health);
		programInit(x);
		programLoop();

		counter++;
		if (counter == 30000 && menuOption[0].flag) {
			generateQM();
			counter = 0;
		}
	}

//    if (htim->Instance == TIM4)
//    {
//    	refresh();
//    }
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

	LiquidCrystal(GPIOD, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10,
	GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14);

	begin(20, 4);
	setCursor(0, 0);

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM1_Init();
	MX_RTC_Init();
	MX_TIM2_Init();
	MX_USART3_UART_Init();
	/* USER CODE BEGIN 2 */

	HAL_GPIO_WritePin(Column_ports[0], Column_pins[0], 1);
	HAL_GPIO_WritePin(Column_ports[1], Column_pins[1], 1);
	HAL_GPIO_WritePin(Column_ports[2], Column_pins[2], 1);
	HAL_GPIO_WritePin(Column_ports[3], Column_pins[3], 1);

	HAL_TIM_Base_Start_IT(&htim1);
//	HAL_TIM_Base_Start_IT(&htim4);
	HAL_UART_Receive_IT(&huart3, &rx_byte, 1);

	clear();

	initPageFlag = 1;
//	menuOption[0].flag = 1;
//	uint32_t stateTime = -1;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (timeFlag) {
			setRtcTime();
			timeFlag = 0;
		}
		if (clearLcdFlag == 1) {
			clear();
			clearLcdFlag = 0;
		}

		if (initPageFlag == 1 && initPageExecuted == 0) {
			/* display name of game & play a song */
			begin_page();
			initPageExecuted = 1;

		} else if (initPageFlag == 0) {
			if (menuFlag == 1 && menuPageExecuted == 0) {
				emptyArray();
				/* menu display */
				menuDisplay();
				menuPageExecuted = 1;
			}

			if (menuOption[0].flag == 1 && menuOption[0].pageExecuted == 0) {
				/* clear lcd and start game */
				PWM_Stop();
				initPlayers();
//				int x = (player[1].numBullet * 1000) + (player[1].health * 100)
//						+ (player[0].numBullet * 10) + (player[0].health);
//				programInit(x);

				initGamePage();

				menuOption[0].pageExecuted = 1;

			} else if (menuOption[1].flag == 1
					&& menuOption[1].pageExecuted == 0) {
				/* clear lcd and display setting */
				setting_page();
				menuOption[1].pageExecuted = 1;

			} else if (menuOption[2].flag == 1) {
				/* clear lcd and name of group member and RTC time(live update) */
				aboutUs_page();
			}
		}

//		moveBullets();

		if (menuOption[0].pageExecuted == 1
				&& (player[0].isAlive == 0 || player[1].isAlive == 0)) {
			//finaltime();
			//uart end
			clear();
			if (player[0].isAlive == 0) {
				HAL_UART_Transmit(&huart3, player[1].name,
						strlen(player[1].name), HAL_MAX_DELAY);
				setScore(player[1].score);
			} else {
				HAL_UART_Transmit(&huart3, player[0].name,
						strlen(player[0].name), HAL_MAX_DELAY);
				setScore(player[0].score);
			}
			//random animation
			for(int i=0;i<19;i++){
					setCursor(i, 1);
					write(1);
			}

		} else
			refresh();

		HAL_Delay(600);

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
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3
			| RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_TIM1;
	PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef sDate = { 0 };

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	 */
	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 0x0;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 64 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1000 - 1;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 4294967295;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
	HAL_TIM_MspPostInit(&htim2);

}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 9600;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			CS_I2C_SPI_Pin | GPIO_PIN_7 | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin
					| LD9_Pin | LD10_Pin | LD8_Pin | LD6_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_2, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : CS_I2C_SPI_Pin PE7 LD4_Pin LD3_Pin
	 LD5_Pin LD7_Pin LD9_Pin LD10_Pin
	 LD8_Pin LD6_Pin */
	GPIO_InitStruct.Pin = CS_I2C_SPI_Pin | GPIO_PIN_7 | LD4_Pin | LD3_Pin
			| LD5_Pin | LD7_Pin | LD9_Pin | LD10_Pin | LD8_Pin | LD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : MEMS_INT4_Pin MEMS_INT1_Pin */
	GPIO_InitStruct.Pin = MEMS_INT4_Pin | MEMS_INT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : PC2 PC3 */
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : PF2 */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PA1 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MISOA7_Pin */
	GPIO_InitStruct.Pin = SPI1_SCK_Pin | SPI1_MISO_Pin | SPI1_MISOA7_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB1 PB2 PB4 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : DM_Pin DP_Pin */
	GPIO_InitStruct.Pin = DM_Pin | DP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF14_USB;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PD1 PD2 PD3 PD4 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : PD5 PD6 PD7 */
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : I2C1_SCL_Pin I2C1_SDA_Pin */
	GPIO_InitStruct.Pin = I2C1_SCL_Pin | I2C1_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_TSC_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI2_TSC_IRQn);

	HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

}

/* USER CODE BEGIN 4 */

void begin_page() {
	setCursor(0, 0);
	print("Welcome to the Tank Battle");
	play_song();

}

void menuDisplay() {
	setCursor(0, menuPointerLocation);
//	write(0);
	print("-");

	setCursor(1, 0);
	print("start game");
	setCursor(1, 1);
	print("setting");
	setCursor(1, 2);
	print("about us");
}

void play_song() {
	call_change_melody();
	return;
}

void aboutUs_page() {
	setCursor(0, 0);

	for (int i = 0; i < 2; i++) {
		print(teamMembers[i]);
		print(" ");
	}

	char timeStr[20], dateStr[20];

	HAL_RTC_GetTime(&hrtc, &mytime, RTC_FORMAT_BIN);

	setCursor(0, 1);
	sprintf(timeStr, "%2d:%2d:%2d", mytime.Hours, mytime.Minutes,
			mytime.Seconds);
	print(timeStr);

	HAL_RTC_GetDate(&hrtc, &mydate, RTC_FORMAT_BIN);

	setCursor(0, 2);
	sprintf(dateStr, "%d - %d - %d", mydate.Year, mydate.Month, mydate.Date);
	print(dateStr);

}

void setting_page() {
	setCursor(0, settingPointerLocation);
	print("-");

	setCursor(1, 0);

	char temp[21];
	sprintf(temp, "df health: %d", dfSetting.health);
	print(temp);

	setCursor(1, 1);

	strcpy(temp, ""); // Clears the previous content
	sprintf(temp, "df num blt: %d", dfSetting.numBullet);
	print(temp);

	setCursor(1, 2);

	strcpy(temp, ""); // Clears the previous content
	sprintf(temp, "df sounEeffect: %d", dfSetting.soundEfect);
	print(temp);

	setCursor(1, 4);

	print(teamMembers[0]);
	print(" ");
	print(teamMembers[1]);

	free(temp);

}

void setRtcTime() {
	mytime.Hours = (int) hour;
	mytime.Minutes = (int) minute;
	mytime.Seconds = (int) second;

	if (HAL_RTC_SetTime(&hrtc, &mytime, RTC_FORMAT_BIN) != HAL_OK) {
		Error_Handler();
	}

	mydate.Year = year;
	mydate.Month = month;
	mydate.Date = day;

	if (HAL_RTC_SetDate(&hrtc, &mydate, RTC_FORMAT_BIN) != HAL_OK) {
		Error_Handler();
	}
}

void setSettingValues(int val) {
	switch (settingPointerLocation) {
	case 0:
		dfSetting.health = val;
		break;
	case 1:
		dfSetting.numBullet = val;
		break;
	case 2:
		if (val == 0 || val == 1)
			dfSetting.soundEfect = val;
		break;
	default:
		break;
	}
	menuOption[1].pageExecuted = 0;
}

void changePointer(int *ptrLoc, int *pageExecuted, char *dir) {
	if (strcmp(dir, "up") == 0) {
		*ptrLoc = (*ptrLoc + 1) % 3;
	} else if (strcmp(dir, "down") == 0) {
		*ptrLoc = (*ptrLoc - 1 + 3) % 3;
	}
	*pageExecuted = 0;
	clearLcdFlag = 1;
}

void customCharsCreate() {
	createChar(0, tankRightChar);
	createChar(1, tankDownChar);
	createChar(2, tankLeftChar);
	createChar(3, tankUpChar);
	createChar(4, bulletChar);
	createChar(5, healthChar);
	createChar(6, barrierChar);
	createChar(7, questionmarkChar);
}

void initPlayers() {

	player[0] = (Player ) { "Alireza", dfSetting.health, dfSetting.numBullet,
					RIGHT, 0, 2, 1, 0 };
	player[1] = (Player ) { "Ali", dfSetting.health, dfSetting.numBullet, LEFT,
					19, 1, 1, 0 };

	array[0][2] = 1;
	array[19][1] = 3;
}

void initGamePage() {
	array[1][1] = 8;
	array[1][2] = 8;
	array[18][1] = 8;
	array[18][2] = 8;

	customCharsCreate();

	Pair pairs[15];
	generatePairs(pairs, 15, 0);

	//amirali
	uint8_t pointerrow;
	uint8_t pointercolumn;

	for (int i = 0; i < 15; ++i) {
		pointerrow = pairs[i].first;
		pointercolumn = pairs[i].second;

		//0-5 for bullet
		if (i < 5) {
			array[pointercolumn][pointerrow] = 5;
		} //5-8 for health
		else if (i < 8) {
			array[pointercolumn][pointerrow] = 6;
		} // 8-14 for barrier
		else if (i < 14) {
			array[pointercolumn][pointerrow] = 7;
		} // 15 for questionmark
		else {
			array[pointercolumn][pointerrow] = 9;
		}
	}
}

int rflag = 0;
void refresh() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 4; j++) {
			if (array[i][j] != prevarray[i][j]) {
				int x = array[i][j];
				setCursor(i, j);
				switch (x) {
				case 0:
					print(" ");
					break;
				case 1:
					write(0);
					break;
				case 2:
					write(1);
					break;
				case 3:
					write(2);
					break;
				case 4:
					write(3);
					break;
				case 5:
					write(4);
					break;
				case 6:
					write(5);
					break;
				case 7:
					write(6);
					break;
				case 8:
					print("|");
					break;
				case 9:
					write(7);
					break;
//				case 10:
//					print(".");
//					break;
				default:
					break;
				}
				prevarray[i][j] = array[i][j];
			}
		}
	}
	if (shootflag == 1) {
		for (int k = 0; k < num_shooted_bullet; k++) {
			if (bullets[k].isAlive) {

				int col = bullets[k].col;
				int row = bullets[k].row;

				switch (bullets[k].direction) {
				case RIGHT:
					bullets[k].col++;
					break;
				case DOWN:
					bullets[k].row++;
					break;
				case LEFT:
					bullets[k].col--;
					break;
				case UP:
					bullets[k].row--;
					break;
				default:
					break;
				}

				int f = array[bullets[k].col][bullets[k].row];

				if (f == 5 || f == 6 || f == 9) { // if health / bulletIcon / chance
					do {
						switch (bullets[k].direction) {
						case RIGHT:
							bullets[k].col++;
							break;
						case DOWN:
							bullets[k].row++;
							break;
						case LEFT:
							bullets[k].col--;
							break;
						case UP:
							bullets[k].row--;
							break;
						default:
							break;
						}
						f = array[bullets[k].col][bullets[k].row];
					} while (f == 5 || f == 6 || f == 9);
				} else if (f == 8) { //if wall
					bullets[k].isAlive = 0;
				} else if (f == 7) { //if barrier
					array[bullets[k].col][bullets[k].row] = 0;
					bullets[k].isAlive = 0;
				} else if (bullets[k].col == player[0].col
						&& bullets[k].row == player[0].row) {
					player[1].score++;
					decreaseHealth(&player[0]);
					bullets[k].isAlive = 0;
				} else if (bullets[k].col == player[1].col
						&& bullets[k].row == player[1].row) {
					player[0].score++;
					decreaseHealth(&player[1]);
					bullets[k].isAlive = 0;
				} else if (bullets[k].col < 0 || bullets[k].col >= 20
						|| bullets[k].row < 0 || bullets[k].row >= 4) {
					bullets[k].isAlive = 0;
				}

//				for (int m = 0; m < num_shooted_bullet; m++) {
//					if (bullets[k].col == bullets[m].col
//							&& bullets[k].row == bullets[m].row) {
//						bullets[k].isAlive = 0;
//						bullets[m].isAlive = 0;
//					}
//				}

				if (!((col == player[0].col && row == player[0].row)
						|| (col == player[1].col && row == player[1].row))) {
					setCursor(col, row);
					print(" ");
					HAL_Delay(500);
				}

				if (bullets[k].isAlive) {
					setCursor(bullets[k].col, bullets[k].row);
					print(".");
				} else {
					bullets[k].col = -1;
					bullets[k].row = -1;
				}
			}
		}
//		num_shooted_bullet--;
	}
}

void shoot(Player *player) {
	if (hasBullet(player) == 0) {
		return;
	}
	decreaseBullet(player);
//	playShotSound();

	num_shooted_bullet++;

	for (int i = 0; i < num_shooted_bullet; i++) {
		if (!bullets[i].isAlive) {
			bullets[i].col = player->col;
			bullets[i].row = player->row;
			bullets[i].direction = player->direction;
			bullets[i].isAlive = 1;

			shootflag = 1;
			break;
		}
	}
}

//void processBullet(){
//
//}

void move(Player *player) {
	int moveflag = 0;

	int column = player->col;
	int row = player->row;

	int tankType = 0;

//	const char *currentDirection = player->direction;
	enum Direction currentDirection = player->direction;

	switch (currentDirection) {
	case RIGHT:
		if (column < 19 && checkMovePossible(column + 1, row)) {
			player->col++;
			player->direction = RIGHT;
			tankType = 1;
			moveflag = 1;
		}
		break;
	case LEFT:
		if (column > 0 && checkMovePossible(column - 1, row)) {
			player->col--;
			player->direction = LEFT;
			tankType = 3;
			moveflag = 1;
		}
		break;
	case DOWN:
		if (row < 3 && checkMovePossible(column, row + 1)) {
			player->row++;
			player->direction = DOWN;
			tankType = 2;
			moveflag = 1;
		}
		break;
	case UP:
		if (row > 0 && checkMovePossible(column, row - 1)) {
			player->row--;
			player->direction = UP;
			tankType = 4;
			moveflag = 1;
		}
		break;
	default:
		break;
	}
	if (moveflag) {
		//check if item exist in new location and apply item. (before change type of new loc in array)
		checkItemExist(player);

		array[column][row] = 0;
		array[player->col][player->row] = tankType;
	}

}
int coun = 0;
void finaltime() {

	if (player[0].isAlive == 0) {
		clear();
		setCursor(1, 2);
		coun++;

		if (coun > 15000) {
			write(0);
			coun = 0;
		} else if (coun > 10000) {
			write(1);
		} else if (coun > 5000) {
			write(2);
		} else {
			write(3);
		}
		setCursor(1, 3);
		print(player[1].name);
		setCursor(2, 3);
		print("score:");
		print(" ");
		print(player[1].score);

	} else if (player[1].isAlive == 0) {
		clear();
		setCursor(1, 2);
		coun++;
		if (coun > 15000) {
			write(0);

		} else if (coun > 10000) {
			write(1);
		} else if (coun > 5000) {
			write(2);
		} else {
			write(3);
		}
		setCursor(1, 3);
		print(player[0].name);
		setCursor(2, 3);
		print("score:");
		print(" ");
		print(player[0].score);
	}

}

int hasBullet(Player *player) {
	if (player->numBullet == 0) {
		return 0;
	}
	return 1;
}

void generateQM() {
	Pair pair[1];
//	HAL_UART_Transmit(&huart3, "g", 1, HAL_MAX_DELAY);
	generatePairs(pair, 1, 1);

//	char x[5];
//	sprintf(x, "%d", pair[0].second);
//	HAL_UART_Transmit(&huart3, x, strlen(x), HAL_MAX_DELAY);

	array[pair[0].second][pair[0].first] = 9;
}

void checkItemExist(Player *player) {

	int x = array[player->col][player->row];

	if (x == 5)	//bullet
		increaseBullet(player);
	else if (x == 6)	//health
		increaseHealth(player);

	else if (x == 9) {
		int rand = HAL_GetTick() % 2;
		if (rand == 0) {
			increaseBullet(player);
		} else if (rand == 1) {
			increaseHealth(player);
		}
	}
}

int checkMovePossible(int col, int row) {
	int x = array[col][row];
	if (x == 1 || x == 2 || x == 3 || x == 4) {		// if tank is ahead
		return 0;
	}
	if (x == 7 || x == 8) {						// wall or barrier is ahead
		return 0;
	}
	return 1;
}

const char *directions[] = { "right", "down", "left", "up" };

void rotateDirection90(Player *player) {

	enum Direction currentDirection = player->direction;

	for (int i = 0; i < 4; i++) {
		if (currentDirection == i) {
			// Rotate clockwise 90 degrees
			player->direction = (enum Direction) ((i + 1) % 4);

			int x = 0;
			switch (player->direction) {
			case RIGHT:
				x = 1;
				break;
			case DOWN:
				x = 2;
				break;
			case LEFT:
				x = 3;
				break;
			case UP:
				x = 4;
				break;
			default:
				break;
			}

			array[player->col][player->row] = x;
			break;
		}
	}
//	const char *currentDirection = player->direction;
//
//	for (int i = 0; i < 4; i++) {
//		if (strcmp(currentDirection, directions[i]) == 0) {
//			// Rotate clockwise 90 degrees
//			player->direction = directions[(i + 1) % 4];
//
//			int x = 0;
//			if (strcmp(player->direction, "right") == 0) {
//				// Write the tank character for the new "right" direction
//				x = 1;
//			} else if (strcmp(player->direction, "down") == 0) {
//				// Write the tank character for the new "down" direction
//				x = 2;
//			} else if (strcmp(player->direction, "left") == 0) {
//				// Write the tank character for the new "left" direction
//				x = 3;
//			} else if (strcmp(player->direction, "up") == 0) {
//				// Write the tank character for the new "up" direction
//				x = 4;
//			}
//
//			array[player->col][player->row] = x;
//
//			break;
//		}
//	}
}

int isDuplicate(Pair arr[], int index, int first, int second, int duringGame) {
	if (duringGame) {
		if (array[second][first] != 0) {
//			HAL_UART_Transmit(&huart3, "p", 1, HAL_MAX_DELAY);
			return 1;
		}
	} else if (duringGame == 0)
		for (int i = 0; i < index; ++i) {
			if (arr[i].first == first && arr[i].second == second) {
				return 1;  // Duplicate found
			}
		}
	return 0;  // No duplicate found
}

void generatePairs(Pair arr[], int n, int duringGame) {
	for (int i = 0; i < n; ++i) {
		int first = HAL_GetTick() % 4; // Generates a random number between 0 and 4
		int second = HAL_GetTick() % 16 + 2; // Generates a random number between 2 and 17

		while (isDuplicate(arr, i, first, second, duringGame)) {
			first = HAL_GetTick() % 4;
			second = HAL_GetTick() % 16 + 2;
		}

		arr[i].first = first;
		arr[i].second = second;
	}
//	HAL_UART_Transmit(&huart3, "l", 1, HAL_MAX_DELAY);

}

void copyArray() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 4; j++) {
			prevarray[i][j] = array[i][j];
		}
	}
}

void emptyArray() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 4; j++) {
			array[i][j] = 0;
			prevarray[i][j] = 0;
		}
	}
}

void increaseBullet(Player *player) {
	if (player->numBullet < 9) {
		player->numBullet++;
	}
}

void decreaseBullet(Player *player) {
	if (player->numBullet > 0) {
		player->numBullet--;
	}
}

void increaseHealth(Player *player) {
	if (player->health < 9) {
		player->health++;
	}
}

void decreaseHealth(Player *player) {
	if (player->health > 0) {
		player->health--;
//		playExplosionSound();
	}  if (player->health <= 0) {
		player->isAlive = 0;
	}
}
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
