#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <SD.h>
#include <Servo.h>
#define REQ_BUF_SZ 50

Servo servo;
int posServo = 0;

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
IPAddress ip(192, 168, 1, 10);
EthernetServer server(80);

boolean ledState[4] = {0};

File webFile;
char HTTP_req[REQ_BUF_SZ] = {0};
char requestIndex = 0;

int temperature1 = 30;
int temperature2 = 31;
int temperature3 = 32;
int temperature4 = 33;

int led1 = 34;
int led2 = 35;
int led3 = 36;
int led4 = 37;

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

  if (!SD.exists("index.html")) {
    Serial.println(F("Can't find index.html file!"));
    return;
  }

  Serial.println(F("Found index.html file."));

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print(F("Server is at "));
  Serial.println(Ethernet.localIP());
  Ethernet.begin(mac, ip);
  server.begin();

  initializationPins();
  initializationTemps();
  runAlarmFirstTime();
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

void initializationTemps() {
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
}

void runAlarmFirstTime() {
  digitalWrite(fireAlarm, HIGH);
  delay(1000);
  digitalWrite(fireAlarm, LOW);
  delay(1000);
  digitalWrite(fireAlarm, HIGH);
  delay(1000);
  digitalWrite(fireAlarm, LOW);
}

void loop() {
  runServer();

  checkGas();

  temperatures();
}

void runServer() {
  EthernetClient client = server.available();

  if (client)
  {
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (requestIndex < (REQ_BUF_SZ - 1))
        {
          HTTP_req[requestIndex] = c;
          requestIndex++;
        }

        if (c == '\n' && currentLineIsBlank)
        {
          client.println(F("HTTP/1.1 200 OK"));

          if (strContains(HTTP_req, "ajax_inputs"))
          {
            client.println(F("Content-Type: text/xml"));
            client.println(F("Connection: keep-alive"));
            client.println();
            setLEDs();
            setDoor();
            response(client);
          }
          else
          {
            client.println(F("Content-Type: text/html"));
            client.println(F("Connection: keep-alive"));
            client.println();
            webFile = SD.open("index.html");
            if (webFile)
            {
              while (webFile.available())
              {
                client.write(webFile.read());
              }
              webFile.close();
            }
          }
          Serial.print(HTTP_req);
          requestIndex = 0;
          strClear(HTTP_req, REQ_BUF_SZ);
          break;
        }

        if (c == '\n')
        {
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void checkGas() {
  if (analogRead(gas) > 100) {
    digitalWrite(fan, HIGH);
    digitalWrite(fireAlarm, HIGH);
    delay(1000);
    digitalWrite(fireAlarm, LOW);
    delay(1000);
    digitalWrite(fireAlarm, HIGH);
    delay(1000);
    digitalWrite(fireAlarm, LOW);
    delay(1000);
    digitalWrite(fireAlarm, HIGH);
  } else {
    digitalWrite(fan, LOW);
    digitalWrite(fireAlarm, LOW);
  }
}

void temperatures() {
  float temperature1 = dht1.readTemperature();
  float temperature2 = dht2.readTemperature();
  float temperature3 = dht3.readTemperature();
  float temperature4 = dht4.readTemperature();

  if (temperature3 > 33 || temperature4 > 33 || temperature2 > 33 || temperature1 > 33) {
    digitalWrite(fan, HIGH);
    digitalWrite(fireAlarm, HIGH);
    delay(1000);
    digitalWrite(fireAlarm, LOW);
    delay(1000);
    digitalWrite(fireAlarm, HIGH);
    delay(1000);
    digitalWrite(fireAlarm, LOW);
    delay(1000);
    digitalWrite(fireAlarm, HIGH);
    digitalWrite(waterPump, HIGH);
  }
}

void setLEDs() {
  // LED 1 (pin 34)
  if (strContains(HTTP_req, "LED1=1")) {
    ledState[0] = 1;
    digitalWrite(led1, HIGH);
  }

  if (strContains(HTTP_req, "LED1=0")) {
    ledState[0] = 0;
    digitalWrite(led1, LOW);
  }

  // LED 2 (pin 35)
  if (strContains(HTTP_req, "LED2=1")) {
    ledState[1] = 1;
    digitalWrite(led2, HIGH);
  }

  if (strContains(HTTP_req, "LED2=0")) {
    ledState[1] = 0;
    digitalWrite(led2, LOW);
  }

  // LED 3 (pin 36)
  if (strContains(HTTP_req, "LED3=1")) {
    ledState[2] = 1;
    digitalWrite(led3, HIGH);
  }

  if (strContains(HTTP_req, "LED3=0")) {
    ledState[2] = 0;
    digitalWrite(led3, LOW);
  }

  // LED 4 (pin 37)
  if (strContains(HTTP_req, "LED4=1")) {
    ledState[3] = 1;
    digitalWrite(led4, HIGH);
  }

  if (strContains(HTTP_req, "LED4=0")) {
    ledState[3] = 0;
    digitalWrite(led4, LOW);
  }

  // fan (pin 38)
  if (strContains(HTTP_req, "LED5=1")) {
    ledState[4] = 1;
    digitalWrite(fan, HIGH);
  }

  if (strContains(HTTP_req, "LED5=0")) {
    ledState[4] = 0;
    digitalWrite(fan, LOW);
  }
}

void setDoor() {
  if (strContains(HTTP_req, "DOOR=1")) {
    for (posServo = 180; posServo >= 0; posServo -= 5) {
      servo.write(posServo);
    }
  }

  if (strContains(HTTP_req, "DOOR=0")) {
    for (posServo = 0; posServo <= 100; posServo += 5) {
      servo.write(posServo);
    }
  }
}

void response(EthernetClient cl) {
  cl.print(F("<?xml version = \"1.0\" ?>"));
  cl.print(F("<inputs>"));

  // Iterate over the LED states array
  for (int i = 0; i < 5; i++) {
    cl.print(F("<LED>"));
    cl.print(ledState[i]);
    cl.println(F("</LED>"));
  }

  // Temp 1
  cl.print("<Temp1>");
  cl.print(dht1.readTemperature());
  cl.print("</Temp1>");

  // Temp 2
  cl.print("<Temp2>");
  cl.print(dht2.readTemperature());
  cl.print("</Temp2>");

  // Temp 3
  cl.print("<Temp3>");
  cl.print(dht3.readTemperature());
  cl.print("</Temp3>");

  // Temp 4
  cl.print("<Temp4>");
  cl.print(dht4.readTemperature());
  cl.print("</Temp4>");

  cl.print(F("</inputs>"));
}

void strClear(char *str, char length) {
  for (int i = 0; i < length; i++)
  {
    str[i] = 0;
  }
}

char strContains(char *str, char *sfind) {
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len)
  {
    return 0;
  }
  while (index < len)
  {
    if (str[index] == sfind[found])
    {
      found++;
      if (strlen(sfind) == found)
      {
        return 1;
      }
    }
    else
    {
      found = 0;
    }
    index++;
  }
  return 0;
}