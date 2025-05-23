#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>

const char* ssid = "La red de wifi";
const char* password = "La clave del internet";
const char* mqtt_server = "192.168.1.X";

WiFiClient espClient;
PubSubClient client(espClient);

// LCD
LiquidCrystal lcd(D1, D2, D5, D6, D7, D8);

// Pines
const int buzzer = D0;      // GPIO16
const int ledCambio = D3;   // GPIO0
const int botonSeleccion = D4; // GPIO2

// Estado
String criptoActual = "BTC";
float umbralBTC = 65000.0;
float umbralSOL = 150.0;
unsigned long intervalo = 5000;
unsigned long tiempoUltimoCambioBTC = 0;
unsigned long tiempoUltimoCambioSOL = 0;
unsigned long ultimoBoton = 0;

// Anti-rebote
bool estadoBotonAnterior = HIGH;

void setup_wifi() {
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  lcd.clear();
  lcd.print("WiFi conectado");
  delay(1000);
}

void mostrarPrecio(String cripto, float precio, float umbral) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(cripto + ": $" + String(precio, 2));

  lcd.setCursor(0, 1);
  if (precio > umbral) {
    lcd.print(":) ALTO");
    tone(buzzer, 1000, 300);
  } else {
    lcd.print(":() BAJO");
    tone(buzzer, 400, 300);
  }

  digitalWrite(ledCambio, HIGH);
  delay(300);
  digitalWrite(ledCambio, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String mensaje = String((char*)payload);
  float precio = mensaje.toFloat();
  String topico = String(topic);
  unsigned long ahora = millis();

  if (topico == "/cripto/BTC/precio" && criptoActual == "BTC") {
    if (ahora - tiempoUltimoCambioBTC > intervalo) {
      tiempoUltimoCambioBTC = ahora;
      mostrarPrecio("BTC", precio, umbralBTC);
    }
  }

  if (topico == "/cripto/SOL/precio" && criptoActual == "SOL") {
    if (ahora - tiempoUltimoCambioSOL > intervalo) {
      tiempoUltimoCambioSOL = ahora;
      mostrarPrecio("SOL", precio, umbralSOL);
    }
  }
}

void verificarBoton() {
  bool estadoBoton = digitalRead(botonSeleccion);
  if (estadoBoton == LOW && estadoBotonAnterior == HIGH && millis() - ultimoBoton > 300) {
    criptoActual = (criptoActual == "BTC") ? "SOL" : "BTC";
    lcd.clear();
    lcd.print("Seleccionado:");
    lcd.setCursor(0, 1);
    lcd.print(criptoActual);
    ultimoBoton = millis();
  }
  estadoBotonAnterior = estadoBoton;
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe("/cripto/BTC/precio");
      client.subscribe("/cripto/SOL/precio");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  lcd.begin(16, 2);
  pinMode(buzzer, OUTPUT);
  pinMode(ledCambio, OUTPUT);
  pinMode(botonSeleccion, INPUT_PULLUP); // ACTIVA resistencia interna

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  verificarBoton(); // revisa el botón constantemente
}
