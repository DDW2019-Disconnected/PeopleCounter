#include <HttpClient.h>
#include <WiFi.h>

const unsigned int TRIG_PIN = 5;
const unsigned int ECHO_PIN = 6;
const unsigned int BAUD_RATE = 9600;

byte myMacAddress[] = { 0xDE, 0xAF, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress server(74,125,232,128);
//IPAddress googleDNS(8,8,4,4);
char serviceURL[] = "www.us-central1-school-230709.cloudfunctions.net"; 
WifiClient client;

unsigned long lastConnectionTime = 0;           // last time the server was updated in ms
const unsigned long postingInterval = 10*1000;  // delay between updates, in ms

const unsigned long walkingBufferTime = 500;  // buffer between walkers, in ms
unsigned long lastWalkerTime = 0;       // last walker, in ms

const unsigned int startRangeNobodyWalking = 100;
const unsigned int endRangeNobodyWalking = 2000;
unsigned int amountOfPeopleBuffered = 10; 
unsigned int amountOfAttempts = 0;

void setup() {
    //sensor
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    //networking
//    pinMode(13, OUTPUT);
//    digitalWrite(13, LOW);

      Serial.begin(BAUD_RATE);

 // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (WiFi.begin(myMacAddress) == 0) {
    Serial.println("Failed to configure Wifi using DHCP");
    if (WiFi.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
    while(!Serial);
}

void loop() {
    Serial.print("distance to nearest object:");
    const unsigned int distance = getDistance();
    Serial.println(distance);
    Serial.print(" cm");

    if (millis() - lastWalkerTime > walkingBufferTime && (distance < startRangeNobodyWalking || distance > endRangeNobodyWalking)) {
      
      if (amountOfAttempts == 4) { 
         lastWalkerTime = millis();
         amountOfPeopleBuffered++;
         amountOfAttempts = 0;
      }

      amountOfAttempts++;
    } else {
      amountOfAttempts = 0;
    }

      // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
//  if (client.available()) {
//    char c = client.read();
//    Serial.write(c);
//  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval && amountOfPeopleBuffered > 0) {
    while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

    httpRequest(amountOfPeopleBuffered);
  }
  

    delay(40);
}

void cleanPulses() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
}

int convertEchoDurationToDistance(long echoDuration) {
    return echoDuration / 29 / 2;
}

long findEchoDuration() {
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    return pulseIn(ECHO_PIN, HIGH);
}

void readWithDifferentSensor() {
    float sum = 0;
   for (int i=0; i<tryCount; i++)
 {
   sum += analogRead(IRpin);
 }
  float volts = (sum / tryCount) *0.0048828125;   // value from sensor * (5/1024) - if running 3.3.volts then change 5 to 3.3
  float distance = 65*pow(volts, -1.10);          // worked out from graph 65 = theretical distance / (1/Volts)S - luckylarry.co.uk
  Serial.println(distance);                       // print the distance
  delay(100);
}

int getDistance() {
    cleanPulses();
    const unsigned long echoDuration = findEchoDuration();
    int distanceFromSensor = convertEchoDurationToDistance(echoDuration);

    return distanceFromSensor;
}

// this method makes a HTTP connection to the server:
void httpRequest(int amountOfPeople) {
  String bodyText = String(amountOfPeople);
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(serviceURL, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:
    client.println("POST /addPeople HTTP/1.1");
    client.println("Host: www.us-central1-school-230709.cloudfunctions.net");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println("Content-type: text/plain");
    client.println("ArduinoID: A");
    client.print("Content-length: ");
    client.println(bodyText.length());
    client.println();
    client.println(bodyText);


    // note the time that the connection was made:
    lastConnectionTime = millis();
    amountOfPeopleBuffered = 0;
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
