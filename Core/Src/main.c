//
//	André A. M. Araújo
//	2021/12/17
//
//	Making Embedded Systems Course at Classpert
//	Exercise 4 - Blinky
//
//	This program toggles the Blue LED every time the user button is pressed.
//	The button input generates an interrupt that starts a timer to debounce
//	the switching signal, without blocking the main loop.
//
//	Also, the Green LED blinks at 2 Hz, controlled by an independent timer interrupt
//
//	Target: STM32L152RB (STM32L-Discovery Board)
//	Button:			PC6, internal pull up, active low, rising and falling edges detection
//	Blue  LED: 		PB6
//	Green LED: 		PB7
//	Debounce timer:	TIM6
//	Blinking timer: TIM7
//
//	Developed in STM32CubeIDE 1.8.0
//

#include "main.h"

#define DEBOUNCE_STABLE_PERIOD 10				// Debounce period [ms]

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

// Global Variables
volatile uint8_t  toggleGreenLED = 0;			// Flags for toggling LEDs on main loop
volatile uint8_t  toggleBlueLED  = 0;			//

volatile uint16_t currentButton  = 0;			// Variables to store button states
volatile uint16_t previousButton = 0;			//

volatile uint16_t debounceCounter = 0;			// Variables for button debounce
volatile uint8_t  debouncedButtonPressed  = 0;	//
volatile uint8_t  debouncedButtonReleased = 0;	//

// Function prototypes (system initialization; from STM32CubeMX)
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);

// Main loop
int main(void)
{
	HAL_Init();				// System initialization:
	SystemClock_Config();	//	SYSCLK == 32 MHz (16 MHz HSI + PLL)
	MX_GPIO_Init();			//	GPIO: See attached file
	MX_TIM6_Init();			// 	Timer 6 for timed button debounce
	MX_TIM7_Init();			// 	Timer 7 for blinking Green LED

	currentButton = HAL_GPIO_ReadPin(BUTTON_USER_GPIO_Port, BUTTON_USER_Pin);
	HAL_TIM_Base_Start_IT(&htim7);			// Start Timer 7 for blinking Green LED

	while (1)
	{
		if (debouncedButtonPressed != 0)	// Blue LED is toggled every time
		{									// the button is pressed
			HAL_GPIO_WritePin (OUT_TEST_GPIO_Port, OUT_TEST_Pin, 0);// Turn off test output
			HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);	// Toggle Blue LED
			debouncedButtonPressed = 0;
		}

		if (debouncedButtonReleased != 0)	// Interrupts are also generated when
		{									// button is released
			debouncedButtonReleased = 0;
		}

		if (toggleGreenLED != 0)			// Green LED blinks at 2Hz
		{									// according to Timer 7 interrupts
			HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);	// Toggle Green LED
			toggleGreenLED = 0;
		}
	}
}

// Function declarations
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType 	  = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState 			  = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState 		  = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource 	  = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL 		  = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PLLDIV 		  = RCC_PLL_DIV3;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType 	 = RCC_CLOCKTYPE_HCLK |RCC_CLOCKTYPE_SYSCLK
							  	      |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_TIM6_Init(void)
{
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim6.Instance 				 = TIM6;		// Timer 6 configuration:
	htim6.Init.Prescaler 		 = (16000 - 1);	//	Increments at 2000 Hz
	htim6.Init.CounterMode 		 = TIM_COUNTERMODE_UP;
	htim6.Init.Period 			 = (2 - 1);		// 	Interrupts every 1 ms
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode 	  = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_TIM7_Init(void)
{
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim7.Instance 				 = TIM7;		// Timer 7 configuration:
	htim7.Init.Prescaler 		 = (32000 - 1);	//  Increments at 1000 Hz
	htim7.Init.CounterMode 		 = TIM_COUNTERMODE_UP;
	htim7.Init.Period 			 = (250 - 1);	//  Interrupts every 250 ms
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode 	  = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOB, LED_BLUE_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(OUT_TEST_GPIO_Port, OUT_TEST_Pin, GPIO_PIN_RESET);

	// Input pin for user button, GPIO Pull-up (Active LOW)
	GPIO_InitStruct.Pin   = BUTTON_USER_Pin;
	GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	HAL_GPIO_Init(BUTTON_USER_GPIO_Port, &GPIO_InitStruct);

	// Test output pin to measure button debounce time with oscilloscope
	GPIO_InitStruct.Pin   = OUT_TEST_Pin;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(OUT_TEST_GPIO_Port, &GPIO_InitStruct);

	// LED outputs
	GPIO_InitStruct.Pin   = LED_BLUE_Pin|LED_GREEN_Pin;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);	// Enable button interrupts
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);			//
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim6)	// Timed debounce routine:
	{					//	After first edge is detected, accumulate
						// 	DEBOUNCE_STABLE_PERIOD equal samples and
						// 	finally set PRESSED or RELEASED flag and
						//	disables this counter until next edge occurs
		previousButton = currentButton;
		currentButton  = HAL_GPIO_ReadPin(BUTTON_USER_GPIO_Port, BUTTON_USER_Pin);

		if (currentButton == previousButton)	// Increments counter if stable
		{
			debounceCounter++;
		}
		else									// Resets counter if bounce occurs
		{
			debounceCounter = 0;
		}

		if (debounceCounter >= DEBOUNCE_STABLE_PERIOD)
		{	// Debounce finished
			HAL_TIM_Base_Stop_IT(&htim6);
			debounceCounter = 0;

			if (currentButton == 0)			// Active LOW: Button Pressed == 0
			{
				debouncedButtonPressed = 1;
			}
			else
			{
				debouncedButtonReleased = 1;
			}
		}
	}
	else if (htim == &htim7)	// Timed Green LED blinky
	{
		toggleGreenLED = 1;		// Toggles LED every 250 ms, one cycle every 500 ms (2 Hz)
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == BUTTON_USER_Pin)		// When edge is detected,
	{									//	Timer 6 starts, for button debounce
		HAL_GPIO_WritePin(OUT_TEST_GPIO_Port, OUT_TEST_Pin, 1);
		HAL_TIM_Base_Start_IT(&htim6);
	}
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
