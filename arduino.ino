#include <DHT.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Servo.h>

Servo servo;
int posServo = 0;

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
IPAddress ip(192, 168, 1, 10);
EthernetServer server(80);

String request = "";

int temperature1 = 30;
int temperature2 = 31;
int temperature3 = 32;
int temperature4 = 33;

int led1 = 34;
int led2 = 35;
int led3 = 36;
int led4 = 37;
boolean ledState[4] = { 0 };  // TODO: Can remove this line and use `digitalRead` for get the state led

int servoMotor = 22;
int fireAlarm = 23;
int waterPump = 24;
int fan = 25;
int gas = A8;

DHT dht1(temperature1, DHT11);
DHT dht2(temperature2, DHT11);
DHT dht3(temperature3, DHT11);
DHT dht4(temperature4, DHT11);

void setup() {
  servo.attach(servoMotor);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Serial.begin(9600);

  Serial.println(F("Initializing SD card..."));

  if (!SD.begin(4)) {
    Serial.println(F("SD card initialization failed"));
    return;
  }

  Serial.println(F("SD card initialized"));
  if (!SD.exists("index.htm")) {
    Serial.println(F("Can't find index.htm file!"));
    return;
  }

  if (!SD.exists("tps.htm")) {
    Serial.println(F("Can't find tps.htm file!"));
    return;
  }

  Serial.println(F("Found index.htm and tps.htm file."));
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print(F("Server is at "));
  Serial.println(Ethernet.localIP());
  Ethernet.begin(mac, ip);

  initializationPins();
  initializationTemperatures();
  activateAlarmWithInterval(fireAlarm, 1, 1000);
}

void initializationPins() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(fireAlarm, OUTPUT);
  pinMode(waterPump, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(gas, INPUT);
}

void initializationTemperatures() {
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
}

void loop() {
  runServer();
  checkGas();
  temperatures();
}

void runServer() {
  EthernetClient client = server.available();

  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request.concat(c);

        if (c == '\n' && currentLineIsBlank) {
          client.println(F("HTTP/1.1 200 OK"));

          if (strContains(request, "leds")) {
            sendCommonHeaders(client, "text/xml");
            setLEDs(request);
            responseLeds(client);
          } else if (strContains(request, "door")) {
            sendCommonHeaders(client, "text/xml");
            setDoor(request);
            responseDoor(client);
          } else if (strContains(request, "temp")) {
            sendCommonHeaders(client, "text/xml");
            responseTemperatures(client);
          } else if (strContains(request, "tps")) {
            sendHtmlResponse(client, "tps.htm");
          } else {
            sendHtmlResponse(client, "index.htm");
          }
          request = "";
          break;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void sendCommonHeaders(EthernetClient client, String contentType) {
  client.println(String("Content-Type: " + contentType));
  client.println(F("Connection: keep-alive"));
  client.println();
}

void sendHtmlResponse(EthernetClient client, String fileName) {
  Serial.println(String("Open HTML file: " + fileName));
  sendCommonHeaders(client, "text/html");
  File webFile = SD.open(fileName);
  if (webFile) {
    while (webFile.available()) {
      client.write(webFile.read());
    }
    webFile.close();
  }
}

void checkGas() {
  int gasReading = analogRead(gas);
  Serial.println(gasReading);
  if (gasReading > 220) {
    activateGasEmergencySystem();
  } else {
    deactivateGasEmergencySystem();
  }
}

void activateGasEmergencySystem() {
  digitalWrite(fan, HIGH);
  digitalWrite(led4, HIGH);
  activateAlarmWithInterval(fireAlarm, 3, 1000);
}

void deactivateGasEmergencySystem() {
  digitalWrite(fan, LOW);
  digitalWrite(fireAlarm, LOW);
  digitalWrite(led4, LOW);
}

void temperatures() {

  bool isAnyTemperatureExceedingThreshold = dht1.readTemperature() > 33 || dht2.readTemperature() > 33 || dht3.readTemperature() > 33 || dht4.readTemperature() > 33;

  if (isAnyTemperatureExceedingThreshold) {
    activateTemperatureEmergencySystem();
  }

  // float temperatures[] = { dht1.readTemperature(), dht2.readTemperature(),
  //                          dht3.readTemperature(), dht4.readTemperature() };
  // bool isAnyTemperatureExceedingThreshold = std::any_of(
  //   std::begin(temperatures), std::end(temperatures),
  //   [](float temp) {
  //     return temp > 33;
  //   });
  // if (isAnyTemperatureExceedingThreshold) {
  //   activateTemperatureEmergencySystem();
  // }
}

void activateTemperatureEmergencySystem() {
  digitalWrite(fan, HIGH);
  digitalWrite(waterPump, HIGH);
  activateAlarmWithInterval(fireAlarm, 3, 1000);
}

void activateAlarmWithInterval(int pin, int cycles, int interval) {
  for (int i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delay(interval);
    digitalWrite(pin, LOW);
    delay(interval);
  }
}

void setLEDs(String request) {
  // LED 1 (pin 34)
  if (strContains(request, "LED1=1")) {
    ledState[0] = true;
    digitalWrite(led1, HIGH);
  }
  if (strContains(request, "LED1=0")) {
    ledState[0] = false;
    digitalWrite(led1, LOW);
  }

  // LED 2 (pin 35)
  if (strContains(request, "LED2=1")) {
    ledState[1] = true;
    digitalWrite(led2, HIGH);
  }
  if (strContains(request, "LED2=0")) {
    ledState[1] = false;
    digitalWrite(led2, LOW);
  }

  // LED 3 (pin 36)
  if (strContains(request, "LED3=1")) {
    ledState[2] = true;
    digitalWrite(led3, HIGH);
  }
  if (strContains(request, "LED3=0")) {
    ledState[2] = false;
    digitalWrite(led3, LOW);
  }
}

String doorIsOpen = "on";

void setDoor(String request) {
  if (strContains(request, "DOOR=1")) {
    for (posServo = 180; posServo >= 0; posServo -= 5) {
      servo.write(posServo);
    }

    doorIsOpen = "on";

    Serial.println("Is open servo");
  }

  if (strContains(request, "DOOR=0")) {
    for (posServo = 0; posServo <= 100; posServo += 5) {
      servo.write(posServo);
    }

    doorIsOpen = "off";

    Serial.println("Is close servo");
  }
}

void responseLeds(EthernetClient client) {
  client.print(F("<?xml version = \"1.0\" ?>"));
  client.print(F("<inputs>"));
  // Iterate over the LED states array
  for (int i = 1; i < 4; i++) {
    printResponseTag(client, "LED" + String(i), ledState[i] ? "on" : "off");
  }
  client.print(F("</inputs>"));
}

void responseDoor(EthernetClient client) {
  client.print(F("<?xml version = \"1.0\" ?>"));
  client.print(F("<inputs>"));
  printResponseTag(client, "DOOR", doorIsOpen);
  client.print(F("</inputs>"));
}

void responseTemperatures(EthernetClient client) {
  client.print(F("<?xml version = \"1.0\" ?>"));
  client.print(F("<inputs>"));
  float temperatures[] = { dht1.readTemperature(), dht2.readTemperature(),
                           dht3.readTemperature(), dht4.readTemperature() };
  for (int i = 0; i < sizeof(temperatures); i++) {
    printResponseTag(client, "Temperature" + String(i + 1), String(temperatures[i]));
  }
  client.print(F("</inputs>"));
}

void printResponseTag(EthernetClient client, String label, String value) {
  client.print("<" + label + ">");
  client.print(value);
  client.print("</" + label + ">");
}

boolean strContains(String str, String sfind) {
  return str.indexOf(sfind) != -1;
}
