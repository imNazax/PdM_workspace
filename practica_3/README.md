# Práctica 3 – Modularización (APIs) del delay no bloqueante
## Descripción

Esta práctica toma las funciones de retardo no bloqueante desarrolladas en la práctica anterior (practica_2) y las encapsula en un módulo de software reutilizable.
El módulo separa la interfaz pública de la implementación en los archivos `API_delay.h` y `API_delay.c`, ubicados dentro de la carpeta `Drivers/API`. El programa principal utiliza dicho módulo para hacer parpadear el LED onboard siguiendo una secuencia de períodos predeterminada con duty cycle del 50%.

## Funcionamiento
El LED de la placa de desarrollo parpadea siguiendo el siguiente patrón. Luego de completar la secuencia, el patrón vuelve a comenzar:

- 1 vez con período de 500 ms (50% duty cycle).
- 2 veces con período de 100 ms (50% duty cycle).
- 1 vez con período de 1000 ms (50% duty cycle).

## Implementación
Los tiempos de parpadeo se definen mediante un arreglo `const` que contiene los valores de temporización utilizados para controlar el LED. La cantidad de tiempos se calcula automáticamente con `sizeof`, sin hardcodear ningún número.

El sistema utiliza la estructura `delay_t` para generar retardos no bloqueantes y las funciones:
- `delayInit()`
- `delayRead()`
- `delayWrite()`
- `delayIsRunning()`

## Archivos principales
- `Core/Src/main.c`
- `Core/Inc/main.h`
- `Drivers/API/Src/API_delay.c`
- `Drivers/API/Inc/API_delay.h`

## Plataforma utilizada
- Microcontrolador: STM32F446RE
- IDE: STM32CubeIDE
- Lenguaje: C

## Autor
Pablo Nazareno Coronati
