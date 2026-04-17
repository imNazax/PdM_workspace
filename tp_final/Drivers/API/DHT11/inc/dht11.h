/**
 * *****************************************************************************
 * @file    dht11.h
 * @brief   Interfaz pública del driver del sensor DHT11.
 *
 * Expone funciones de alto nivel para inicializar y leer el sensor.
 * No depende de ningún HAL ni hardware específico — eso lo resuelve
 * port.c internamente.
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

#ifndef DHT11_H
#define DHT11_H

/* ── Dependencias ──────────────────────────────────────────────────────────── */
#include <stdint.h>
#include <stdbool.h>

/* ── Tipos ─────────────────────────────────────────────────────────────────── */

/**
 * @brief  Datos leídos del sensor DHT11.
 */
typedef struct {
    float temperatura;  /**< Temperatura en °C  */
    float humedad;      /**< Humedad relativa %  */
} dht11_data_t;

/* ── Prototipos ────────────────────────────────────────────────────────────── */

/**
 * @brief  Inicializa el driver del sensor DHT11.
 * @retval true  si la inicialización fue exitosa.
 * @retval false si hubo un error.
 *
 * @note   Internamente configura el GPIO y el DWT a través de port.c.
 *         Espera 1 s para que el sensor se estabilice.
 */
bool DHT11_Init(void);

/**
 * @brief  Lee temperatura y humedad del sensor DHT11.
 * @param  datos  Puntero a estructura dht11_data_t donde se guardan los valores.
 * @retval true   si la lectura fue exitosa y el checksum es correcto.
 * @retval false  si hubo error de comunicación o checksum inválido.
 */
bool DHT11_Read(dht11_data_t *datos);

#endif /* DHT11_H */
