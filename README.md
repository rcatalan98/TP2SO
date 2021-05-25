## Autores
<hr>

- [Roberto José Catalán](https://github.com/rcatalan98)

- [Maria Victoria Conca](https://github.com/Mickyconca)
 
- [Gian Luca Pecile](https://github.com/glpecile)

<hr>

# Manual de Usuario
Sistema operativo básico basado en Barebonesx64 por RowDaBoat. Para poder correr el sistema se debe tener instalado qemu-system a fin de visualizar y docker instalado para el compilado con el contenedor `agodio/itba-so:1.0` dado por la cátedra. Al tener dichos requisitos se debe compilar desde docker y luego ejecutar usando:
1. La primera vez que se accede ```cd Toolchain``` y luego ```make all```. Este paso solo se debe realizar la primera vez que se descarga el repositorio. Además, se puede pasar un parametro para determinar el memory manager que se utilizara de la siguiente forma: 
- `make MM=BUDDY_MM all` (para utilizar el buddy).
- `make MM=FREELIST_MM all` (para utilizar free list).
De no funcionar, se puede cambiar la variable `MM` por la deseada dentro del `makefile` de `kernel`. 
2. Ahora resta una úntima compilación del proyecto volviendo al directorio anterior y utilizando ```make all```.
3. Por último resta correr el sistema utilizando qemu con el comando ```run.sh```.

## Shell
Intéprete de comandos del usuario, el sistema inicia al mismo por defecto. 
Dispone de las siguientes funcionalidades:
- **help**: Despliega en pantalla las funciones disponibles para el usuario. 
- **inforeg**: Despliega en pantalla un snapshot de los registros al momento que es llamado.
- **printmem**: Despliega en pantalla un volcado de memoria de 32 bytes a partir de una dirección de memoria válida recibida como argumento.
- **time**: Despliega en pantalla información sobre el día y hora actual del sistema.
- **invalidOpCodeException**: Verifica el funcionamiento correcto de la excepción de tipo operación de código inválido.
- **invalidZeroDivisionException**: Verifica el funcionamiento correcto de la excepción de tipo división por cero.
- **chess**: Despliega juego de ajedrez en formato gráfico humano a humano. Al escribir ```chess -man``` se despliega un menú con las reglas del juego. Además, se dispone de una opción para continuar una partida existente usando ```chess -c```. 
- **clear**: Limpia la pantalla de los comandos ingresados.
- ***ps:*** Despliega el estado de los procesos.
- ***loop:*** Crea un proceso el cual itera sobre si mismo.
- ***kill:*** Elimina el proceso cuyo pid es el recibido como argumento de entrada.
- ***nice:*** Cambia la prioridad del proceso cuyo pid es el recibido como argumento de entrada.
- ***block:*** Bloquea el proceso cuyo pid es el recibido como argumento de entrada.
- ***unblock:*** Desbloquea el proceso cuyo pid es el recibido como argumento de entrada.
- ***mem:*** Imprime el estado de la memoria del memory manager por bloques.
- ***test_processes:*** Prueba la creacion de procesos con el scheduler (archivo dado por la cátedra).
- ***test_prio:*** Prueba la prioridad del scheduler (archivo dado por la cátedra).
- ***test_sync:*** Prueba la sincronizacion de semaforos (archivo dado por la cátedra).
- ***test_no_sync:*** Prueba la sincronizacion sin el uso de semaforos (archivo dado por la cátedra).
- ***test_mm:*** Prueba el manejo de memoria del memory manager (archivo dado por la cátedra).
- ***pipe:*** Despliega información sobre de los pipes activos.
- ***sem:*** Despliega información sobre de los semaforos activos.
- ***wc:*** Cuenta las lineas recibidas como argumento de entrada.
- ***filter:*** Filtra las vocales recibidas como argumento de entrada.
- ***cat:*** Imprime en pantalla los caracteres recibidos como argumento de entrada.
- ***phylo:*** Dilema de los filosofos comiendo, inicia con 5 filosofos, más indiaciones son dadas al ejecutarlo.

## Shortcuts
- ***Shift + Caps Lock***: Permite ceder el estado foreground al siguiente proceso.

## Chess
¡Juegue una nueva partida o reanude su partida existente de ajedrez en formato gráfico por comando de línea! 
- Aplica toda regla general documentada del ajedrez. Se puede ganar el juego al realizar un haque mate o bien al comer al rey.
- En la parte inferior de la pantalla, centrado se encuentra un contador por turno donde el jugador no puede tener de diferencia de tiempo mayor a un minuto con respecto al tiempo que tomó la jugada del juador anterior. En caso de ser el primer moviemiento de la partida no puede superar un minuto el mismo. Si se supera dicho tiempo automáticamente gana la partida el jugador contrario.
- Para poder realizar un movimiento primero debe tener el formato ```'FROM_X''FROM_Y' 'TO_X''TO_Y'``` con **toda letra ingresada para un movimiento en mayúscula** debido a la existencia de caracteres especiales reservados:
  - Coronación: Todo peón es coronado a una reína por defecto.
  - Enroque: Para realizar un enroque corto ```e 2``` mientras que un enroque largo se realiza con ```e 3```.
  - Rotación: ```r``` para rotar 90° la posición de las piezas y tablero de ajedrez.
  - Salida: ```Esc``` la tecla de escape permite salir del juego con la habilidad de resumir la partida ingresando ```chess -c``` desde la terminal.
- Luego de realizar una jugada válida, la misma se imprimirá en pantalla, a modo de log, como jugada exitosa y luego se cambiara de turno reiniciando el contador. Se puede recrear la partida si se lo desea con las jugadas existentes del log.

## Testeo

Para el testeo con tanto **PVS-Studio** y **Cppcheck** se debe primero instalar **PVS-Studio** de no tenerlo, de la siguiente manera en su contenedor de *docker*.
```bash
apt-get update
apt-get install pvs-studio
pvs-studio-analyzer credentials "PVS-Studio Free" "FREE-FREE-FREE-FREE"
```
Luego correr el siguiente comando:
```bash
 make test
```
Los resultados se encontrarán de la siguiente manera:

 - **PVS-Studio:** report.tasks
 - **Cppcheck:** cppoutput.cppOut

Para remover los mismos, correr el comando `make cleanTest` en el mismo directorio donde fue realizada la compilación.