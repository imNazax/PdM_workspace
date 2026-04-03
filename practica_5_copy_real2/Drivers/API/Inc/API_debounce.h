#ifndef API_DEBOUNCE_H
#define API_DEBOUNCE_H

#include "API_delay.h"

/* Inicializa la MEF */
void debounceFSM_init(void);

/* Actualiza la MEF (debe llamarse periódicamente) */
void debounceFSM_update(void);

/**
 * @brief  Acción ante flanco descendente confirmado (botón presionado).
 *         Llamada internamente por debounceFSM_update().
 * @param  Ninguno
 * @retval Ninguno
 */
void buttonPressed(void);

/**
 * @brief  Acción ante flanco ascendente confirmado (botón liberado).
 *         Llamada internamente por debounceFSM_update().
 * @param  Ninguno
 * @retval Ninguno
 */
void buttonReleased(void);

/**
 * @brief Devuelve true si hubo un evento de tecla presionada
 *        (flanco descendente).
 *        Resetea automáticamente el estado interno.
 */
bool_t readKey(void);

#endif
