//Sensor temperatura y humedad: DHT22

//Librerias
#include <Wire.h>
#include <DHT.h>//Librería del DHT
#include <LiquidCrystal_I2C.h>//libreria de la LCD
#include <DallasTemperature.h>
#include <Servo.h>
#include <OneWire.h>

//----------------DHT----------------//
//Pines sensor DHT
#define DHTTYPE DHT11
#define DHTPIN 3  //Definición del pin como entrada del sensor DHT
DHT dht(DHTPIN, DHTTYPE); //inicializa el sensor DHT
//Se definen temperatura y humedad para almacenar lo medido por el sensor
float temperatura = 0.0;
float humedad = 0.0;
//--------------------------------//

//-----------------LDR------------------//
//Variables LDR
#define LDRPIN A0 //Pin entrada 0 analógico
int ldrVal = 0;
float lux = 0.0;
float lux_anterior;
float voltage=0.0;
#define NUM_LEDS 8
#define PINSW A3
int leds_encendidos = 0;
float primera_medida=false;
const int ledPins[NUM_LEDS] = {13, 12, 11, 9, 8, 5, 4, 2};
                  // R1  R2  G1  G2 Y1 Y2 B1 B2  
//----------LED RGB----------//
#define PINRED 13//Definción de los pines del led RGB
#define PINGREEN 12
#define PINBLUE 11
//-------------------------------------//

//-------------LCD---------------//
LiquidCrystal_I2C lcd(0x27, 20,4);

//Simbolos LCD Humnedad
byte humidity[8] = {B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110};
//Simbolo LCD Temperatura
byte temp[8] = {B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110};
//Simbolo LCD iluminacion
byte ilum[8] = {B00000, B10101, B01110, B11011, B01110, B10101, B00000, B00000};
//Simbolo sensor HC-SR04
byte presence[8] = {B01110, B01110, B00100, B11111, B00100, B01110, B01010, B01010};

//------------------------//

//-----------PULSADOR------------------//
#define PINPULS A2 //pulsador con el pin analogico A2
int modo = 0; //modo 0, todo apagado
int PulsState = LOW; //Estado inicial del pulsador: Sin pulsar
bool flag = false; //Flag de interrupción del pulsador, incialmente inactivo
int contador = 0; //Contador del modo, me sirve para ir cambiando de un modo a otro
uint32_t timerBoton = 0; // Timer del botón
//--------------------------//

//----------Sensor de presencia----------//
#define PINTRIG 5//Pines del sensor SR04
#define PINECHO 6
float duracion, distanciaanterior;//Variables para almacenamiento y procesamiento de las mediciones
float distancia=0;
//-----------------------------------//

//-------Sensor temperatura externa-------//
#define sensorPin 7
float temp_externa_anterior, temp_interna_anterior;
float temp_externa_actual =0;
float temp_interna_actual =0;
unsigned long previousMillis = 0;
const long interval = 10; // Intervalo en milisegundos para mover el servo
int angulo =0;
long retardo =0;
int step = 2; // Paso de movimiento del servo


OneWire oneWire(sensorPin);
DallasTemperature sensors(&oneWire);
float temp_objetivo = 25;
float LimInf = temp_objetivo - 3;
float LimSup = temp_objetivo  +3;
int ventilar = 0;
boolean vuelta=false;
//Servomotor
Servo myservo;
#define SERVOPIN 10
int pos =0;
//--------------------------------------//

//------------------------------------//
void setup() {
  // put your setup code here, to run once:
  //inicia comunicación serial
  Serial.begin(9600);
  Serial.println("Iniciando el sistema"); 

  //Inicializa el sensor, llamada a la función dht
  dht.begin();

  //Configura pin LDR como entrada
  pinMode(LDRPIN, INPUT);

  
  pinMode(PINSW, INPUT_PULLUP);
 

  //Definicion de pines del LED RGB como salida
  pinMode(PINRED, OUTPUT);
  pinMode(PINGREEN, OUTPUT);
  pinMode(PINBLUE, OUTPUT);

  pinMode(ledPins[4], OUTPUT);
  pinMode(ledPins[3], OUTPUT);
  pinMode(ledPins[6], OUTPUT);
  pinMode(ledPins[7], OUTPUT);
  //apagado inicial deL led
  analogWrite(PINRED, 0);
  analogWrite(PINGREEN, 0);
  analogWrite(PINBLUE, 0);

  //Configuracion pines del sensor de presencia
  pinMode(PINTRIG, OUTPUT);
  pinMode(PINECHO, INPUT);
  digitalWrite(PINTRIG, LOW);

  //Configuración pin LM35
  pinMode(sensorPin, INPUT);
  // Inicialización del sensor
  sensors.begin();
  delay(20);

  // Servomotor 
 // myservo.attach(SERVOPIN);
  //myservo.write(0);
  
  
  //Inicializacion de LCD
  lcd.init();
  lcd.createChar(3,humidity);
  lcd.createChar(5, temp);
  lcd.createChar(7, ilum);
  lcd.createChar(9, presence);
 
  lcd.display();
    
  //configuracion del pulsador como entrada
  pinMode(PINPULS, INPUT_PULLUP);//Configuro como entrada pull-up

}

void loop() {


  //------Tratamiento del pulsacion del boton--------//
  // los modos se sucende de la siguiente forma: 
  //Modo 0-> Modo 1-> Modo 2-> Modo 3-> Modo 0 -> Modo 1....
  //de formque al pulsar se pasa de un modo a otro siguiendo este orden.
  //Al pulsar la primera vez se pasa al modo 1, si volvemos a pulsar pasamos al modo 2, 
  //pulsar otra vez nos lleva al modo 3 y pulsar otra vez nos devuelve al modo 0.
  //Estando en el modo i solo es posible pasar al modo i+1.
 bool btnState = !digitalRead(PINPULS);
 if(btnState && !flag && millis()- timerBoton >50){//Para asegurarnos de que se trata de una pulsación se establece un tiempo minimo
  flag = true;//Se activa el flag de pulsación
  timerBoton = millis();
  contador+=1;//El contador permitirá relacionar la pulsacion con el modo correspondiente
  //apagado inicial del led 
    BrilloLed(0, 0, 0);

  if(contador==5) contador = 0;
  if(contador == 0){
    //myservo.write(0);
    BrilloLed(0, 0, 0);//Apagado de led RGB
    myservo.detach();//quito el servo, por ahora no lo vamos a usar
    modo = 0;
    Serial.println("Entrada en Modo 0: Sistema apagado"); //Se comunicara por el monitor el estado a que pasamos
  }
  if(contador == 1){
    
    modo =1;
    Serial.println("Entrada en Modo 1: Humedad y Temperatura"); 
    }
  if(contador == 2){
    modo = 2;
    ldrVal=0;
    lux= 0.0;
    lux_anterior = 0.0;
    leds_encendidos = 0;
    primera_medida = true;
    LimpiezaLCD();
    Serial.println("Entrada en Modo 2: Iluminacion");
  } 
  if(contador == 3){
    modo = 3;
    distancia=0;
     LimpiezaLCD();
    Serial.println("Entrada en Modo 3: Presencia");
  } 
   if(contador == 4){
    modo = 4;
    temp_interna_actual = 0;
    temp_externa_actual=0;
    ventilar =0;
    distancia = 0;
    myservo.attach(SERVOPIN);//Conectamos el servo
    LimpiezaLCD();
    Serial.println("Entrada en Modo 4: Control de Temperatura");
  } 

 }
 if (!btnState && flag && millis()-timerBoton >50){
    flag = false;//Flag de pulsación desactivado
    timerBoton = millis();
 }
  //------LCD----------//
  if (modo ==1){ //Entramos en modo 1: Temperatura y Humedad

    //------DHT: Humedad y Temperatura----//
    //Se lee temperatura y humedad del sensor y se almacena en las variables:
    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();
  
    //Se muestra las medidas tomadas del sensor en la LCD:
    lcd.display();
    lcd.backlight();
    //Enseña la temperatura en el LCD
    lcd.setCursor(0,0);
    lcd.write(5);//Añado el icono temperatura
    lcd.setCursor(2,0);
    lcd.print(temperatura,1);//Escribe valor temperatura tomado del sensor
    lcd.print(" C");

    //Enseña la humendad en el LCD
    lcd.setCursor(0,1);
    lcd.write(3); //Añado icono Humedad
    lcd.setCursor(2,1);
    lcd.print(humedad,1);//Escribe el valor humedad tomado del sensor
    lcd.print(" %"); 
  }

  if(modo == 2){ //Entramos en modo 2: Iluminacon
    //En este modo se muestra la iluminación en el LCD y se controla a intensidad con 8LEDS
    float margen_lux = 2.0;
    //-------LDR: Iluminacion-----// 

    //El switch es para cambiar de modo diurno (Control 80%) a modo nocturno (sin Control)
    boolean estadoSwitch = digitalRead(PINSW);  
    if (estadoSwitch == HIGH) {
      //Modo diurno: Control ilumiación al 80%
     // if (lux<80-margen_lux || lux>80+margen_lux){
        //Si no estoy dentro del margen deseado enciendo un led más
       // leds_encendidos = controlarIluminacion(lux, leds_encendidos); //Se enciende los leds correpondientes 
        
     // }
    int ldrValue = analogRead(LDRPIN);
  
    // Convertir el valor del LDR a un porcentaje de iluminación (0-100%)
    //Para lograr visualizar el efecto del control al 80% se parte de una medida tomada
    //por el sensor, el resto de iteraciones usará como medida de lux un incremento
    //del valor medido para simular un incremento de la luex al haber encendido leds
    if(primera_medida==true){
       lux_anterior = lux;
      lux = map(ldrValue, 0, 1023, 100, 0);
      primera_medida=false;
    }
   
    mostrarIluminacion(lux, lux_anterior);
    lux = controlarIluminacion(lux) ;

    
  // Esperar un poco antes de la próxima lectura
  delay(200);

      
    } else {
      //Modo nocturno: No control iluminación al 80%: enciende los leds correspondientes nada mas
      lux_anterior = lux;
      ldrVal = analogRead(LDRPIN); //lectura del valor LDR
      lux = (1023 - ldrVal) / 1023.0 * 100;
      if(lux_anterior != lux){
        mostrarIluminacion(lux, lux_anterior);
        encenderLeds(lux);
      }
    }
    
    
  }

 if(modo == 3){//Entramos en modo 3: Presencia

    //Tomamos medidas del sensor de presencia: Se mide la distancia
    //hasta un objeto u obstáculo entre 2 y 100cm
    distanciaanterior=distancia;  
    distancia = medirDistancia();

    //Establezco un criterior para controlar el refresco de la distancia
    if(distancia > distanciaanterior+3 || distancia < distanciaanterior-3){
      lcd.display();
      lcd.backlight();
      LimpiezaLCD();
       if (distancia<200 && distancia>2 ){ // La medida se encuentra en el rango de distancias
      //de este sensor
      //Se activa el led RGB e ilumina en rojo
      BrilloLed(255, 0, 0);
      //Represnetacion de lo obtenido en el sensor en la LCD
      
      lcd.setCursor(0,0);
      lcd.write(9); //Simbolo para presencia
      lcd.setCursor(2,0);
      lcd.print(distancia, 1);
      lcd.print(" cm");
      delay(10);
    }
    else{//En caso de fuera de rango se informa de ello en el LCD y no se enciende el led

      lcd.setCursor(0,0);
      lcd.write(9); //Simbolo para presencia
      lcd.setCursor(2,0);
      lcd.print(" No presencia");
      BrilloLed(0, 0, 0);//apagado led RGB

       delay(10);
      }
    }
  }

if (modo == 4){//Modo control de temperatura
  //float temp_externa = analogRead(PINLM35);
  

  //Tomamos medidas de temperatura
  temp_externa_anterior = temp_externa_actual;
  temp_interna_anterior = temp_interna_actual;
  temp_interna_actual = dht.readTemperature();
  sensors.requestTemperatures();
  temp_externa_actual = sensors.getTempCByIndex(0);
  
  distanciaanterior = distancia;
  distancia = medirDistancia();
   //Si se produce algún cambio en las temperaturas entramos en el if
   if(temp_externa_actual != temp_externa_anterior || temp_interna_actual!= temp_interna_anterior || distancia > distanciaanterior+3 || distancia < distanciaanterior-3){

    mostrar_temperaturas();
      //Si la temperatura exterior es mas baja abrimos ventilación
      float temp_media = (temp_externa_actual+temp_interna_actual)/2;
        Serial.print("Temperatura media: ");
        Serial.println(temp_media);
      if(temp_interna_actual > LimSup){
        
        BrilloLed(255, 0, 0); //Led en rojo para simbolizar que hace calor
        //Si la media de las dos temperatura es menor que la que hay había en la habitación consideramos
        //que abrir la ventilación referscará la habitación
        if(temp_media < temp_interna_actual){

          //Abrir ventilación es poner el servo con angulo 180 (Totalmente abierta)
          myservo.write(180);
          ventilar = 1;
          Serial.println("Abrimos ventilación para refrescar la habitación");
          //temp_interna_actual = temp_media;
        }
        else{
         myservo.write(0);
          ventilar = 0;
          Serial.println("Fuera hace demasiado calor, cerramos ventilación");
        }
      }
      //Otra opción es que haga más frio dentro que fuera y esté por debajo de limInf
      else if(temp_interna_actual < LimInf){
        BrilloLed(0, 0, 255); //Led en azul para simbolizar que hace frio

        if(ventilar==0 && temp_media>=temp_interna_actual ){
          //Si la media entre fuera y dentro está por encima de la temperatura actual del cuarto y está cerrada la escotilla
          //abrimos ventilación un angulo de 90 grados
          myservo.write(90);
          ventilar =1;
          Serial.println("Abrimos ventilación para calentar la habitación, fuera hace suficiente calor");
          
        }
        //Cerramos ventilación en caso de que esté abierta si la temperatura interior esta por debajo de la actual
        if(ventilar ==1 && temp_media<temp_interna_actual){ 
          myservo.write(0);
          ventilar = 0;
          Serial.println("Ventalación abierta pero hace demasiado frio fuera: Cerramos");
          
          }
      }

      if(temp_interna_actual <=LimSup && temp_interna_actual >= LimInf){
        //Abrimos la escotilla 45 grados para permitir ventilación, aire limpio
        BrilloLed(0, 255, 0);//Led en verde simboliza temperatura ideal
       
        //Si la sala está vacía aprovecho para ventilar el cuarto
       
       if(distancia >=200 || distancia <=2){
        myservo.write(45);
        Serial.println("Temperatura ideal: Ventilamos el cuarto (45ª)");
        } 
        //Si hay alguien cierro la ventilación
        else{
          Serial.println("Cerramos ventilación");
          myservo.write(0);
        }
      }
   }
    
}  


  if(modo == 0){ //Entramos en modo apagado, el led RGB y la patalla del LCD se apagan
    
    BrilloLed(0, 0, 0);//apagado deL led
    LimpiezaLCD(); //Pantalla LCD vacia
    lcd.noBacklight();
  }
}

//--------------------FUNCIONES AUXILIARES----------------------//
//Esta funcion limpia el LCD y deja la pantalla sin contenido
void LimpiezaLCD(){ 
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,2);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("                ");
}

//Esta funcion sirve para regular el led RGB y modificar su valor
void BrilloLed(int rojo, int verde, int azul){
  analogWrite(PINRED, rojo);
  analogWrite(PINGREEN, verde);
  analogWrite(PINBLUE, azul);
}



long medirDistancia() {
  // Enviar un pulso de 10 microsegundos al pin TRIG
  digitalWrite(PINTRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINTRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINTRIG, LOW);

  // Leer el tiempo que tarda en recibir el eco
  long duration = pulseIn(PINECHO, HIGH);

  // Calcular la distancia en centímetros
  long distance = duration * 0.034 / 2;

  //El rango del sensir es de 2 a 200cm, fuera del rango se considera que no
  //detecta presencia de nada: Habitación vacía
  if (distance > 200){
    distance=200;
  }
  else if(distance <2){
    distance=2;
  }
  return distance;
}

void mostrar_temperaturas(){
 //Mostramos temperaturas en el LCD
 //Se muestra las medidas tomadas del sensor en la LCD:
    lcd.display();
    lcd.backlight();
    LimpiezaLCD();
    //Enseña la temperatura exterior en el LCD
    lcd.setCursor(0,0);
    lcd.write(5);//Añado el icono temperatura
    lcd.setCursor(2,0);
    lcd.print("Exter: ");
    lcd.print(temp_externa_actual,1);//Escribe valor temperatura tomado del sensor
    lcd.print(" C");

    //Enseña la temperatura interior en el LCD
    lcd.setCursor(0,1);
    //lcd.write(11); //Añado icono Humedad
    lcd.setCursor(2,1);
     lcd.print("Int: "); 
    lcd.print(temp_interna_actual,1);//Escribe el valor humedad tomado del sensor
    lcd.print(" C"); 
}


void encenderLeds (float lux){
  int ledsToTurnOn = map(lux, 100, 0, 0, NUM_LEDS);
    
  // Apagar todos los LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  // Encender el número adecuado de LEDs
  for (int i = 0; i < ledsToTurnOn; i++) {
    digitalWrite(ledPins[i], HIGH);   
  }
 if(lux<=5){
   digitalWrite(2, HIGH);
   ledsToTurnOn++;
 }
}


float controlarIluminacion(float lux) {
  
  // Mostrar el porcentaje calculado en el monitor serie
    Serial.print(" - Illumination: ");
    Serial.print(lux);
    Serial.println("%");
  
    // Calcular la diferencia respecto al umbral deseado
    int difference = lux - 80;
    
    // Determinar la cantidad de LEDs a encender
    int ledsToTurnOn = NUM_LEDS * lux / 100;
    Serial.print("Leds a encender:");
    Serial.println(ledsToTurnOn);

    // Ajustar la cantidad de LEDs encendidos para mantener la iluminación al 80%
    if (difference < -2) {
      ledsToTurnOn++;
      //Esto de aquí es para simular el efecto de que aumenta la iluminación al encender un led más
      lux_anterior = lux;
      lux= lux+2;//Para simular efecto del control
    } else if (difference > 2) {
      ledsToTurnOn--;

      //Esto lo mismo pero para simular que reduce la luz al apagar uno más
      lux_anterior = lux;
      lux=lux-2;//Para simular el efecto del control
    }
  
   // Limitar el número de LEDs a encender entre 0 y NUM_LEDS
    ledsToTurnOn = constrain(ledsToTurnOn, 0, NUM_LEDS);
  
  // Encender/Apagar los LEDs según sea necesario
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < ledsToTurnOn) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
  return lux;
    
}
  
  void mostrarIluminacion(float lux, float lux_anterior){
 // Control de los LEDs
    if(lux_anterior != lux){
                
     // controlarIluminacion(lux);
      //Muestra de la información en LCD
      lcd.display();
      lcd.backlight();
      LimpiezaLCD();
      lcd.setCursor(0,0);
       lcd.write(7); //Añado icono de iluminacion
      lcd.setCursor(2,0);
      lcd.print(lux,1);//Escribe el valor lux tomado del sensor
      //Cuanto más oscura esté la habitación menor es el porcentaje de luz ambiental
      lcd.print(" %");
      lcd.setCursor(0,1);
      lcd.print("                ");
      //encenderLeds(lux);
   }
  }
