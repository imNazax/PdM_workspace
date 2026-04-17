/**
 * *****************************************************************************
 * @file    port.c  (UART)
 * @brief   Funciones dependientes del hardware para el driver UART.
 *
 * Este archivo es el ÚNICO que "sabe" que existe un STM32 y su HAL UART.
 * Si se migra a otro micro, solo se reescribe este archivo.
 *
 * Usa USART2, que está conectada internamente al ST-LINK y sale por USB.
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

/* ── Includes ──────────────────────────────────────────────────────────────── */
#include "stm32f4xx_hal.h"

/* ── Variable externa (handle UART definido en main.c) ─────────────────────── */
extern UART_HandleTypeDef huart2;

/* ── Configuración ─────────────────────────────────────────────────────────── */
#define UART_TX_TIMEOUT  HAL_MAX_DELAY

/* ═══════════════════════════════════════════════════════════════════════════ */
/*               Funciones exportadas a uart.c                                */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Envía un buffer de bytes por UART (modo bloqueante).
 * @param  buf  Puntero al buffer de datos.
 * @param  len  Cantidad de bytes a enviar.
 */
void port_uart_send_bytes(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart2, buf, len, UART_TX_TIMEOUT);
}
