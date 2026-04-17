/**
 * *****************************************************************************
 * @file    lcd_i2c.h
 * @brief   Interfaz pública del driver del LCD 16x2 con módulo I2C (PCF8574).
 *
 * Expone funciones de alto nivel para inicializar y controlar el LCD.
 * No depende de ningún HAL ni hardware específico — eso lo resuelve
 * port.c internamente.
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

#ifndef LCD_I2C_H
#define LCD_I2C_H

/* ── Dependencias ──────────────────────────────────────────────────────────── */
#include <stdint.h>
#include <stdbool.h>

/* ── Definiciones ──────────────────────────────────────────────────────────── */

/** Número de columnas del display LCD. */
#define LCD_COLS    16

/** Número de filas del display LCD. */
#define LCD_ROWS    2

/* ── Prototipos ────────────────────────────────────────────────────────────── */

/**
 * @brief  Inicializa el display LCD 16x2.
 *
 * Ejecuta la secuencia de inicialización del HD44780 en modo 4 bits
 * a través del PCF8574 por I2C. Limpia la pantalla y enciende el backlight.
 *
 * @note   Debe llamarse una sola vez al inicio, después de inicializar el I2C.
 */
void LCD_Init(void);

/**
 * @brief  Posiciona el cursor en la fila y columna indicadas.
 * @param  fila    0 (primera) o 1 (segunda).
 * @param  columna 0 a 15.
 */
void LCD_SetCursor(uint8_t fila, uint8_t columna);

/**
 * @brief  Escribe un string en la posición actual del cursor.
 * @param  texto   Puntero al string terminado en '\0'.
 */
void LCD_Print(char *texto);

/**
 * @brief  Borra todo el contenido del display y cursor a (0, 0).
 */
void LCD_Clear(void);

#endif /* LCD_I2C_H */
