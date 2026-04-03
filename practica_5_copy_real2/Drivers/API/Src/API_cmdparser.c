/**
 ******************************************************************************
 * @file    API_cmdparser.c
 * @brief   Parser de comandos por UART usando MEF (modo polling).
 *
 * Implementa la MEF con 5 estados (CMD_IDLE, CMD_RECEIVING, CMD_PROCESS,
 * CMD_EXEC, CMD_ERROR). Procesa hasta 16 bytes por invocación y reconoce
 * comentarios que empiezan con '#' o con '//' (línea completa).
 *
 * Comandos soportados (case-insensitive):
 *  - HELP
 *  - LED ON / LED OFF / LED TOGGLE
 *  - STATUS
 *
 * Respuestas y errores terminan siempre con "\r\n".
 ******************************************************************************
 */

#include "API_cmdparser.h"
#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <ctype.h>

/* Dependencias externas */
extern UART_HandleTypeDef huart2;
extern volatile bool_t comandoUART_activo;

/* Buffer para la línea recibida */
static char lineBuffer[CMD_MAX_LINE];
static uint8_t bufferIdx = 0;

/* Buffer para procesamiento (proc) */
static char procBuffer[CMD_MAX_LINE];

/* Estados de la MEF (5 estados según consigna) */
typedef enum {
    CMD_IDLE,       /* Espera primer carácter no-terminador */
    CMD_RECEIVING,  /* Acumula caracteres en buffer */
    CMD_PROCESS,    /* Tokeniza y valida */
    CMD_EXEC,       /* Ejecuta la acción */
    CMD_ERROR       /* Imprime error y vuelve a IDLE (ignora hasta EOL) */
} cmd_state_t;

static cmd_state_t state = CMD_IDLE;

/* Estado/resultado del procesamiento */
static cmd_status_t procStatus = CMD_OK;

/* Prototipos privados */
static void cmdProcessLine(void);
static void toLowerString(char *str);
static void trimSpaces(char *str);
static void execute_command(void);

/* Inicializa el parser */
void cmdParserInit(void)
{
    state = CMD_IDLE;
    bufferIdx = 0;
    procStatus = CMD_OK;
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memset(procBuffer, 0, sizeof(procBuffer));
}

/*
 * cmdPoll: debe llamarse periódicamente desde el loop principal.
 * Procesa hasta 16 bytes por invocación (no bloqueante).
 */
void cmdPoll(void)
{
    uint8_t c;
    int i;

    for (i = 0; i < 16; i++)
    {
        /* Leer 1 byte sin bloquear (timeout = 0) */
        if (HAL_UART_Receive(&huart2, &c, 1, 0) != HAL_OK)
            break; /* no hay más datos ahora */

        switch (state)
        {
            /* ===== CMD_IDLE: esperar primer carácter no-terminador ===== */
            case CMD_IDLE:
                /* Ignorar terminadores y espacios iniciales */
                if (c == '\r' || c == '\n')
                    break;
                if (c == ' ' || c == '\t')
                    break;

                /* Comentario con '#' -> ignorar hasta EOL */
                if (c == '#')
                {
                    state = CMD_ERROR; /* usar ERROR para ignorar hasta EOL */
                    break;
                }

                /* Posible comentario '//' -> necesitamos ver siguiente byte.
                   Si no está disponible, tratamos '/' como inicio de línea. */
                if (c == '/')
                {
                    uint8_t next;
                    if (HAL_UART_Receive(&huart2, &next, 1, 0) == HAL_OK)
                    {
                        if (next == '/')
                        {
                            state = CMD_ERROR; /* ignorar hasta EOL */
                        }
                        else
                        {
                            /* No era '//' -> iniciar buffer con '/' y 'next' */
                            bufferIdx = 0;
                            memset(lineBuffer, 0, sizeof(lineBuffer));
                            if (bufferIdx < CMD_MAX_LINE - 1)
                                lineBuffer[bufferIdx++] = '/';
                            if (bufferIdx < CMD_MAX_LINE - 1)
                                lineBuffer[bufferIdx++] = (char)next;
                            state = CMD_RECEIVING;
                        }
                    }
                    else
                    {
                        /* Solo recibimos '/', lo tomamos como primer carácter */
                        bufferIdx = 0;
                        memset(lineBuffer, 0, sizeof(lineBuffer));
                        lineBuffer[bufferIdx++] = '/';
                        state = CMD_RECEIVING;
                    }
                    break;
                }

                /* Inicio normal de línea */
                bufferIdx = 0;
                memset(lineBuffer, 0, sizeof(lineBuffer));
                lineBuffer[bufferIdx++] = (char)c;
                state = CMD_RECEIVING;
                break;

            /* ===== CMD_RECEIVING: acumula caracteres ===== */
            case CMD_RECEIVING:
                /* Fin de línea -> pasar a PROCESS */
                if (c == '\r' || c == '\n')
                {
                    /* Asegurar terminador */
                    if (bufferIdx >= CMD_MAX_LINE)
                        bufferIdx = CMD_MAX_LINE - 1;
                    lineBuffer[bufferIdx] = '\0';
                    state = CMD_PROCESS;
                    /* Procesar la línea inmediatamente */
                    cmdProcessLine();
                    /* cmdProcessLine() dejará el estado en CMD_EXEC o CMD_ERROR o CMD_IDLE */
                    break;
                }

                /* Agregar carácter si hay espacio */
                if (bufferIdx < CMD_MAX_LINE - 1)
                {
                    lineBuffer[bufferIdx++] = (char)c;
                }
                else
                {
                    /* Overflow: informar y pasar a ERROR (ignorar hasta EOL) */
                    uartSendString((uint8_t *)"ERROR: line too long\r\n");
                    state = CMD_ERROR;
                }
                break;

            /* ===== CMD_PROCESS: no se espera recibir bytes aquí porque
               cmdProcessLine() hace la transición; por seguridad volvemos a IDLE ===== */
            case CMD_PROCESS:
                state = CMD_IDLE;
                break;

            /* ===== CMD_EXEC: ejecutar acción pendiente ===== */
            case CMD_EXEC:
                execute_command();
                state = CMD_IDLE;
                break;

            /* ===== CMD_ERROR: ignorar hasta fin de línea ===== */
            case CMD_ERROR:
                if (c == '\r' || c == '\n')
                {
                    state = CMD_IDLE;
                }
                break;

            default:
                state = CMD_IDLE;
                break;
        } /* switch */
    } /* for */
}

/* Imprime ayuda (respuestas terminan con \r\n) */
void cmdPrintHelp(void)
{
    uartSendString((uint8_t *)"\r\nComandos disponibles:\r\n");
    uartSendString((uint8_t *)"  HELP\r\n");
    uartSendString((uint8_t *)"  LED ON\r\n");
    uartSendString((uint8_t *)"  LED OFF\r\n");
    uartSendString((uint8_t *)"  LED TOGGLE\r\n");
    uartSendString((uint8_t *)"  STATUS\r\n\r\n");
}

/* ===== Funciones privadas ===== */

/*
 * cmdProcessLine:
 *  - copia la línea a procBuffer
 *  - normaliza (minúsculas)
 *  - trim de espacios
 *  - valida comando y argumentos
 *  - setea procStatus y cambia estado a CMD_EXEC o CMD_ERROR
 */
static void cmdProcessLine(void)
{
    /* Copiar con límite y asegurar terminador */
    strncpy(procBuffer, lineBuffer, CMD_MAX_LINE - 1);
    procBuffer[CMD_MAX_LINE - 1] = '\0';

    /* Normalizar */
    toLowerString(procBuffer);
    trimSpaces(procBuffer);

    /* Línea vacía -> volver a IDLE */
    if (procBuffer[0] == '\0')
    {
        state = CMD_IDLE;
        return;
    }

    /* ===== HELP ===== */
    if (strcmp(procBuffer, "help") == 0)
    {
        procStatus = CMD_OK;
        state = CMD_EXEC;
        return;
    }

    /* ===== STATUS ===== */
    if (strcmp(procBuffer, "status") == 0)
    {
        procStatus = CMD_OK;
        state = CMD_EXEC;
        return;
    }

    /* ===== LED ... ===== */
    if (strncmp(procBuffer, "led", 3) == 0)
    {
        char *arg = procBuffer + 3;
        trimSpaces(arg);

        if (strcmp(arg, "on") == 0 || strcmp(arg, "off") == 0 || strcmp(arg, "toggle") == 0)
        {
            procStatus = CMD_OK;
            state = CMD_EXEC;
            return;
        }
        else
        {
            procStatus = CMD_ERR_ARG;
            /* Enviar mensaje de error y volver a IDLE (consigna pide CMD_ERROR que imprime y vuelve a IDLE) */
            uartSendString((uint8_t *)"ERROR: bad arguments\r\n");
            state = CMD_ERROR;
            return;
        }
    }

    /* ===== Comando desconocido ===== */
    procStatus = CMD_ERR_UNKNOWN;
    uartSendString((uint8_t *)"ERROR: unknown command\r\n");
    state = CMD_ERROR;
}

/*
 * execute_command: ejecuta la acción ya validada en procBuffer.
 * Debe ser llamada cuando state == CMD_EXEC.
 */
static void execute_command(void)
{
    /* HELP */
    if (strcmp(procBuffer, "help") == 0)
    {
        cmdPrintHelp();
        return;
    }

    /* STATUS */
    if (strcmp(procBuffer, "status") == 0)
    {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)
            uartSendString((uint8_t *)"LED is ON\r\n");
        else
            uartSendString((uint8_t *)"LED is OFF\r\n");
        return;
    }

    /* LED ... */
        if (strncmp(procBuffer, "led", 3) == 0)
        {
            char *arg = procBuffer + 3;
            trimSpaces(arg);

            if (strcmp(arg, "on") == 0)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                comandoUART_activo = true;
                uartSendString((uint8_t *)"LED ON\r\n");
            }
            else if (strcmp(arg, "off") == 0)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                comandoUART_activo = true;
                uartSendString((uint8_t *)"LED OFF\r\n");
            }
            else if (strcmp(arg, "toggle") == 0)
            {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
                comandoUART_activo = true;
                uartSendString((uint8_t *)"LED TOGGLED\r\n");
            }
            return;
        }
    }

    /* Convierte string a minúsculas (seguro con unsigned char) */
    static void toLowerString(char *str)
    {
        while (*str)
        {
            *str = (char)tolower((unsigned char)*str);
            str++;
        }
    }

    /* Elimina espacios/tabs al inicio y final (in-place) */
    static void trimSpaces(char *str)
    {
        char *start = str;
        char *end;

        /* Avanzar inicio */
        while (*start == ' ' || *start == '\t')
            start++;

        if (*start == '\0')
        {
            str[0] = '\0';
            return;
        }

        /* Mover si hace falta */
        if (start != str)
            memmove(str, start, strlen(start) + 1);

        /* Recortar final */
        end = str + strlen(str) - 1;
        while (end > str && (*end == ' ' || *end == '\t'))
            end--;

        *(end + 1) = '\0';
    }
