/**
 * *****************************************************************************
 * @file    API_delay.h
 * @brief   Interfaz pública del módulo de retardos no bloqueantes.
 *
 * Reutilizado de la Práctica 3 / Práctica 5 de PdM.
 ******************************************************************************
 */

#ifndef API_DELAY_H
#define API_DELAY_H

/* ── Dependencias ─────────────────────────────────────────────────────────── */
#include <stdint.h>   /* uint32_t  → requerido por tick_t  */
#include <stdbool.h>  /* bool      → requerido por bool_t  */

/* ── Tipos ────────────────────────────────────────────────────────────────── */
typedef uint32_t tick_t;
typedef bool     bool_t;

typedef struct {
    tick_t startTime;
    tick_t duration;
    bool_t running;
} delay_t;

/* ── Prototipos ───────────────────────────────────────────────────────────── */
/* Inicializa la estructura con la duración dada, temporizador detenido */
void   delayInit       ( delay_t *delay, tick_t duration );

/* Devuelve true cuando expira el tiempo; arranca el conteo en la primera llamada */
bool_t delayRead       ( delay_t *delay );

/* Actualiza la duración sin interrumpir el conteo en curso */
void   delayWrite      ( delay_t *delay, tick_t duration );

/* Devuelve true si el temporizador está contando actualmente */
bool_t delayIsRunning  ( delay_t *delay );

#endif /* API_DELAY_H */
