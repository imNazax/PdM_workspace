/**
 ******************************************************************************
 * @file    API_uart.c
 * @brief   Implementación del módulo UART.
 *
 * Validaciones de parámetros, comprobación de estado HAL y manejo de errores.
 ******************************************************************************
 */

#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/* Configuración */
#define UART_TIMEOUT_MS   100u
#define UART_MAX_SIZE     256u

/* Handle externo generado por CubeMX */
extern UART_HandleTypeDef huart2;

/* Funciones públicas ------------------------------------------------------- */

bool_t uartInit(void)
{
    /* Validar handle */
    if (&huart2 == NULL)
        return false;

    /* Verificar que la instancia esté configurada */
    if (huart2.Instance == NULL)
        return false;

    /* Verificar estado HAL (no reset) */
    if (HAL_UART_GetState(&huart2) == HAL_UART_STATE_RESET)
        return false;

    /* Mensaje informativo */
    const uint8_t msg[] =
        "\r\n=== UART inicializada ===\r\n"
        "Puerto   : USART2\r\n"
        "Baudrate : 115200\r\n"
        "Formato  : 8N1\r\n"
        "Modo     : Polling\r\n"
        "=========================\r\n";

    /* Enviar mensaje y comprobar retorno */
    size_t len = strlen((const char *)msg);
    if (len == 0 || len > UART_MAX_SIZE)
        return false;

    if (HAL_UART_Transmit(&huart2, (uint8_t *)msg, (uint16_t)len, UART_TIMEOUT_MS) != HAL_OK)
    {
        /* Intento de notificar error por UART falló; devolvemos false */
        return false;
    }
    return true;
}

void uartSendString(uint8_t *pstring)
{
    if (pstring == NULL)
        return;

    size_t len = strlen((const char *)pstring);

    if (len == 0 || len > UART_MAX_SIZE)
        return;

    if (HAL_UART_Transmit(&huart2, pstring, (uint16_t)len, UART_TIMEOUT_MS) != HAL_OK)
    {
        /* Opcional: manejar error (no bloqueante) */
    }
}

void uartSendStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL)
        return;

    if (size == 0 || size > UART_MAX_SIZE)
        return;

    if (HAL_UART_Transmit(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK)
    {
        /* Opcional: manejar error */
    }
}

void uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL)
        return;

    if (size == 0 || size > UART_MAX_SIZE)
        return;

    /* Recibe en modo polling; la función bloquea hasta recibir 'size' bytes o timeout */
    if (HAL_UART_Receive(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK)
    {
        /* Opcional: manejar error */
    }
}
