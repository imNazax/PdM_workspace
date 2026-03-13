# Práctica 2 – Retardos no bloqueantes (Punto 1 y 2)
## Descripción
En esta práctica se implementa un mecanismo de retardos no bloqueantes utilizando temporización por software en un microcontrolador STM32.

El objetivo es implementar un módulo de software para trabajar con retardos no bloqueantes.

## Funcionamiento
El programa hace parpadear el LED de la placa de desarrollo con el siguiente comportamiento:

- 100 ms encendido
- 100 ms apagado

El parpadeo se realiza de forma periódica utilizando un temporizador no bloqueante implementado mediante una estructura `delay_t`.

## Implementación
Se implementan tres funciones auxiliares, que usan `HAL_GetTick()` como base de tiempo.:

- `delayInit()`: Inicializa la estructura de retardo
- `delayRead()`: Verifica si el tiempo del retardo se cumplió
- `delayWrite()`: Permite modificar la duración del retardo

## Archivos principales
- `Core/Src/main.c`
- `Core/Inc/main.h`

## Plataforma utilizada
- Microcontrolador STM32F446RE
- IDE: STM32CubeIDE
- Lenguaje: C

## Autor
Pablo Nazareno Coronati
