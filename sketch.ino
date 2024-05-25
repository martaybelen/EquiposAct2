//Sensor temperatura y humedad: DHT22

//Librerias
#include <Wire.h>
#include <DHT.h>//Librería del DHT
#include <LiquidCrystal_I2C.h>//libreria de la LCD
#include <DallasTemperature.h>
#include <Servo.h>

//----------------DHT----------------//
//Pines sensor DHT
#define DHTTYPE DHT22
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
const float GAMMA = 0.7;
const float RL10 = 50;

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
float duracion, distancia;//Variables para almacenamiento y procesamiento de las mediciones
//-----------------------------------//

//-------Sensor temperatura externa-------//
#define sensorPin 7
float temp_externa;
OneWire oneWire(sensorPin);
DallasTemperature sensors(&oneWire);
float temp_objetivo = 25;
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
 

  //Definicion de pines del LED RGB como salida
  pinMode(PINRED, OUTPUT);
  pinMode(PINGREEN, OUTPUT);
  pinMode(PINBLUE, OUTPUT);
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

  myservo.attach(9);
  
  
  
  //Inicializacion de LCD
  lcd.init();
  lcd.createChar(5,humidity);
  lcd.createChar(7, temp);
  lcd.createChar(9, ilum);
  lcd.createChar(11, presence);
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
    modo = 0;
    Serial.println("Entrada en Modo 0: Sistema apagado"); //Se comunicara por el monitor el estado a que pasamos
  }
  if(contador == 1){
    
    modo =1;
    Serial.println("Entrada en Modo 1: Humedad y Temperatura"); 
    }
  if(contador == 2){
    modo = 2;
    Serial.println("Entrada en Modo 2: Iluminacion");
  } 
  if(contador == 3){
    modo = 3;
     LimpiezaLCD();
    Serial.println("Entrada en Modo 3: Presencia");
  } 
   if(contador == 4){
    modo = 4;
    LimpiezaLCD();
    Serial.println("Entrada en Modo 4: Temperatura exterior");
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
    lcd.write(7);//Añado el icono temperatura
    lcd.setCursor(2,0);
    lcd.print(temperatura,1);//Escribe valor temperatura tomado del sensor
    lcd.print(" C");

    //Enseña la humendad en el LCD
    lcd.setCursor(0,1);
    lcd.write(5); //Añado icono Humedad
    lcd.setCursor(2,1);
    lcd.print(humedad,1);//Escribe el valor humedad tomado del sensor
    lcd.print(" %"); 
  }

  if(modo == 2){ //Entramos en modo 2: Iluminacon
    //En este modo se muestra la iluminación en el LCD y se controla a intensidad del Led RGB
    
    //-------LDR: Iluminacion-----//
    ldrVal = analogRead(LDRPIN); //lectura del valor LDR
    lux = map(ldrVal, 0, 1023, 0, 100); //Normaliza el valor medido sobre 100 para obtener la iluminación en porcentaje
    lux = 100-lux;//Cuanto más iluminada este la habitacion mayor es el valor %
    int brillo; //Almacera el valor del led

    //Se representan la ilumiancion a traves de la intensidad de brillo del 
    //led RGB:La intensidad con la que brilla el led aumenta a medida que lo hace la variable lux
    if (lux==0){//el led no brilla si no hay luz
      brillo=0;
    }
    else if (lux>0 && lux<=25){
       brillo = (255*13)/100; //Se regula el brillo del led RGB      
    }
    else if (lux>25 && lux<=50){
      brillo = (255*38)/100; //Se regula el brillo del led RGB    
    }
    else if (lux>50 && lux<=75){
      brillo = (255*63)/100; //Se regula el brillo del led RGB
    }
    else{
    brillo = 255; //Se regula el brillo del led RGB
    }

    //Se regula el brillo del led RGB
    BrilloLed(0, 0, brillo);
    
    //Muestra de la información en LCD
    lcd.display();
    lcd.backlight();
    //LimpiezaLCD();
    lcd.setCursor(0,0);
    lcd.write(9); //Añado icono de iluminacion
    lcd.setCursor(2,0);
    lcd.print(lux,1);//Escribe el valor lux tomado del sensor
    //Cuanto más oscura esté la habitación menor es el porcentaje de luz ambiental
    lcd.print(" %");
    lcd.setCursor(0,1);
    lcd.print("                ");
  
  }

  if(modo == 3){//Entramos en modo 3: Presencia

    //Tomamos medidas del sensor de presencia: Se mide la distancia 
    //hasta un objeto u obstáculo entre 2 y 400cm
    digitalWrite(PINTRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PINTRIG, LOW);
    duracion = pulseIn(PINECHO, HIGH);//Medicion del tiempo de echo en us: determina la duracion del pulso
    distancia = (duracion/58); //Convierte el tiempo en distancia entre el sensor y el objeto
  
    if (distancia<400 && distancia>2 ){ // La medida se encuentra en el rango de distancias
    //de este sensor
      //Se activa el led RGB e ilumina en rojo
      BrilloLed(255, 0, 0);
      
      //Represnetacion de lo obtenido en el sensor en la LCD
      lcd.display();
      lcd.backlight(); 
      LimpiezaLCD();
      lcd.setCursor(0,0);
      lcd.write(11); //Simbolo para presencia
      lcd.setCursor(2,0);
      lcd.print(distancia, 1);
      lcd.print(" cm");
    }
    else{//En caso de fuera de rango se informa de ello en el LCD y no se enciende el led
      
      lcd.display();//Mostrar mensaje en LCD
      lcd.backlight(); 
      
      lcd.setCursor(0,0);
      lcd.write(11); //Simbolo para presencia
      lcd.setCursor(2,0);
      lcd.print(" No presencia");
      BrilloLed(0, 0, 0);//apagado led RGB
      
       delay(10);
    }
   
  }

if (modo == 4){//Modo temperatura exterior
  //float temp_externa = analogRead(PINLM35);
  sensors.requestTemperatures();
  float temp_externa = sensors.getTempCByIndex(0);
  
  //float millivolts = (temp_externa / 1023.0) * 5000;
  //float celsius = millivolts / 100; 
 
  delay(1000);
   //Se muestra las medidas tomadas del sensor en la LCD:
    lcd.display();
    lcd.backlight();
    //LimpiezaLCD();
    //Enseña la temperatura en el LCD
    lcd.setCursor(0,0);
    lcd.write(7);//Añado el icono temperatura
    lcd.setCursor(2,0);
    lcd.print(temp_externa,1);//Escribe valor temperatura tomado del sensor
    lcd.print(" C");

    //Algoritmo zona muerta
    int Enfriar = 0;
    int Calentar = 0;
    float LimInf = temp_objetivo - 3;
    float LimSup = temp_objetivo  +3;
    if (temp_externa>=LimSup) {
      Enfriar = 1;
      Calentar = 0;
      for (int angulo = 0; angulo <= 75; angulo++) {
        myservo.write(angulo);  // Mover el servo al ángulo especificado
        delay(15);  // Esperar 15ms para permitir que el servo alcance la posición
      }
    
      // Mover el servo de 180 a 0 grados
      for (int angulo = 75; angulo >= 0; angulo--) {
        myservo.write(angulo);  // Mover el servo al ángulo especificado
        delay(15);  // Esperar 15ms para permitir que el servo alcance la posición
      }
    }

    else if (temp_externa<=LimInf){
      Calentar = 1;
      Enfriar = 0;
      for (int angulo = 0; angulo <= 75; angulo++) {
        myservo.write(angulo);  // Mover el servo al ángulo especificado
        delay(15);  // Esperar 15ms para permitir que el servo alcance la posición
      }
    
      // Mover el servo de 180 a 0 grados
      for (int angulo = 75; angulo >= 0; angulo--) {
        myservo.write(angulo);  // Mover el servo al ángulo especificado
        delay(15);  // Esperar 15ms para permitir que el servo alcance la posición
      }
    }
    

}
  if(modo == 0){ //Entramos en modo apagado, el led RGB y la patalla del LCD se apagan
    
    BrilloLed(0, 0, 0);//apagado deL led
    LimpiezaLCD(); //Pantalla LCD vacia
    lcd.noBacklight();
  }
}

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
