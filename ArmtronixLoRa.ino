/*
  Copyright ® 2019 December devMobile Software, All Rights Reserved

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  You can do what you want with this code, acknowledgment would be nice.

  http://www.devmobile.co.nz

*/
#include <stdlib.h>
#include <LoRa.h>
#include <TH02_dev.h>

#define DEBUG
#define DEBUG_TELEMETRY
#define DEBUG_LORA

// LoRa field gateway configuration (these settings must match your field gateway)
const char FieldGatewayAddress[] = {"LoRaIoT1"};
const char DeviceAddress[] = {"ArmTronix01"};
const float FieldGatewayFrequency =  915000000.0;
const byte FieldGatewaySyncWord = 0x12 ;

// Payload configuration
const int ChipSelectPin = PA4;
const int InterruptPin = PA11;
const int ResetPin = PC13;

// LoRa radio payload configuration
const byte SensorIdValueSeperator = ' ' ;
const byte SensorReadingSeperator = ',' ;
const int LoopSleepDelaySeconds = 30 ;

const byte PayloadSizeMaximum = 64 ;
byte payload[PayloadSizeMaximum];
byte payloadLength = 0 ;


void setup()
{
  Serial.begin(9600);
#ifdef DEBUG
  while (!Serial);
#endif
  Serial.println("Setup called");

  Serial.println("LoRa setup start");
  
  // override the default chip select and reset pins
  LoRa.setPins(ChipSelectPin, ResetPin, InterruptPin); 
  if (!LoRa.begin(FieldGatewayFrequency))
  {
    Serial.println("LoRa begin failed");
    while (true); // Drop into endless loop requiring restart
  }

  // Need to do this so field gateways pays attention to messsages from this device
  LoRa.enableCrc();
  LoRa.setSyncWord(FieldGatewaySyncWord);

#ifdef DEBUG_LORA
  LoRa.dumpRegisters(Serial);
#endif
  Serial.println("LoRa Setup done.");

  // Configure the Seeedstudio TH02 temperature & humidity sensor
  Serial.println("TH02 setup start");
  TH02.begin();
  delay(100);
  Serial.println("TH02 setup done");

  PayloadHeader((byte*)FieldGatewayAddress,strlen(FieldGatewayAddress), (byte*)DeviceAddress, strlen(DeviceAddress));

  Serial.println("Setup done");
  Serial.println();
}


void loop()
{
  float temperature ;
  float humidity ;

  Serial.println("Loop called");

  PayloadReset();

  // Read the temperature & humidity & battery voltage values then display nicely
  temperature = TH02.ReadTemperature();
  Serial.print("T:");
  Serial.print( temperature, 1 ) ;
  Serial.println( "C " ) ;

  PayloadAdd( "T", temperature, 1);

  humidity = TH02.ReadHumidity();
  Serial.print("H:");
  Serial.print( humidity, 0 ) ;
  Serial.println( "% " ) ;

  PayloadAdd( "H", humidity, 0) ;

#ifdef DEBUG_TELEMETRY
  Serial.print( "RFM9X/SX127X Payload len:");
  Serial.print( payloadLength );
  Serial.println( " bytes" );
#endif

  LoRa.beginPacket();
  LoRa.write( payload, payloadLength );
  LoRa.endPacket();

  Serial.println("Loop done");
  Serial.println();
  delay(LoopSleepDelaySeconds * 1000l);
}


void PayloadHeader( byte *to, byte toAddressLength, byte *from, byte fromAddressLength)
{
  byte addressesLength = toAddressLength + fromAddressLength ;

#ifdef DEBUG_TELEMETRY
  Serial.print("PayloadHeader- ");
  Serial.print( "To len:");
  Serial.print( toAddressLength );
  Serial.print( " From len:");
  Serial.print( fromAddressLength );
  Serial.print( " Header len:");
  Serial.print( addressesLength );
  Serial.println( );
#endif

  payloadLength = 0 ;

  // prepare the payload header with "To" Address length (top nibble) and "From" address length (bottom nibble)
  payload[payloadLength] = (toAddressLength << 4) | fromAddressLength ;
  payloadLength += 1;

  // Copy the "To" address into payload
  memcpy(&payload[payloadLength], to, toAddressLength);
  payloadLength += toAddressLength ;

  // Copy the "From" into payload
  memcpy(&payload[payloadLength], from, fromAddressLength);
  payloadLength += fromAddressLength ;
}


void PayloadAdd( const char *sensorId, float value, byte decimalPlaces)
{
  byte sensorIdLength = strlen( sensorId ) ;

#ifdef DEBUG_TELEMETRY
  Serial.print("PayloadAdd-float ");
  Serial.print( "SensorId:");
  Serial.print( sensorId );
  Serial.print( " Len:");
  Serial.print( sensorIdLength );
  Serial.print( " Value:");
  Serial.print( value, decimalPlaces );
  Serial.print( " payloadLen:");
  Serial.print( payloadLength);
#endif

  memcpy( &payload[payloadLength], sensorId,  sensorIdLength) ;
  payloadLength += sensorIdLength ;
  payload[ payloadLength] = SensorIdValueSeperator;
  payloadLength += 1 ;
  payloadLength += strlen( dtostrf(value, -1, decimalPlaces, (char *)&payload[payloadLength]));
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += 1 ;
  
#ifdef DEBUG_TELEMETRY
  Serial.print( " payloadLen:");
  Serial.println( payloadLength);
#endif
}


void PayloadAdd( const char *sensorId, int value )
{
  byte sensorIdLength = strlen( sensorId ) ;

#ifdef DEBUG_TELEMETRY
  Serial.print("PayloadAdd-int ");
  Serial.print( "SensorId:");
  Serial.print( sensorId );
  Serial.print( " Len:");
  Serial.print( sensorIdLength );
  Serial.print( " Value:");
  Serial.print( value );
  Serial.print( " payloadLen:");
  Serial.print( payloadLength);
#endif  

  memcpy( &payload[payloadLength], sensorId,  sensorIdLength) ;
  payloadLength += sensorIdLength ;
  payload[ payloadLength] = SensorIdValueSeperator;
  payloadLength += 1 ;
  payloadLength += strlen( itoa( value,(char *)&payload[payloadLength],10));
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += 1 ;
  
#ifdef DEBUG_TELEMETRY
  Serial.print( " payloadLen:");
  Serial.println( payloadLength);
#endif
}


void PayloadAdd( const char *sensorId, unsigned int value )
{
  byte sensorIdLength = strlen( sensorId ) ;

#ifdef DEBUG_TELEMETRY
  Serial.print("PayloadAdd-unsigned int ");
  Serial.print( "SensorId:");
  Serial.print( sensorId );
  Serial.print( " Len:");
  Serial.print( sensorIdLength );
  Serial.print( " Value:");
  Serial.print( value );
  Serial.print( " payloadLen:");
  Serial.print( payloadLength);
#endif  

  memcpy( &payload[payloadLength], sensorId,  sensorIdLength) ;
  payloadLength += sensorIdLength ;
  payload[ payloadLength] = SensorIdValueSeperator;
  payloadLength += 1 ;
  payloadLength += strlen( utoa( value,(char *)&payload[payloadLength],10));
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += 1 ;

#ifdef DEBUG_TELEMETRY
  Serial.print( " payloadLen:");
  Serial.println( payloadLength);
#endif
}


void PayloadReset()
{
  byte fromAddressLength = payload[0] & 0xf ;
  byte toAddressLength = payload[0] >> 4 ;
  byte addressesLength = toAddressLength + fromAddressLength ;

  payloadLength = addressesLength + 1;

#ifdef DEBUG_TELEMETRY
  Serial.print("PayloadReset- ");
  Serial.print( "To len:");
  Serial.print( toAddressLength );
  Serial.print( " From len:");
  Serial.print( fromAddressLength );
  Serial.print( " Header len:");
  Serial.println( addressesLength );
#endif
}
