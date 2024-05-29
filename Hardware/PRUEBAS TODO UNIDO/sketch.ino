#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <Servo.h>
#include <OneWire.h>

// Pines y definiciones
#define DHTTYPE DHT22
#define DHTPIN 3
#define LDRPIN A0
#define NUM_LEDS 8

#define TRIGPIN 11
#define ECHOPIN 12

#define ONE_WIRE_BUS 2
#define RGB_RED 6
#define RGB_GREEN 5
#define RGB_BLUE 3
#define SERVOPIN 9

// Umbrales de temperatura
const float TEMP_OBJETIVO = 25.0;
const float HISTERESIS = 3.0;
const float threshold = 80.0; // Umbral de 80%

DHT dht(DHTPIN, DHTTYPE); // Inicializa el sensor DHT
LiquidCrystal_I2C lcd(0x27, 20, 4); // Inicializa la pantalla LCD

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Servo myservo;

float lux = 0.0;
bool ventilacionActiva = false;
unsigned long previousMillis = 0;
const long interval = 10; // Intervalo en milisegundos para mover el servo
int pos = 0; // Posición actual del servo
int step = 3; // Paso de movimiento del servo

const int ledPins[NUM_LEDS] = {2, 4, 5, 6, 7, 8, 9, 10}; // Pines de los LEDs

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Configuración de pines de los LEDs como salida
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Apaga todos los LEDs inicialmente
  }

  // Configuración de pines del sensor de ultrasonidos
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  // Inicialización de la LCD
  lcd.init();
  lcd.backlight();

  // Inicia el sensor DS18B20
  sensors.begin();

  // Configuración de pines del LED RGB como salida
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  apagarLedRGB(); // Asegurarse de que el LED RGB esté apagado inicialmente

  // Configuración del servomotor
  myservo.attach(SERVOPIN);
  myservo.write(0); // Asegurarse de que el servomotor esté en la posición inicial
}

void loop() {
  // Leer el valor del sensor LDR
  int ldrVal = analogRead(LDRPIN);
  lux = (1023 - ldrVal) / 1023.0 * 100; // Normaliza el valor a un porcentaje

  // Mostrar el valor de iluminación en la LCD
  lcd.setCursor(0, 0);
  lcd.print("Iluminacion: ");
  lcd.print(lux, 1);
  lcd.print(" %");

  // Control de los LEDs
  controlarIluminacion(lux);

  // Medir la distancia con el sensor de ultrasonidos
  long distancia = medirDistancia();

  // Mostrar la distancia en la LCD
  lcd.setCursor(0, 1);
  lcd.print("Distancia: ");
  lcd.print(distancia);
  lcd.print(" cm");

  // Solicita la temperatura del sensor DS18B20
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);

  // Verifica si la lectura es válida
  if (temperatura == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: No se puede leer el sensor DS18B20.");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  // Ajusta el color del LED RGB basado en la temperatura
  adjustLedColor(temperatura);

  // Control del servomotor basado en la temperatura
  if (temperatura > TEMP_OBJETIVO + HISTERESIS) {
    // Temperatura por encima del umbral superior: activar ventilación
    ventilacionActiva = true;
  } else {
    ventilacionActiva = false;
    myservo.write(0); // Detener el servomotor
  }

  // Si la ventilación está activa, mover el servo como un ventilador
  if (ventilacionActiva) {
    moverServoComoVentilador();
  }

  delay(1); // Espera 1 segundo antes de la siguiente lectura
}

void controlarIluminacion(float lux) {
  int ledsToTurnOn = map(lux, 0, 100, 0, NUM_LEDS);

  // Apagar todos los LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  // Encender el número adecuado de LEDs
  for (int i = 0; i < ledsToTurnOn; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
}

long medirDistancia() {
  // Enviar un pulso de 10 microsegundos al pin TRIG
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);

  // Leer el tiempo que tarda en recibir el eco
  long duration = pulseIn(ECHOPIN, HIGH);

  // Calcular la distancia en centímetros
  long distance = duration * 0.034 / 2;

  // Limitar la distancia a 100 cm
  if (distance > 100) {
    distance = 100;
  }

  return distance;
}

void adjustLedColor(float temperature) {
  if (temperature > TEMP_OBJETIVO + HISTERESIS) { // Temperatura alta
    digitalWrite(RGB_RED, LOW);   // LED Rojo encendido
    digitalWrite(RGB_GREEN, HIGH);
    digitalWrite(RGB_BLUE, HIGH);
  } else if (temperature < TEMP_OBJETIVO - HISTERESIS) { // Temperatura baja
    digitalWrite(RGB_RED, HIGH);
    digitalWrite(RGB_GREEN, HIGH);
    digitalWrite(RGB_BLUE, LOW);  // LED Azul encendido
  } else { // Temperatura en el rango ideal
    digitalWrite(RGB_RED, HIGH);
    digitalWrite(RGB_GREEN, LOW); // LED Verde encendido
    digitalWrite(RGB_BLUE, HIGH);
  }
}

void moverServoComoVentilador() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    pos += step;
    if (pos >= 180 || pos <= 0) {
      step = -step; // Cambia la dirección del movimiento
    }
    myservo.write(pos);
  }
}

void apagarLedRGB() {
  digitalWrite(RGB_RED, HIGH);
  digitalWrite(RGB_GREEN, HIGH);
  digitalWrite(RGB_BLUE, HIGH);
}
