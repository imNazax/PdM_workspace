/**
 ******************************************************************************
 * @file    API_cmdparser.c
 * @brief   Parser de comandos por UART mediante MEF (modo polling).
 *
 * MEF con 5 estados:
 *   CMD_IDLE       → Espera el primer carácter no-terminador
 *   CMD_RECEIVING  → Acumula caracteres hasta recibir '\r' o '\n'
 *   CMD_PROCESS    → Tokeniza y valida el comando (transitorio)
 *   CMD_EXEC       → Ejecuta la acción validada   (transitorio)
 *   CMD_ERROR      → Descarta bytes hasta fin de línea
 *
 * Consideraciones:
 *  - Accede al LED exclusivamente a través de los callbacks registrados.
 *  - '#' en CMD_IDLE activa un flag isComment; '//' se filtra en cmdProcessLine.
 *    Ambos casos evitan lecturas adicionales dentro del switch.
 *  - cmdProcessLine() valida; executeCommand() actúa. Sin duplicación de lógica.
 ******************************************************************************
 */

#include "API_cmdparser.h"
#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <ctype.h>

/* ── Variable externa ─────────────────────────────────────────────────────── */
extern UART_HandleTypeDef huart2;

/* ── Variables privadas ───────────────────────────────────────────────────── */

static led_callback_t ledSetCallback  = NULL;
static led_read_cb_t  ledReadCallback = NULL;

static char    lineBuffer[CMD_MAX_LINE];
static uint8_t bufferIdx = 0;

static char    procBuffer[CMD_MAX_LINE];

/** true mientras se recibe una línea marcada como comentario */
static bool    isComment = false;

typedef enum {
    CMD_IDLE,       /** Espera primer carácter útil                   */
    CMD_RECEIVING,  /** Acumula caracteres hasta terminador            */
    CMD_PROCESS,    /** Tokeniza y valida (transitorio, sin polling)   */
    CMD_EXEC,       /** Ejecuta la acción  (transitorio, sin polling)  */
    CMD_ERROR       /** Descarta hasta fin de línea                    */
} cmd_state_t;

static cmd_state_t state = CMD_IDLE;

/* ── Prototipos privados ──────────────────────────────────────────────────── */
static void cmdProcessLine(void);
static void executeCommand(void);
static void toLowerString(char *str);
static void trimSpaces(char *str);
static bool isTerminator(uint8_t c);

/* ── Implementación pública ───────────────────────────────────────────────── */

void cmdParserInit(led_callback_t setCb, led_read_cb_t readCb)
{
    ledSetCallback  = setCb;
    ledReadCallback = readCb;
    state           = CMD_IDLE;
    bufferIdx       = 0;
    isComment       = false;
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memset(procBuffer, 0, sizeof(procBuffer));
}

void cmdPoll(void)
{
    uint8_t c;

    for (int i = 0; i < 16; i++)
    {
        if (HAL_UART_Receive(&huart2, &c, 1, 0) != HAL_OK)
            break;

        switch (state)
        {
            /* ── CMD_IDLE ──────────────────────────────────────────────────
             * Descarta terminadores y espacios iniciales.
             * '#' activa el flag isComment y pasa a CMD_RECEIVING para
             * descartar el resto de la línea sin un mensaje de error.
             * '//' no se puede detectar aquí con un solo carácter leído:
             * se filtra más adelante en cmdProcessLine().
             * ─────────────────────────────────────────────────────────── */
            case CMD_IDLE:
                if (isTerminator(c) || c == ' ' || c == '\t')
                    break;

                bufferIdx = 0;
                isComment = false;
                memset(lineBuffer, 0, sizeof(lineBuffer));

                if (c == '#')
                {
                    isComment = true;
                    state = CMD_RECEIVING;
                    break;
                }

                lineBuffer[bufferIdx++] = (char)c;
                state = CMD_RECEIVING;
                break;

            /* ── CMD_RECEIVING ─────────────────────────────────────────────
             * Acumula caracteres.
             * Al recibir terminador: descarta si era comentario,
             * o pasa a CMD_PROCESS (transitorio) en caso contrario.
             * Overflow → CMD_ERROR.
             * ─────────────────────────────────────────────────────────── */
            case CMD_RECEIVING:
                if (isTerminator(c))
                {
                    lineBuffer[bufferIdx] = '\0';

                    if (isComment)
                    {
                        state = CMD_IDLE;
                        break;
                    }

                    state = CMD_PROCESS;
                    cmdProcessLine();   /* deja state en CMD_IDLE al retornar */
                    break;
                }

                if (bufferIdx < CMD_MAX_LINE - 1)
                {
                    lineBuffer[bufferIdx++] = (char)c;
                }
                else
                {
                    uartSendString((uint8_t *)"ERROR: line too long\r\n");
                    state = CMD_ERROR;
                }
                break;

            /* ── CMD_ERROR ─────────────────────────────────────────────────
             * Descarta todo hasta recibir un terminador.
             * ─────────────────────────────────────────────────────────── */
            case CMD_ERROR:
                if (isTerminator(c))
                    state = CMD_IDLE;
                break;

            /* CMD_PROCESS y CMD_EXEC son transitorios: nunca se llega aquí */
            default:
                state = CMD_IDLE;
                break;
        }
    }
}

void cmdPrintHelp(void)
{
    uartSendString((uint8_t *)
        "\r\nComandos disponibles:\r\n"
        "  HELP\r\n"
        "  LED ON\r\n"
        "  LED OFF\r\n"
        "  LED TOGGLE\r\n"
        "  STATUS\r\n\r\n");
}

/* ── Implementación privada ───────────────────────────────────────────────── */

/**
 * @brief  Normaliza la línea, filtra comentarios '//' y valida el comando.
 *         Si válido -> CMD_EXEC + executeCommand(); si no → error por UART.
 *         Siempre termina con state = CMD_IDLE.
 */
static void cmdProcessLine(void)
{
    strncpy(procBuffer, lineBuffer, CMD_MAX_LINE - 1);
    procBuffer[CMD_MAX_LINE - 1] = '\0';

    toLowerString(procBuffer);
    trimSpaces(procBuffer);

    /* Línea vacía: ignorar */
    if (procBuffer[0] == '\0')
    {
        state = CMD_IDLE;
        return;
    }

    /* Comentario '//': ignorar silenciosamente */
    if (strncmp(procBuffer, "//", 2) == 0)
    {
        state = CMD_IDLE;
        return;
    }

    /* Comandos sin argumentos */
    if (strcmp(procBuffer, "help") == 0 ||
        strcmp(procBuffer, "status") == 0)
    {
        state = CMD_EXEC;
        executeCommand();
        state = CMD_IDLE;
        return;
    }

    /* Comando LED: verificar argumento */
    if (strncmp(procBuffer, "led", 3) == 0)
    {
        const char *arg = procBuffer + 3;
        while (*arg == ' ' || *arg == '\t')
            arg++;

        if (strcmp(arg, "on")     == 0 ||
            strcmp(arg, "off")    == 0 ||
            strcmp(arg, "toggle") == 0)
        {
            state = CMD_EXEC;
            executeCommand();
            state = CMD_IDLE;
            return;
        }

        uartSendString((uint8_t *)"ERROR: bad arguments\r\n");
        state = CMD_IDLE;
        return;
    }

    uartSendString((uint8_t *)"ERROR: unknown command\r\n");
    state = CMD_IDLE;
}

/**
 * @brief  Ejecuta la acción del comando ya validado en procBuffer.
 *         Usa los callbacks para toda interacción con el LED.
 */
static void executeCommand(void)
{
    if (strcmp(procBuffer, "help") == 0)
    {
        cmdPrintHelp();
        return;
    }

    if (strcmp(procBuffer, "status") == 0)
    {
        if (ledReadCallback != NULL)
        {
            uartSendString(ledReadCallback()
                ? (uint8_t *)"LED is ON\r\n"
                : (uint8_t *)"LED is OFF\r\n");
        }
        else
        {
            uartSendString((uint8_t *)"STATUS: no disponible\r\n");
        }
        return;
    }

    if (strncmp(procBuffer, "led", 3) == 0)
    {
        const char *arg = procBuffer + 3;
        while (*arg == ' ' || *arg == '\t')
            arg++;

        led_action_t action;
        const char  *response;

        if (strcmp(arg, "on") == 0)
        {
            action   = LED_ACTION_ON;
            response = "LED ON\r\n";
        }
        else if (strcmp(arg, "off") == 0)
        {
            action   = LED_ACTION_OFF;
            response = "LED OFF\r\n";
        }
        else    /* "toggle": ya validado en cmdProcessLine */
        {
            action   = LED_ACTION_TOGGLE;
            response = "LED TOGGLED\r\n";
        }

        if (ledSetCallback != NULL)
            ledSetCallback(action);

        uartSendString((uint8_t *)response);
    }
}

static bool isTerminator(uint8_t c)
{
    return (c == '\r' || c == '\n');
}

static void toLowerString(char *str)
{
    for (; *str; str++)
        *str = (char)tolower((unsigned char)*str);
}

static void trimSpaces(char *str)
{
    char *start = str;
    while (*start == ' ' || *start == '\t')
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);

    if (str[0] == '\0')
        return;

    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t'))
        end--;

    *(end + 1) = '\0';
}
