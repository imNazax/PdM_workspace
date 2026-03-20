/**
 ******************************************************************************
 * @file    API_delay.c
 * @brief   Implementación del módulo de retardos no bloqueantes.
 ******************************************************************************
 */

#include "API_delay.h"
#include "stm32f4xx_hal.h"   /* HAL_GetTick() */

/* ── Implementación de prototipos───────────────────────────────────────────────────────── */

/**
 * @brief  Inicializa la estructura de retardo.
 *
 * Carga la duración y deja el temporizador detenido.
 * El conteo arranca recién en la primera llamada a delayRead().
 *
 * @param  delay    Puntero a la estructura delay_t.
 * @param  duration Duración del retardo en milisegundos.
 */
void delayInit(delay_t *delay, tick_t duration)
{
    if (delay == NULL)
        return;

    delay->duration  = duration;
    delay->running   = false;
    delay->startTime = 0;
}

/**
 * @brief  Verifica si el retardo finalizó (temporizador no bloqueante).
 *
 * La primera vez que se llama inicia el conteo. Devuelve true una sola
 * vez por ciclo, cuando se cumple el tiempo configurado, y reinicia
 * automáticamente el temporizador para el ciclo siguiente.
 *
 * @param  delay  Puntero a la estructura delay_t.
 * @retval true   El tiempo de retardo finalizó.
 * @retval false  El tiempo aún no terminó.
 */
bool_t delayRead(delay_t *delay)
{
    if (delay == NULL)
        return false;

    if (!delay->running)
    {
        delay->startTime = HAL_GetTick();
        delay->running   = true;
    }

    if (HAL_GetTick() - delay->startTime >= delay->duration)
    {
        delay->running = false;
        return true;
    }

    return false;
}

/**
 * @brief  Actualiza la duración del retardo en caliente.
 *
 * No reinicia el conteo en curso ni toca el campo running.
 * El nuevo valor se aplica a partir del próximo ciclo.
 *
 * @param  delay    Puntero a la estructura delay_t.
 * @param  duration Nueva duración en milisegundos.
 */
void delayWrite(delay_t *delay, tick_t duration)
{
    if (delay == NULL)
        return;

    delay->duration = duration;
}

/**
 * @brief  Informa si el temporizador está contando actualmente.
 *
 * Devuelve una copia del campo running — no modifica la estructura.
 * Útil para verificar el estado antes de llamar a delayWrite.
 *
 * @param  delay  Puntero a la estructura delay_t.
 * @retval true   El temporizador está en curso.
 * @retval false  El temporizador está detenido (o delay es NULL).
 */
bool_t delayIsRunning(delay_t *delay)
{
    if (delay == NULL)
        return false;

    return delay->running;
}
