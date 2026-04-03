/**
 ******************************************************************************
 * @file    API_cmdparser.h
 * @brief   Interfaz pública del parser de comandos por UART.
 *
 * Implementa un parser basado en MEF (5 estados) que opera en modo polling.
 * Es no bloqueante, case-insensitive y tolerante a espacios.
 *
 * El módulo es independiente del hardware: interactúa con el LED a través
 * de dos callbacks registrados en cmdParserInit().
 *
 * Comandos soportados:
 *  - HELP
 *  - LED ON / LED OFF / LED TOGGLE
 *  - STATUS
 ******************************************************************************
 */

#ifndef API_CMDPARSER_H
#define API_CMDPARSER_H

#include <stdint.h>
#include <stdbool.h>

/* ── Definiciones ──────────────────────────────────────────────────────────── */

/** Tamaño máximo de línea, incluyendo '\0' */
#define CMD_MAX_LINE    64

/** Máxima cantidad de tokens (comando + argumentos) */
#define CMD_MAX_TOKENS   3

/* ── Tipos ─────────────────────────────────────────────────────────────────── */

/** Códigos de estado del parser */
typedef enum {
    CMD_OK = 0,       /**< Operación exitosa        */
    CMD_ERR_OVERFLOW, /**< Línea demasiado larga     */
    CMD_ERR_SYNTAX,   /**< Error de sintaxis         */
    CMD_ERR_UNKNOWN,  /**< Comando desconocido       */
    CMD_ERR_ARG       /**< Argumentos inválidos      */
} cmd_status_t;

/** Acciones posibles sobre el LED */
typedef enum {
    LED_ACTION_ON,
    LED_ACTION_OFF,
    LED_ACTION_TOGGLE
} led_action_t;

/**
 * @brief  Callback para ejecutar una acción sobre el LED.
 * @param  action  Acción solicitada (ON / OFF / TOGGLE)
 */
typedef void (*led_callback_t)(led_action_t action);

/**
 * @brief  Callback para leer el estado actual del LED.
 * @retval true  si el LED está encendido
 * @retval false si está apagado
 */
typedef bool (*led_read_cb_t)(void);

/* ── API pública ───────────────────────────────────────────────────────────── */

/**
 * @brief  Inicializa el parser de comandos.
 *
 * @param  setCb   Callback invocado al recibir LED ON/OFF/TOGGLE. Puede ser NULL.
 * @param  readCb  Callback invocado al recibir STATUS. Puede ser NULL.
 */
void cmdParserInit(led_callback_t setCb, led_read_cb_t readCb);

/**
 * @brief  Ejecuta la MEF del parser.
 *
 * Debe llamarse periódicamente desde el loop principal.
 * Procesa hasta 16 bytes por invocación sin bloquear.
 */
void cmdPoll(void);

/**
 * @brief  Imprime la lista de comandos disponibles por UART.
 */
void cmdPrintHelp(void);

#endif /* API_CMDPARSER_H */
