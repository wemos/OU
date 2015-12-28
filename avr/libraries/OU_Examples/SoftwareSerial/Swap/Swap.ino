/*
  Test Serial.swap();
*/
#include <SoftwareSerial.h>

SoftwareSerial mySerial(SRX, STX); // RX, TX

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.swap();//Connect CH340G's RXD/TXD to STX/SRX, and the OU's RX/TX are float.
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(57600);
  mySerial.println("Hello, I'm SSerail.");
}

void loop() 
{
  if (mySerial.available())
    mySerial.write(mySerial.read());

}

