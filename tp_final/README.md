# Trabajo Práctico Final – Estación de Monitoreo Ambiental
## Descripción
Sistema de monitoreo ambiental que lee temperatura y humedad desde un sensor DHT11 cada 2 segundos, muestra los valores en un display LCD 16x2 conectado vía I2C (PCF8574) y los reporta simultáneamente por UART a una terminal serie en la PC.
La lógica de la aplicación está controlada por una MEF de 4 estados implementada en `main.c`, utilizando delays no bloqueantes.
## Arquitectura driver / port
Cada driver se divide en dos archivos según lo requerido por la cátedra de PCSE:
- **driver.c**: Lógica genérica y reutilizable. No depende del HAL de STM32.
- **port.c**: Funciones cortas que llaman directamente al HAL. Único archivo que se reescribe al migrar de plataforma.
## Periféricos utilizados
| Periférico | Protocolo | Pines | Configuración |
|:---|:---|:---|:---|
| **DHT11** (Sensor) | GPIO + 1-Wire propietario | PA0 (Open Drain) | Pull-up externo 10kΩ, DWT para delays µs |
| **LCD 16x2** (Display) | I2C1 (Standard Mode) | PB8 (SCL), PB9 (SDA) | 100 kHz, PCF8574 dirección 0x27 |
| **USART2** (Terminal PC) | UART asíncrona | PA2 (TX), PA3 (RX) | 115200 baud, 8N1, vía ST-LINK USB |
## Máquina de Estados Finita
```
        ┌───────────────────────────────────────┐
        │                                       │
        ▼                                       │
  ┌──────────┐                                  │
  │   IDLE   │                                  │
  │(Esperando)│                                  │
  └────┬─────┘                                  │
       │ delayRead() == true                    │
       ▼                                        │
  ┌──────────┐       Error                      │
  │   LEER   │─────────────────────────────────►│
  │ (DHT11)  │                                  │
  └────┬─────┘                                  │
       │ Lectura OK                             │
       ▼                                        │
  ┌──────────┐                                  │
  │ MOSTRAR  │                                  │
  │  (LCD)   │                                  │
  └────┬─────┘                                  │
       │ inmediato                              │
       ▼                                        │
  ┌──────────┐                                  │
  │ REPORTAR │──────────────────────────────────┘
  │  (UART)  │  Envía datos + reinicia delay
  └──────────┘
```
| Estado | Acción | Transición |
|:---|:---|:---|
| **IDLE** | Espera que expire el delay no bloqueante de 2 s | → LEER cuando `delayRead()` devuelve `true` |
| **LEER** | Invoca `DHT11_Read()` para obtener temperatura y humedad | → MOSTRAR si OK / → IDLE si falla |
| **MOSTRAR** | Actualiza LCD con `LCD_SetCursor()` + `LCD_Print()` | → REPORTAR (inmediato) |
| **REPORTAR** | Envía datos formateados por UART (`T=24.5 H=55.0\r\n`) | → IDLE (reinicia delay) |
## Módulos de software
### API_delay (`Drivers/API/API_delay/`)
Módulo de delays no bloqueantes reutilizado de la Práctica 5. Funciones principales:
- `delayInit()`: Inicializa estructura con duración dada.
- `delayRead()`: Devuelve `true` cuando expira el tiempo configurado.
- `delayWrite()`: Actualiza duración sin interrumpir conteo.
- `delayIsRunning()`: Informa si el temporizador está activo.
### DHT11 (`Drivers/API/DHT11/`)
Driver del sensor de temperatura y humedad.
- `dht11.c`: Protocolo 1-Wire (señal de inicio, lectura de 40 bits, verificación de checksum). **No incluye HAL.**
- `port.c`: GPIO (PA0, Open Drain) + delays en microsegundos vía DWT. **Único archivo que toca HAL.**
### LCD_I2C (`Drivers/API/LCD_I2C/`)
Driver del display LCD 16x2 con módulo I2C (PCF8574 → HD44780).
- `lcd_i2c.c`: Inicialización modo 4 bits, envío de comandos/datos, control de cursor y backlight. **No incluye HAL.**
- `port.c`: `HAL_I2C_Master_Transmit()` al PCF8574. **Único archivo que toca HAL.**
### UART (`Drivers/API/UART/`)
Driver de comunicación serie para reporte a PC.
- `uart.c`: Formateo de mensajes con `snprintf()`. **No incluye HAL.**
- `port.c`: `HAL_UART_Transmit()` sobre USART2. **Único archivo que toca HAL.**
## Estructura del proyecto
```
tp_final/
├── Core/
│   ├── Inc/
│   │   └── main.h                 ← Tipos MEF, includes, defines
│   └── Src/
│       ├── main.c                 ← MEF + while(1)
│       ├── stm32f4xx_hal_msp.c    ← MSP init (generado por CubeMX)
│       └── stm32f4xx_it.c         ← Interrupciones (SysTick)
│
└── Drivers/
    └── API/
        ├── API_delay/
        │   ├── inc/API_delay.h
        │   └── src/API_delay.c
        ├── DHT11/
        │   ├── inc/dht11.h
        │   └── src/
        │       ├── dht11.c        ← Lógica genérica (sin HAL)
        │       └── port.c         ← Hardware específico (con HAL)
        ├── LCD_I2C/
        │   ├── inc/lcd_i2c.h
        │   └── src/
        │       ├── lcd_i2c.c      ← Lógica genérica (sin HAL)
        │       └── port.c         ← Hardware específico (con HAL)
        └── UART/
            ├── inc/uart.h
            └── src/
                ├── uart.c         ← Lógica genérica (sin HAL)
                └── port.c         ← Hardware específico (con HAL)
```
## Buenas prácticas aplicadas
- **Delays no bloqueantes** en vez de `HAL_Delay()` para mantener el micro responsivo.
- **`snprintf`** en vez de `sprintf` para prevenir buffer overflow.
- **Timeouts** en todos los `while` del protocolo DHT11 (Power of 10, Regla 2).
- **Validación de punteros NULL** al inicio de funciones públicas.
- **Variables privadas con `static`** para encapsulamiento de módulos.
- **Sin `malloc`/`free`**: toda la memoria es estática o en stack.
- **Sin recursión ni `goto`**.
## Demo en video
[![Demo del proyecto](https://img.youtube.com/vi/untw_pYlT7I/0.jpg)](https://www.youtube.com/shorts/untw_pYlT7I)
## Plataforma utilizada
- **Microcontrolador:** STM32F446RE (NUCLEO-F446RE)
- **IDE:** STM32CubeIDE
- **Lenguaje:** C
- **Terminal serie:** Picocom (115200 baud)
## Autor
Pablo Nazareno Coronati — CESE FIUBA
