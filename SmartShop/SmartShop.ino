#include <ESP8266WiFi.h>
#define RED D8
#define GREEN D6
#define TRIG_IN D0
#define ECHO_IN D2
#define TRIG_OUT D1
#define ECHO_OUT D3
#define MAX_DISTANCE 150    // Maximum distance (in cm) for the sensors to try to read.
#define DEFAULT_DISTANCE 45 // Default distance (in cm) is only used if calibration fails.
#define MIN_DISTANCE 15     // Minimum distance (in cm) for calibrated threshold.

bool prev_inblocked = false, prev_outblocked = false; //These booleans record whether the entry/exit was blocked on the previous reading of the sensor.
float calibrate_in = 0, calibrate_out = 0;            // The calibration in the setup() function will set these to appropriate values.
long duration_in, duration_out;
int distance_in, distance_out, count;
int iterations = 5;
const char *ssid = "FPT_Thanh Huy";
const char *password = "0907579269";

const char *host = "192.168.1.235";

const String url = "/";
int limit = 10;

void setup()
{
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(TRIG_IN, OUTPUT);  // Sets the trigPin as an Output
  pinMode(ECHO_IN, INPUT);   // Sets the echoPin as an Input
  pinMode(TRIG_OUT, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_OUT, INPUT);  // Sets the echoPin as an Input
  Serial.begin(9600);        // Starts the serial communication
  Serial.println("Calibrating...");

  //IN
  digitalWrite(TRIG_IN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_IN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_IN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration_in = pulseIn(ECHO_IN, HIGH);

  // Calculating the distance
  distance_in = duration_in * 0.034 / 2;
  delay(40); // Wait 40 milliseconds between pings. 29ms should be the shortest delay between pings.digitalWrite(TRIG_IN, LOW);

  //OUT
  digitalWrite(TRIG_OUT, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_OUT, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_OUT, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration_out = pulseIn(ECHO_OUT, HIGH);

  // Calculating the distance
  distance_out = duration_out * 0.034 / 2;
  delay(40);
  delay(1500);
  for (int a = 0; a < iterations; a++)
  {
    delay(50);
    calibrate_in += distance_in;
    delay(50);
    calibrate_out += distance_in;
    delay(200);
  }
  calibrate_in = 0.75 * calibrate_in / iterations; //The threshold is set at 75% of the average of these readings. This should prevent the system counting people if it is knocked.
  calibrate_out = 0.75 * calibrate_out / iterations;

  if (calibrate_in > MAX_DISTANCE || calibrate_in < MIN_DISTANCE)
  { //If the calibration gave a reading outside of sensible bounds, then the default is used
    calibrate_in = DEFAULT_DISTANCE;
  }
  if (calibrate_out > MAX_DISTANCE || calibrate_out < MIN_DISTANCE)
  {
    calibrate_out = DEFAULT_DISTANCE;
  }

  Serial.print("Entry threshold set to: ");
  Serial.println(calibrate_in);
  Serial.print("Exit threshold set to: ");
  Serial.println(calibrate_out);
}

// the loop function runs over and over again forever
void loop()
{

  //IN
  digitalWrite(TRIG_IN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_IN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_IN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration_in = pulseIn(ECHO_IN, HIGH);

  // Calculating the distance
  distance_in = duration_in * 0.034 / 2;
  delay(40); // Wait 40 milliseconds between pings. 29ms should be the shortest delay between pings.digitalWrite(TRIG_IN, LOW);

  //OUT
  digitalWrite(TRIG_OUT, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_OUT, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_OUT, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration_out = pulseIn(ECHO_OUT, HIGH);

  // Calculating the distance
  distance_out = duration_out * 0.034 / 2;
  delay(40);

  if (distance_in < calibrate_in && distance_in > 0)
  { // If closer than wall/calibrated object (person is present) && throw out zero readings
    if (prev_inblocked == false)
    {
      count++; // Increase count by one
      Serial.print("\nCount: ");
      Serial.println(count);
      WiFiClient client;
      const int httpPort = 8081;
      if (!client.connect(host, httpPort))
      {
        Serial.println("connection failed");
        return;
      }

      // We now create a URI for the request
      String url = "/";

      Serial.print("Requesting URL: ");
      Serial.println(url);
      String data = "type=in&count="+String(count);

      Serial.print("Requesting POST: ");
      // Send request to the server:
      client.println("POST / HTTP/1.1");
      client.println("Host: server_name");
      client.println("Accept: */*");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println();
      client.print(data);
      // This will send the request to the server
      /*this is a get method working
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
           "Connection: close\r\n\r\n");*/
      unsigned long timeout = millis();
      while (client.available() == 0)
      {
        if (millis() - timeout > 5000)
        {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
        }
      }
      prev_inblocked = true;
    }
  }
  else
  {
    prev_inblocked = false;
  }
  if (distance_out < calibrate_out && distance_out > 0)
  {
    if (!prev_outblocked)
    {
      count--; // Decrease count by one
      Serial.print("\nCount: ");
      Serial.println(count);
      WiFiClient client;
      const int httpPort = 8081;
      if (!client.connect(host, httpPort))
      {
        Serial.println("connection failed");
        return;
      }

      // We now create a URI for the request
      String url = "/";

      Serial.print("Requesting URL: ");
      Serial.println(url);
      String data = "type=out&count="+String(count);

      Serial.print("Requesting POST: ");
      // Send request to the server:
      client.println("POST / HTTP/1.1");
      client.println("Host: server_name");
      client.println("Accept: */*");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println();
      client.print(data);
      // This will send the request to the server
      /*this is a get method working
          client.print(String("GET ") + url + " HTTP/1.1\r\n" +
           "Connection: close\r\n\r\n");*/
      unsigned long timeout = millis();
      while (client.available() == 0)
      {
        if (millis() - timeout > 5000)
        {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
        }
      }
      prev_outblocked = true;
    }
  }
  else
  {
    prev_outblocked = false;
  }

  if (count<=limit)
  {
    digitalWrite(GREEN, HIGH);
  }
  else
  {
    digitalWrite(GREEN, LOW);
  }
  if (count>limit)
  {
    digitalWrite(RED, HIGH);
  }
  else
  {
    digitalWrite(RED, LOW);
  }
}
