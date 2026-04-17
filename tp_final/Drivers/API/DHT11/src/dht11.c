/**
 * *****************************************************************************
 * @file    dht11.c
 * @brief   Lógica genérica del driver del sensor DHT11.
 *
 * Protocolo de comunicación del DHT11
 * (señal de inicio, lectura de bits, verificación de checksum).
 * NO depende de ningún HAL — toda interacción con el hardware
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
#include "dht11.h"
#include <stddef.h>

/* ── Prototipos de port.c (funciones que tocan el hardware) ────────────────── */

/** Inicializa el GPIO del pin DATA y el DWT para delay_us. */
extern void    port_dht11_init(void);

/** Configura el pin DATA como salida open-drain. */
extern void    port_dht11_set_output(void);

/** Configura el pin DATA como entrada. */
extern void    port_dht11_set_input(void);

/** Escribe un valor en el pin DATA (0 = bajo, 1 = alto). */
extern void    port_dht11_pin_write(uint8_t val);

/** Lee el estado actual del pin DATA. Retorna 0 o 1. */
extern uint8_t port_dht11_pin_read(void);

/** Retardo en microsegundos (resolución µs vía DWT). */
extern void    port_dht11_delay_us(uint32_t us);

/** Retardo en milisegundos (vía HAL_Delay). */
extern void    port_dht11_delay_ms(uint32_t ms);

/* ── Variables privadas ────────────────────────────────────────────────────── */
static bool dht11Initialized = false;

/* ═══════════════════════════════════════════════════════════════════════════ */
/*                       Funciones privadas                                   */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Envía la señal de inicio al DHT11 y espera el ACK.
 * @retval true  si el sensor respondió.
 * @retval false si hubo timeout.
 */
static bool DHT11_StartSignal(void)
{
    uint16_t timeout;

    /* 1. MCU baja la línea DATA ≥18 ms */
    port_dht11_set_output();
    port_dht11_pin_write(0);
    port_dht11_delay_ms(20);

    /* 2. MCU libera la línea (pull-up externo la sube) */
    port_dht11_pin_write(1);
    port_dht11_delay_us(30);

    /* 3. Cambiar a entrada para leer la respuesta */
    port_dht11_set_input();

    /* 4. El DHT11 debe bajar la línea ~80 µs */
    timeout = 0;
    while (port_dht11_pin_read() == 1)
    {
        port_dht11_delay_us(1);
        if (++timeout > 100) return false;
    }

    /* 5. Esperar fin del pulso bajo (~80 µs) */
    timeout = 0;
    while (port_dht11_pin_read() == 0)
    {
        port_dht11_delay_us(1);
        if (++timeout > 100) return false;
    }

    /* 6. Esperar fin del pulso alto (~80 µs) */
    timeout = 0;
    while (port_dht11_pin_read() == 1)
    {
        port_dht11_delay_us(1);
        if (++timeout > 100) return false;
    }

    return true;
}

/**
 * @brief  Lee un byte (8 bits) del bus del DHT11.
 * @retval Byte leído.
 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t  byte = 0;
    uint16_t timeout;

    for (int i = 7; i >= 0; i--)
    {
        /* Esperar inicio del bit (línea en bajo ~50 µs) */
        timeout = 0;
        while (port_dht11_pin_read() == 0)
        {
            port_dht11_delay_us(1);
            if (++timeout > 100) return 0;
        }

        /* Medir duración del pulso alto */
        uint32_t highTime = 0;
        while (port_dht11_pin_read() == 1)
        {
            port_dht11_delay_us(1);
            highTime++;
            if (highTime > 100) return 0;
        }

        /* >40 µs en alto → bit '1' */
        if (highTime > 40)
        {
            byte |= (1 << i);
        }
    }

    return byte;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/*                       Funciones públicas                                   */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Inicializa el driver DHT11 (GPIO + DWT vía port.c).
 */
bool DHT11_Init(void)
{
    port_dht11_init();

    /* Línea en reposo = HIGH */
    port_dht11_set_output();
    port_dht11_pin_write(1);

    /* Esperar 1 s para que el sensor se estabilice (datasheet) */
    port_dht11_delay_ms(1000);

    dht11Initialized = true;
    return true;
}

/**
 * @brief  Lee temperatura y humedad del sensor DHT11.
 * @param  datos  Puntero a dht11_data_t para almacenar los valores.
 * @retval true   lectura exitosa, checksum OK.
 * @retval false  error de comunicación o checksum inválido.
 */
bool DHT11_Read(dht11_data_t *datos)
{
    if (datos == NULL || !dht11Initialized)
        return false;

    uint8_t rawData[5];

    /* Paso 1: señal de inicio + ACK */
    if (!DHT11_StartSignal())
        return false;

    /* Paso 2: leer 5 bytes (40 bits) */
    for (int i = 0; i < 5; i++)
    {
        rawData[i] = DHT11_ReadByte();
    }

    /* Paso 3: verificar checksum */
    uint8_t checksum = rawData[0] + rawData[1] + rawData[2] + rawData[3];
    if (checksum != rawData[4])
        return false;

    /* Paso 4: decodificar */
    datos->humedad     = (float)rawData[0] + (float)rawData[1] / 10.0f;
    datos->temperatura = (float)rawData[2] + (float)rawData[3] / 10.0f;

    return true;
}
