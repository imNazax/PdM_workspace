/* USER CODE BEGIN Header */
/**
 *****************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */
#include "main.h"

/* USER CODE BEGIN Includes */
#include "API_delay.h"
#include "API_debounce.h"
#include "API_uart.h"
#include "API_cmdparser.h"
/* USER CODE END Includes */

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* Períodos de parpadeo del LED en milisegundos */
#define DELAY_FAST_MS  ((tick_t)100)
#define DELAY_SLOW_MS  ((tick_t)500)

static delay_t delayLed;
static tick_t  blinkPeriod  = DELAY_FAST_MS;
static bool_t  manualMode   = false;   /* true = LED bajo control UART */

/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* ── Callbacks para el parser ──────────────────────────────────────────────── */

/**
 * @brief  Ejecuta la acción sobre el LED solicitada por el parser.
 *         Activa el modo manual para que el parpadeo se detenga.
 */
static void ledSetAction(led_action_t action)
{
    switch (action)
    {
        case LED_ACTION_ON:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            break;
        case LED_ACTION_OFF:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            break;
        case LED_ACTION_TOGGLE:
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            break;
        default:
            break;
    }
    manualMode = true;  /* detener el parpadeo automático */
}

/**
 * @brief  Devuelve el estado actual del LED (para el comando STATUS).
 */
static bool ledReadState(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET);
}

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/* ── main ──────────────────────────────────────────────────────────────────── */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    if (!uartInit())
        Error_Handler();

    cmdParserInit(ledSetAction, ledReadState);

    /* Estado inicial: LED apagado, parpadeo automático */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    manualMode  = false;
    blinkPeriod = DELAY_FAST_MS;

    delayInit(&delayLed, blinkPeriod);
    debounceFSM_init();

    while (1)
    {
        debounceFSM_update();
        cmdPoll();

        /* Botón: en modo manual vuelve al parpadeo; en modo automático
         * alterna la velocidad de parpadeo. */
        if (readKey())
        {
            if (manualMode)
            {
                manualMode = false;
                delayInit(&delayLed, blinkPeriod); /* reiniciar para evitar toggle inmediato */
            }
            else
            {
                blinkPeriod = (blinkPeriod == DELAY_FAST_MS) ? DELAY_SLOW_MS : DELAY_FAST_MS;
                delayWrite(&delayLed, blinkPeriod);
            }
        }

        /* Parpadeo automático: solo fuera del modo manual */
        if (!manualMode && delayRead(&delayLed))
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /* Pulsador PC13: entrada con pull-up (activo en bajo) */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* LED PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
