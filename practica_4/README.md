# Práctica 4 – MEF anti-rebote por software

## Descripción
Esta práctica implementa una Máquina de Estados Finitos (MEF) para el manejo de anti-rebotes por software sobre el pulsador de la placa NUCLEO-F446RE. El objetivo es detectar de forma confiable los flancos ascendente y descendente del botón, filtrando los rebotes mecánicos mediante un retardo no bloqueante de 40 ms.

## Punto 1
La MEF completa se implementa directamente en `main.c`, sin módulos externos. Ante un flanco descendente confirmado se llama a `buttonPressed()`, que enciende el LED; ante un flanco ascendente confirmado se llama a `buttonReleased()`, que lo apaga.

### Funciones implementadas
- `debounceFSM_init()` — inicializa la MEF en el estado `BUTTON_UP`.
- `debounceFSM_update()` — actualiza la MEF; debe llamarse periódicamente desde el loop principal.
- `buttonPressed()` — acción ante flanco descendente: enciende el LED (PA5).
- `buttonReleased()` — acción ante flanco ascendente: apaga el LED (PA5).

### Estados de la MEF
```c
typedef enum {
    BUTTON_UP,       // Botón suelto (estable)
    BUTTON_FALLING,  // Posible presión detectada (rebote)
    BUTTON_DOWN,     // Botón presionado (estable)
    BUTTON_RAISING   // Posible liberación detectada (rebote)
} debounceState_t;
```

## Punto 2
La MEF se encapsula en un módulo reutilizable (`API_debounce.c` / `API_debounce.h`) ubicado en `Drivers/API/Src` y `Drivers/API/Inc`, respectivamente. El programa principal usa `readKey()` para detectar pulsaciones y alterna la frecuencia de parpadeo del LED entre 100 ms y 500 ms.

### Funciones públicas del módulo
- `debounceFSM_init()` — inicializa la MEF.
- `debounceFSM_update()` — actualiza la MEF; debe llamarse periódicamente.
- `readKey()` — devuelve `true` si hubo una pulsación válida y resetea el evento internamente.
- `buttonPressed()` — callback ante flanco descendente confirmado.
- `buttonReleased()` — callback ante flanco ascendente confirmado.

### Comportamiento
El LED parpadea de forma continua e independiente del botón. Cada pulsación válida alterna el período entre 100 ms (parpadeo rápido) y 500 ms (parpadeo lento). El manejo del LED y la detección del botón están completamente desacoplados.

## Archivos
- `Core/Src/main.c`
- `Core/Inc/main.h`
- `Drivers/API/Src/API_delay.c`
- `Drivers/API/Inc/API_delay.h`
- `Drivers/API/Src/API_debounce.c`
- `Drivers/API/Inc/API_debounce.h`

## Plataforma utilizada
- Microcontrolador: STM32F446RE
- IDE: STM32CubeIDE
- Lenguaje: C

## Autor
Pablo Nazareno Coronati
