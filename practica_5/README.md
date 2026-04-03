# Práctica 5 – UART + Parser de comandos con MEF

## Descripción
Esta práctica implementa un módulo de comunicación UART en modo polling utilizando la HAL de STM32, junto con una Máquina de Estados Finitos (MEF) para el parseo de comandos recibidos por terminal serie.
El objetivo es permitir la interacción con el sistema mediante comandos de texto, procesados de forma no bloqueante.

## Punto 1 – Módulo UART
Se implementa una capa de abstracción para la UART en los archivos `API_uart.c` / `API_uart.h`, ubicados en `Drivers/API/Src` y `Drivers/API/Inc`.

### Funciones públicas
- `uartInit()`: Inicializa la UART y envía un mensaje por consola con la configuración.
- `uartSendString()`: Envía un string completo (hasta `\0`).
- `uartSendStringSize()`: Envía una cantidad específica de caracteres.
- `uartReceiveStringSize()`: Recibe una cantidad específica de caracteres.

### Consideraciones
- Validación de parámetros (punteros y tamaños).
- Verificación del estado de retorno de funciones HAL (`HAL_OK`, etc.).
- Uso de `static` para encapsulamiento interno.

## Punto 2 – Parser de comandos con MEF
Se implementa un módulo `API_cmdparser` que permite recibir y procesar comandos por UART mediante una MEF en modo polling.

### Archivos
- `API/inc/API_cmdparser.h`
- `API/src/API_cmdparser.c`

### Funcionamiento general
- Se reciben caracteres de a uno desde UART.
- Se almacenan en un buffer hasta detectar fin de línea (`\r`, `\n` o `\r\n`).
- Se ignoran comentarios (`#` o `//`).
- Se procesa la línea completa sin bloquear el programa principal.

### Estados de la MEF
La MEF controla la recepción y validación de comandos:

- `CMD_IDLE`: Espera inicio de recepción.
- `CMD_RECEIVING`: Acumulando caracteres.
- `CMD_PROCESS`: Línea completa recibida.
- `CMD_ERROR`: Error en la recepción (overflow u otro).
- `CMD_EXECUTE`: Ejecución del comando.

### Comandos implementados
Los comandos no distinguen entre mayúsculas y minúsculas:

- `HELP`: Muestra lista de comandos disponibles.
- `LED ON`: Enciende el LED.
- `LED OFF`: Apaga el LED.
- `LED TOGGLE`: Alterna el estado del LED.
- `STATUS`: Muestra el estado actual del LED.

### Manejo de errores
- Buffer lleno → `ERROR: line too long`
- Comando desconocido → `ERROR: unknown command`
- Argumentos inválidos → `ERROR: bad arguments`

## Comportamiento
El sistema permite interactuar mediante una terminal serie enviando comandos en texto plano.  
El procesamiento se realiza sin bloquear el loop principal, permitiendo mantener otras tareas en ejecución.

## Demo en video
[Ver demo](https://youtu.be/9KYeEW2zKLM)

## Archivos relevantes
- `Core/Src/main.c`
- `Core/Inc/main.h`
- `Drivers/API/Src/API_uart.c`
- `Drivers/API/Inc/API_uart.h`
- `Drivers/API/Src/API_cmdparser.c`
- `Drivers/API/Inc/API_cmdparser.h`

## Plataforma utilizada
- Microcontrolador: STM32F446RE
- IDE: STM32CubeIDE
- Lenguaje: C

## Autor
- Pablo Nazareno Coronati
