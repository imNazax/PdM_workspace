/**
 * *****************************************************************************
 * @file    uart.c
 * @brief   Lógica genérica del driver de comunicación UART.
 *
 * Este archivo maneja el formateo y construcción de mensajes.
 * NO depende de ningún HAL — toda transmisión se hace a través
 * de las funciones de port.c.
 *
 * Si se migra a otro microcontrolador, este archivo NO se modifica.
 * Solo se reescribe port.c.
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

/* ── Includes ──────────────────────────────────────────────────────────────── */
#include "uart.h"
#include <string.h>
#include <stdio.h>

/* ── Prototipos de port.c ──────────────────────────────────────────────────── */

/** Envía un buffer de bytes por UART. */
extern void port_uart_send_bytes(uint8_t *buf, uint16_t len);

/* ── Variables privadas ────────────────────────────────────────────────────── */
static char uartBuffer[UART_BUFFER_SIZE];

/* ═══════════════════════════════════════════════════════════════════════════ */
/*                       Funciones públicas                                   */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Inicializa el driver UART y envía mensaje de confirmación.
 */
void UART_Init(void)
{
    UART_SendString("[UART] Inicializado correctamente.\r\n");
}

/**
 * @brief  Envía un string por UART terminado en '\0'.
 * @param  texto  Puntero al string a transmitir.
 */
void UART_SendString(char *texto)
{
    if (texto == NULL)
        return;

    uint16_t len = (uint16_t)strlen(texto);

    if (len > 0)
    {
        port_uart_send_bytes((uint8_t *)texto, len);
    }
}

/**
 * @brief  Formatea y envía temperatura y humedad por UART.
 * @param  temperatura  Valor en °C.
 * @param  humedad      Valor en %.
 *
 * Formato: "T=23.5 H=60.2\r\n"
 */
void UART_SendData(float temperatura, float humedad)
{
    snprintf(uartBuffer, UART_BUFFER_SIZE, "T=%.1f H=%.1f\r\n",
             temperatura, humedad);

    UART_SendString(uartBuffer);
}
