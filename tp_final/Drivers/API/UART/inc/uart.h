/**
 * *****************************************************************************
 * @file    uart.h
 * @brief   Interfaz pública del driver de comunicación UART.
 *
 * Expone funciones de alto nivel para enviar strings y datos
 * formateados por UART. No depende de ningún HAL ni hardware
 * específico — eso lo resuelve port.c internamente.
 *
 * Configuración esperada: 115200 baudios, 8N1 (configurado en CubeMX).
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

#ifndef UART_H
#define UART_H

/* ── Dependencias ──────────────────────────────────────────────────────────── */
#include <stdint.h>
#include <stdbool.h>

/* ── Definiciones ──────────────────────────────────────────────────────────── */

/** Tamaño máximo del buffer interno para formatear mensajes. */
#define UART_BUFFER_SIZE    128

/* ── Prototipos ────────────────────────────────────────────────────────────── */

/**
 * @brief  Inicializa el driver UART.
 *
 * Envía un mensaje de confirmación por la terminal serie.
 */
void UART_Init(void);

/**
 * @brief  Envía un string por UART terminado en '\0'.
 * @param  texto  Puntero al string a transmitir.
 */
void UART_SendString(char *texto);

/**
 * @brief  Formatea y envía temperatura y humedad por UART.
 * @param  temperatura  Valor en °C.
 * @param  humedad      Valor en %.
 *
 * Formato de salida: "T=23.5 H=60.2\r\n"
 */
void UART_SendData(float temperatura, float humedad);

#endif /* UART_H */
