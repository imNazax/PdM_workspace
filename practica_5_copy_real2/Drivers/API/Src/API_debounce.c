#include "API_debounce.h"
#include "stm32f4xx_hal.h"

/* --- Configuración --- */
#define DEBOUNCE_TIME    ((tick_t)40)
#define BUTTON_PORT      GPIOC
#define BUTTON_PIN       GPIO_PIN_13
#define LED_PORT         GPIOA
#define LED_PIN          GPIO_PIN_5

/* --- Estados (PRIVADO) --- */
typedef enum {
    BUTTON_UP,
    BUTTON_FALLING,
    BUTTON_DOWN,
    BUTTON_RAISING
} debounceState_t;

/* --- Variables privadas --- */
static debounceState_t estado;
static delay_t delayDebounce;
static bool_t keyPressed = false;

/* --- Función privada --- */

/**
 * @brief  Lee el estado físico del pulsador (PC13, activo en bajo).
 * @param  Ninguno
 * @retval true  si el botón está presionado
 * @retval false si el botón está suelto
 */
static bool_t readButton(void)
{
    return HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET;
}

/* --- API pública --- */

/**
 * @brief  Inicializa la MEF anti-rebote.
 *         Coloca el estado inicial en BUTTON_UP.
 * @param  Ninguno
 * @retval Ninguno
 */
void debounceFSM_init(void)
{
    estado = BUTTON_UP;
    delayInit(&delayDebounce, DEBOUNCE_TIME);
}

/**
 * @brief  Actualiza la MEF anti-rebote. Debe llamarse periódicamente.
 *         Lee el pulsador, filtra rebotes y genera eventos ante
 *         flancos válidos.
 * @param  Ninguno
 * @retval Ninguno
 */
void debounceFSM_update(void)
{
    switch (estado)
    {
        case BUTTON_UP:  /* Botón estable y suelto. Si se detecta presión, inicia el debounce */
            if (readButton())
            {
                estado = BUTTON_FALLING;
                delayInit(&delayDebounce, DEBOUNCE_TIME);
            }
            break;

        case BUTTON_FALLING: /* Espera que expire el tiempo de debounce para confirmar la presión */
            if (delayRead(&delayDebounce))
            {
                if (readButton())
                {
                    estado = BUTTON_DOWN; /* Presión confirmada: genera el evento y avanza */
                    keyPressed = true;
                    buttonPressed();
                }
                else
                {
                    estado = BUTTON_UP; /* Era rebote: vuelve al estado estable */
                }
            }
            break;

        case BUTTON_DOWN:  /* Botón estable y presionado. Si se detecta liberación, inicia el debounce */
            if (!readButton())
            {
                estado = BUTTON_RAISING;
                delayInit(&delayDebounce, DEBOUNCE_TIME);
            }
            break;

        case BUTTON_RAISING:  /* Espera que expire el tiempo de debounce para confirmar la liberación */
            if (delayRead(&delayDebounce))
            {
                if (!readButton())
                {
                    estado = BUTTON_UP; /* Liberación confirmada */
                    buttonReleased();
                }
                else
                {
                    estado = BUTTON_DOWN;  /* Era rebote: vuelve al estado presionado */
                }
            }
            break;

        default:
            /* Estado inválido: reinicia la MEF de forma segura */
            debounceFSM_init();
            break;
    }
}

/**
 * @brief  Acción ante flanco descendente confirmado (botón presionado).
 *         En el Punto 2 no maneja el LED directamente — eso lo hace
 *         main.c mediante readKey(). Se provee para cumplir la interfaz
 *         definida en la consigna.
 * @param  Ninguno
 * @retval Ninguno
 */
void buttonPressed(void)
{
    /* Evento consumido por readKey() en main.c */
}

/**
 * @brief  Acción ante flanco ascendente confirmado (botón liberado).
 *         Ver nota en buttonPressed().
 * @param  Ninguno
 * @retval Ninguno
 */
void buttonReleased(void)
{
    /* Evento consumido por readKey() en main.c */
}

/**
 * @brief  Devuelve true si hubo una pulsación válida (flanco descendente).
 *         Resetea automáticamente el estado interno al ser llamada.
 * @param  Ninguno
 * @retval true  se detectó una presión válida
 * @retval false no hubo evento
 */
bool_t readKey(void)
{
    if (keyPressed)
    {
        keyPressed = false;
        return true;
    }
    return false;
}
