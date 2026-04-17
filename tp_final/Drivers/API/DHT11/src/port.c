/**
 * *****************************************************************************
 * @file    port.c  (DHT11)
 * @brief   Funciones dependientes del hardware para el driver DHT11.
 *
 * Este archivo es el ÚNICO que "sabe" que existe un STM32 y su HAL.
 * Si se migra a otro micro, solo se reescribe este archivo.
 *
 * Pin DATA del DHT11 -> PA0 (con pull-up externo de 10kΩ a 3.3V)
 *
 * Plataforma: NUCLEO-STM32F446RE
 * Proyecto:   PdM + PCSE — CESE FIUBA
 ******************************************************************************
 */

/* ── Includes ──────────────────────────────────────────────────────────────── */
#include "stm32f4xx_hal.h"

/* ── Configuración del pin ─────────────────────────────────────────────────── */
#define DHT11_PORT   GPIOA
#define DHT11_PIN    GPIO_PIN_0

/* ═══════════════════════════════════════════════════════════════════════════ */
/*               Funciones exportadas a dht11.c                               */
/* ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Inicializa el GPIO del pin DATA y habilita el DWT para delay_us.
 */
void port_dht11_init(void)
{
    /* Habilitar clock del puerto */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Habilitar DWT (Data Watchpoint and Trace) para delay_us */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief  Configura el pin DATA como salida open-drain.
 */
void port_dht11_set_output(void)
{
    GPIO_InitTypeDef cfg = {0};
    cfg.Pin   = DHT11_PIN;
    cfg.Mode  = GPIO_MODE_OUTPUT_OD;
    cfg.Pull  = GPIO_NOPULL;        /* Pull-up externo 10kΩ */
    cfg.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &cfg);
}

/**
 * @brief  Configura el pin DATA como entrada.
 */
void port_dht11_set_input(void)
{
    GPIO_InitTypeDef cfg = {0};
    cfg.Pin  = DHT11_PIN;
    cfg.Mode = GPIO_MODE_INPUT;
    cfg.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &cfg);
}

/**
 * @brief  Escribe un valor en el pin DATA.
 * @param  val  0 = bajo, distinto de 0 = alto.
 */
void port_dht11_pin_write(uint8_t val)
{
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN,
                      val ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief  Lee el estado del pin DATA.
 * @retval 0 si está en bajo, 1 si está en alto.
 */
uint8_t port_dht11_pin_read(void)
{
    return (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

/**
 * @brief  Retardo en microsegundos usando el contador de ciclos DWT.
 * @param  us  Cantidad de microsegundos.
 */
void port_dht11_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);

    while ((DWT->CYCCNT - start) < ticks)
    {
        /* Espera activa */
    }
}

/**
 * @brief  Retardo en milisegundos (wrapper de HAL_Delay).
 * @param  ms  Cantidad de milisegundos.
 */
void port_dht11_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
