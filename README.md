# EquiposAct2
## Repositorio para Actividad 2 Equipos e Instrumentación
#### En esta actividad se propone el desarrollo de un sistema de control de temperatura e iluminación. 

#Link proyecto WOKWI https://wokwi.com/projects/398941635078214657
#Link proyecto GITHUB https://github.com/martaybelen/EquiposAct2.git
### Poyecto de partida:
#### Se parte del proyecto realizado en la actividad anterior que cuenta con un sistema de 4 modos:
|Modos|Funcionalidad|
|:----|------------:|
|Modo 0| Es el modo inicial del sistema, todo apagado|
|:----|------------:|
|Modo 1| Modo que toma medidas de temperatura y humendad mediante el DHT11/22|
|:----|------------:|
|Modo 2| Modo que toma medidas de iluminación mediante LDR|
|:----|------------:|
|Modo 3| Modo que toda medidas para detectar la proximidad de un cuerpo (cm)|

#### Los resultados de las mediciones a través del correspondiente sensor en cada modo se presentada en una pantalla LCD.
#### La conmutación entre modos se realiza mediante un pulsador de forma que cada vez que se pulsa se pasa al siguiente modo. El orden es el siguiente: modo0->modo1->modo2->modo3->modo0->modo1->...

### Novedades incorporadas al sistema:
#### Nuevo modo 4 para el control de temperatura en el interior de una habitación
Este nuevo modo toma **dos** medidas de temperatura: 
|Medida|Funcionalidad|
|:----|------------:|
|Temperatura interna| Mediante el DHT para conocer temperatura de la habitación|
|:----|------------:|
|Temperatura externa| Mediante el LM35 para conocer temperatura del exterior|
|:----|------------:|

**Lógica para el control**
Algoritmo de zona muerta: Temperatura ideal de la habiración es 25 grados. Se añade un margen de -+3 grados. se define **limite inferior (25-3) y límite superior (25+3)**.En base a esto hay tres opciones:
| Condición | Acción | LED |
|:----|------------|----|---:|
|temperatura interior < limite inferior| Calentar | Azul  |
|:----|------------|----|---:|
|temperatura interior > limite superior| Enfriar  | Rojo  |
|:----|------------|---|----:|
|limInf< temperatura interna < limsup  | Ventilar | Verde |
|:----|------------|---|----:|

Las funciones de calentar, enfriar y ventilar dependen de la temperatura media entre el interior y el exterior y se reliazarán mediante un servomotor, el ángulo de este representa la abertura de una rejilla de ventilación.
**Calentar:** Si la temperatura media es mayor que la de la habitación, abrir -> 90 grados.
**Enfriar:**Si la temperatura media es menor que la de la habitación, abrir -> 180 grados.
**Ventilar:** Si no hay nadie en la sala, abrir -> 45 grados. ( detección de presencia con el mismo método que el modo 3)

En otros casos la escotilla permanece cerrada.
#### Nuevas funciones auxiliares incluidas para desarrollar el modo 4
**medirDistancia():** Se utiliza para detectar presencia en modo 3 (mejora de la anterior versión) y en modo 4.
**mostrar_temperaturas():**Muestra en el LCD las temperaturas interna y externa

#### Modificación del modo 2: Introducimos el control de iluminación
Modificación del modo 2 para control de la iluminación. Se definen dos modos de funcionamiento controlados por un switch
|Modos|Funcionalidad|
|:----|------------:|
|Modo nocturno| Iluminación de los 8 leds a corde con el sensor|
|:----|------------:|
|Modo diurno| Control de iluminación para lograr entorno al **80%**|
|:----|------------:|
#### Nuevas funciones auxiliares incluidas para desarrollar el nuevo modo 2
**encenderLeds():**Función para el modo nocturno. Selecciona los leds según el sensor que hay que encender y los enciende. A menor iluminación más leds se necesitan para iluminar la habitación.
**controlarIluminacion():**enciende leds hasta lograr una iluminación en torno al 80%, Si se supera el margen, apaga leds. Se establece un rango de iluminación entre 78% y 82%.
**mostrarIluminacion():**Muestra en el LDC la lux en % cada vez que hay una actualización de su valor.

### NOTA IMPORTANTE WOKWI:
Para simular la repercusión de encender leds para lograr ese 80% se fuerzan variaciones de la variable lux que demuestren el funcionamiento de este modo. Se comienza con una medición tomada del sensor y después se continúa como valor de lux el resultante de la función **controlarIluminacion()**. Cuando se enciende un led más el lux obtenido se incrementa +2, cuando se apaga un led se reduce -2. De esta forma se espera simular el impacto que tendría en la realidad regular la iluminación de la sala.

### NOTA IMPORTANTE PINES:
Debido a la insufuencia de pines se reutilizan algunos para varias funcionalidades diferentes, como puede ser los leds del modo iluminación y el led RGB que sin ser empleado en este modo, su iluminación varía debido a compartir pines con otros leds.