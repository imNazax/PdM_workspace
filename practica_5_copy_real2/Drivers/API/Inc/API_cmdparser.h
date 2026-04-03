/**
 ******************************************************************************
 * @file    API_cmdparser.h
 * @brief   Interfaz pública del parser de comandos por UART.
 *
 * Este módulo implementa un parser de comandos basado en una Máquina
 * de Estados Finita (MEF) que opera en modo polling.
 *
 * Permite:
 *  - Recibir caracteres desde UART
 *  - Construir líneas completas
 *  - Procesar comandos simples
 *
 * El parser es:
 *  - No bloqueante
 *  - Case-insensitive
 *  - Tolerante a espacios
 *
 * Comandos soportados:
 *  - HELP
 *  - LED ON / OFF / TOGGLE
 *  - STATUS
 ******************************************************************************
 */

#ifndef API_CMDPARSER_H
#define API_CMDPARSER_H

/* ── Inclusiones ───────────────────────────────────────────────────────── */
#include <stdint.h>
#include <stdbool.h>

/* ── Definiciones ──────────────────────────────────────────────────────── */

/**
 * @brief Tamaño máximo de línea (incluye terminador '\0')
 */
#define CMD_MAX_LINE      64

/**
 * @brief Máxima cantidad de tokens esperados
 *        (comando + argumentos)
 */
#define CMD_MAX_TOKENS     3

/* ── Tipos de estado de retorno ────────────────────────────────────────── */

/**
 * @brief Códigos de estado del parser
 *
 * Nota: actualmente no todos son utilizados explícitamente,
 * pero se definen para extensibilidad futura.
 */
typedef enum {
    CMD_OK = 0,          /**< Operación exitosa */
    CMD_ERR_OVERFLOW,    /**< Línea demasiado larga */
    CMD_ERR_SYNTAX,      /**< Error de sintaxis */
    CMD_ERR_UNKNOWN,     /**< Comando desconocido */
    CMD_ERR_ARG          /**< Argumentos inválidos */
} cmd_status_t;

/* ── API pública ───────────────────────────────────────────────────────── */

/**
 * @brief Inicializa el parser de comandos.
 *
 * Debe llamarse una única vez antes de usar el módulo.
 */
void cmdParserInit(void);

/**
 * @brief Ejecuta la MEF del parser.
 *
 * Debe llamarse periódicamente dentro del loop principal.
 * No bloquea la ejecución.
 */
void cmdPoll(void);

/**
 * @brief Imprime la lista de comandos disponibles.
 *
 * Envía la ayuda por UART.
 */
void cmdPrintHelp(void);

#endif /* API_CMDPARSER_H */
