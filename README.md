# Conecta4

Redes de computadores - Practica 5: juego conecta 4 online

### Prerequisitos

Ninguno

## Compilacion

Compilar todo
```
make all 
```
o 
```
make
```

Objetivo 1
```
make obejtivo1
```

Objetivo 2
```
make obejtivo2
```

Objetivo 3
```
make obejtivo3
```
Eliminar ejecutables
```
make clean
```


## Objetivo 1

* Si el cliente pulsa CTRL+C, se captura la señal (SIGINT) y le manda la palabra  "SALIR\n" al servidor.
	  
* Si el servidor recibe "SALIR\n", finaliza la partida, y cierrra la conexiones con los clientes. 

* En el obejtivo 1, si uno de los jugadores abandona la partida, el servidor recibira "SALIR\n" por parte del jugador que ha abandonado la partida y le enviaria "SALIR\n" al otro jugador. Seguidamente finalizara la partida (eliminarla del array de partidas) y cerrara conexion con ambos jugadores.

* En el obejtivo 2, al haber solo un jugador, si el servidor recibe "SALIR\n", simplemente finalizara la partida y cerrara conexion con el jugador.


## Objetivo 2

* El servidor admite mas de una partida simultaneamente. El numero de partidas sumulaneas esta establecido en las constante. Si el numero de partidas es igual al maximo y alguien se intentara conectara, se le envia el mensaje "FULL".

* Por tema de limpieza y claridad del codigo, el bot ha sido añadido como una libreria. La funcion de la libraria encarga de devolver la columna elegida por el bot se llama "simulador".

* La compilacion con la libreria del bot, esta incluida en Makefile obejtivo 2

```
make objetivo2
```

## Objetivo 3

* El tablero es sacado siempre por la salida estandar de linux stdout(terminal), tanto por parte del cliente como del servidor. El tablero es impreso cada vez que uno de los jugadores meta ficha en el tablero.


## Estilo de programacion

[coding-style.rst](https://github.com/torvalds/linux/blob/master/Documentation/process/coding-style.rst)

## Autores

* **Ruben Cherif Narvaez** - 99rubenche@gmail.com - [rabiixx](https://github.com/rabiixx)
