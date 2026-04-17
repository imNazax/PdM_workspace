/**
 * *****************************************************************************
 * @file    lcd_i2c.c
 * @brief   Lógica genérica del driver del LCD 16x2 (HD44780 + PCF8574).
 *
 * Este archivo contiene la inicialización del HD44780 en modo 4 bits,
 * el envío de comandos/datos y las funciones de texto y cursor.
 * NO depende de ningún HAL — toda interacción con el bus I2C
 * se hace a través de las funciones de port.c.
 *
 * Si se migra a otro microcontrolador, este archivo NO se modifica.
 * Solo se reescribe port.c.
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

/* ── Includes ──────────────────────────────────────────────────────────────── */
#include "lcd_i2c.h"
#include <stddef.h>

/* ── Prototipos de port.c ──────────────────────────────────────────────────── */

/** Envía un byte al PCF8574 por I2C. */
extern void port_lcd_i2c_send(uint8_t data);

/** Retardo en milisegundos (wrapper de HAL_Delay). */
extern void port_lcd_delay_ms(uint32_t ms);

/* ── Definiciones internas del HD44780 ─────────────────────────────────────── */

/* Bits del byte enviado al PCF8574 */
#define LCD_BIT_RS          0x01    /* Register Select (0=cmd, 1=dato)  */
#define LCD_BIT_RW          0x02    /* Read/Write (siempre 0)           */
#define LCD_BIT_EN          0x04    /* Enable (pulso)                   */
#define LCD_BIT_BACKLIGHT   0x08    /* Backlight (1=encendido)          */

/* Comandos del HD44780 */
#define LCD_CMD_CLEAR        0x01
#define LCD_CMD_HOME         0x02
#define LCD_CMD_ENTRY_MODE   0x06
#define LCD_CMD_DISPLAY_ON   0x0C
#define LCD_CMD_FUNCTION_SET 0x28   /* 4 bits, 2 líneas, 5x8 */

/* ── Variables privadas ────────────────────────────────────────────────────── */
static uint8_t backlightState = LCD_BIT_BACKLIGHT;

/* ═══════════════════════════════════════════════════════════════════════════ */
/*                       Funciones privadas                                   */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Genera un pulso en el pin Enable del LCD.
 */
static void LCD_PulseEnable(uint8_t data)
{
    port_lcd_i2c_send(data | LCD_BIT_EN);     /* EN = 1 */
    port_lcd_delay_ms(1);
    port_lcd_i2c_send(data & ~LCD_BIT_EN);    /* EN = 0 */
    port_lcd_delay_ms(1);
}

/**
 * @brief  Envía un nibble (4 bits) al LCD.
 * @param  nibble  Valor con bits 4-7 posicionados.
 * @param  rs      0 para comando, LCD_BIT_RS para dato.
 */
static void LCD_SendNibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0) | rs | backlightState;
    LCD_PulseEnable(data);
}

/**
 * @brief  Envía un byte completo (dos nibbles) al LCD.
 */
static void LCD_SendByte(uint8_t byte, uint8_t rs)
{
    LCD_SendNibble(byte & 0xF0, rs);              /* nibble alto */
    LCD_SendNibble((byte << 4) & 0xF0, rs);       /* nibble bajo */
}

/**
 * @brief  Envía un comando al LCD (RS = 0).
 */
static void LCD_SendCommand(uint8_t cmd)
{
    LCD_SendByte(cmd, 0);
}

/**
 * @brief  Envía un dato (carácter) al LCD (RS = 1).
 */
static void LCD_SendDataByte(uint8_t data)
{
    LCD_SendByte(data, LCD_BIT_RS);
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/*                       Funciones públicas                                   */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Inicializa el LCD 16x2 en modo 4 bits vía I2C (PCF8574).
 */
void LCD_Init(void)
{
    /* Esperar ≥40 ms después del power-on (datasheet HD44780) */
    port_lcd_delay_ms(50);

    backlightState = LCD_BIT_BACKLIGHT;
    port_lcd_i2c_send(backlightState);
    port_lcd_delay_ms(100);

    /* Secuencia de inicialización para cambiar a modo 4 bits */
    LCD_SendNibble(0x30, 0);  port_lcd_delay_ms(5);    /* intento 1 */
    LCD_SendNibble(0x30, 0);  port_lcd_delay_ms(1);    /* intento 2 */
    LCD_SendNibble(0x30, 0);  port_lcd_delay_ms(1);    /* intento 3 */
    LCD_SendNibble(0x20, 0);  port_lcd_delay_ms(1);    /* modo 4 bits */

    /* Configuración: 4 bits, 2 líneas, fuente 5x8 */
    LCD_SendCommand(LCD_CMD_FUNCTION_SET);
    port_lcd_delay_ms(1);

    /* Display ON, cursor OFF, blink OFF */
    LCD_SendCommand(LCD_CMD_DISPLAY_ON);
    port_lcd_delay_ms(1);

    /* Limpiar pantalla */
    LCD_SendCommand(LCD_CMD_CLEAR);
    port_lcd_delay_ms(2);

    /* Entry Mode: incremento, sin shift */
    LCD_SendCommand(LCD_CMD_ENTRY_MODE);
    port_lcd_delay_ms(1);
}

/**
 * @brief  Posiciona el cursor.
 * @param  fila    0 o 1.
 * @param  columna 0 a 15.
 */
void LCD_SetCursor(uint8_t fila, uint8_t columna)
{
    uint8_t offsets[] = { 0x00, 0x40 };

    if (fila >= LCD_ROWS)    fila    = LCD_ROWS - 1;
    if (columna >= LCD_COLS) columna = LCD_COLS - 1;

    LCD_SendCommand(0x80 | (offsets[fila] + columna));
}

/**
 * @brief  Escribe un string en la posición actual del cursor.
 */
void LCD_Print(char *texto)
{
    if (texto == NULL)
        return;

    while (*texto != '\0')
    {
        LCD_SendDataByte((uint8_t)*texto);
        texto++;
    }
}

/**
 * @brief  Borra la pantalla y posiciona el cursor en (0, 0).
 */
void LCD_Clear(void)
{
    LCD_SendCommand(LCD_CMD_CLEAR);
    port_lcd_delay_ms(2);
}
