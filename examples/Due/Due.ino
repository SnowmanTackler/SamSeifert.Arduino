// This sketch is ONLY for the Arduino Due!
// You should make the following connections with the Due and GPS module:
// GPS power pin to Arduino Due 3.3V output.
// GPS ground pin to Arduino Due ground.
// For hardware serial 1 (recommended):
//   GPS TX to Arduino Due Serial1 RX pin 19
//   GPS RX to Arduino Due Serial1 TX pin 18

#include <SamPMTK.h>

SamPMTK pmtk1;

void setup()
{

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Setting Correct Baud...");

  pmtk1.Attach(Serial1);

  pmtk1.SetUpdateRate(200);
  //SendCommand(&SerialGPS, "PMTK605"); // Ask for firmware version
  pmtk1.Subscribe(1, 0, 0, 0, 0, 0); // Get GLL
  pmtk1.SubscribeAntenna(false);
 
  Serial.println("Starting!");
}

void loop()                     // run over and over again
{
  pmtk1.CheckMessages(MessageReceived);
}

void MessageReceived(byte message_length)
{
  Serial.println(pmtk1.RecieveString());
}

