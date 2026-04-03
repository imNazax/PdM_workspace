/**
 ******************************************************************************
 * @file    API_uart.h
 * @brief   Interfaz pública del módulo UART.
 ******************************************************************************
 */

#ifndef API_UART_H
#define API_UART_H

#include <stdint.h>
#include <stdbool.h>

typedef bool bool_t;

/**
 * @brief  Inicializa la UART e imprime los parámetros de configuración.
 * @retval true  si la inicialización fue exitosa
 * @retval false si hubo un error
 */
bool_t uartInit(void);

/**
 * @brief  Envía un string completo por UART hasta encontrar '\0'.
 * @param  pstring  Puntero al string a enviar
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief  Envía un string de longitud fija por UART.
 * @param  pstring  Puntero al string a enviar
 * @param  size     Cantidad de caracteres a enviar (1–256)
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief  Recibe un string de longitud fija por UART en modo polling.
 * @param  pstring  Puntero al buffer de recepción
 * @param  size     Cantidad de caracteres a recibir (1–256)
 */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);

#endif /* API_UART_H */
