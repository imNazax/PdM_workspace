/**
 ******************************************************************************
 * @file    API_uart.c
 * @brief   Implementación del módulo UART.
 *
 * Capa de abstracción sobre HAL_UART para USART2 en modo polling (115200/8N1).
 * Todas las funciones validan sus parámetros antes de operar.
 ******************************************************************************
 */

#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/* ── Configuración ────────────────────────────────────────────────────────── */
#define UART_TIMEOUT_MS   100u
#define UART_MAX_SIZE     256u

/* ── Variable externa generada por CubeMX ────────────────────────────────── */
extern UART_HandleTypeDef huart2;

/* ── Implementación ───────────────────────────────────────────────────────── */

/**
 * @brief  Verifica que la UART esté inicializada e imprime los parámetros
 *         de configuración por la terminal serie.
 *
 * La inicialización de hardware la realiza MX_USART2_UART_Init() en main.c.
 * Esta función solo verifica el estado y envía el mensaje informativo.
 *
 * @retval true  si la UART está lista y el mensaje se envió correctamente
 * @retval false si el periférico no está listo o falló la transmisión
 */
bool_t uartInit(void)
{
    if (huart2.Instance == NULL)
        return false;

    if (HAL_UART_GetState(&huart2) == HAL_UART_STATE_RESET)
        return false;

    const uint8_t msg[] =
        "\r\n=== UART inicializada ===\r\n"
        "Puerto   : USART2\r\n"
        "Baudrate : 115200\r\n"
        "Formato  : 8N1\r\n"
        "Modo     : Polling\r\n"
        "=========================\r\n";

    uint16_t len = (uint16_t)strlen((const char *)msg);

    if (HAL_UART_Transmit(&huart2, (uint8_t *)msg, len, UART_TIMEOUT_MS) != HAL_OK)
        return false;

    return true;
}

/**
 * @brief  Envía un string completo por UART hasta encontrar '\0'.
 */
void uartSendString(uint8_t *pstring)
{
    if (pstring == NULL)
        return;

    uint16_t len = (uint16_t)strlen((const char *)pstring);

    if (len == 0 || len > UART_MAX_SIZE)
        return;

    HAL_UART_Transmit(&huart2, pstring, len, UART_TIMEOUT_MS);
}

/**
 * @brief  Envía exactamente size bytes por UART.
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL || size == 0 || size > UART_MAX_SIZE)
        return;

    HAL_UART_Transmit(&huart2, pstring, size, UART_TIMEOUT_MS);
}

/**
 * @brief  Recibe exactamente size bytes por UART en modo polling.
 */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL || size == 0 || size > UART_MAX_SIZE)
        return;

    HAL_UART_Receive(&huart2, pstring, size, UART_TIMEOUT_MS);
}
