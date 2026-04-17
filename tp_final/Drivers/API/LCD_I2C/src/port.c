/**
 * *****************************************************************************
 * @file    port.c  (LCD_I2C)
 * @brief   Funciones dependientes del hardware para el driver LCD I2C.
 *
 * Este archivo es el ÚNICO que "sabe" que existe un STM32 y su HAL I2C.
 * Si se migra a otro micro, solo se reescribe este archivo.
 *
 * Conexiones:
 *   SDA → PB9 (I2C1_SDA)
 *   SCL → PB8 (I2C1_SCL)
 *   Dirección PCF8574: 0x27
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

/* ── Includes ──────────────────────────────────────────────────────────────── */
#include "stm32f4xx_hal.h"

/* ── Variable externa (handle I2C definido en main.c) ──────────────────────── */
extern I2C_HandleTypeDef hi2c1;

/* ── Configuración ─────────────────────────────────────────────────────────── */

/**
 * @brief Dirección I2C del PCF8574.
 * La dirección de 7 bits es 0x27.
 * El HAL espera la dirección desplazada: 0x27 << 1 = 0x4E.
 */
#define LCD_I2C_ADDR    (0x27 << 1)

/** Timeout de transmisión I2C en ms. */
#define LCD_I2C_TIMEOUT 100

/* ═══════════════════════════════════════════════════════════════════════════ */
/*               Funciones exportadas a lcd_i2c.c                             */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Envía un byte al PCF8574 por I2C.
 * @param  data  Byte a enviar (contiene RS, EN, BL, D4-D7).
 */
void port_lcd_i2c_send(uint8_t data)
{
    HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &data, 1, LCD_I2C_TIMEOUT);
}

/**
 * @brief  Retardo en milisegundos.
 * @param  ms  Cantidad de milisegundos.
 */
void port_lcd_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
