# Práctica 2 – Retardos no bloqueantes (con punto 3 opcional)
## Descripción
Esta versión de la práctica incluye la implementación del punto 3 opcional, extendiendo el programa desarrollado en los puntos 1 y 2.

Se utiliza un temporizador no bloqueante basado en HAL_GetTick() para controlar el parpadeo del LED sin detener la ejecución del programa.

## Funcionamiento
El LED de la placa de desarrollo, después de completar la secuencia, el patrón vuelve a comenzar. Parpadea siguiendo el siguiente patrón:

- 5 veces con período de 1 segundo (50% duty cycle).
- 5 veces con período de 200 ms (50% duty cycle).
- 5 veces con período de 100 ms (50% duty cycle).

## Implementación
Los tiempos de parpadeo se definen mediante un arreglo que contiene los valores de temporización utilizados para controlar el LED.

El sistema utiliza la estructura delay_t para generar retardos no bloqueantes y las funciones:

- `delayInit()`
- `delayRead()`
- `delayWrite()`

## Archivos principales
- `Core/Src/main.c`
- `Core/Inc/main.h`

## Plataforma utilizada
- Microcontrolador STM32F446RE
- IDE: STM32CubeIDE
- Lenguaje: C

## Autor
Pablo Nazareno Coronati
