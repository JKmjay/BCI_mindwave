////////////////////////////////////////////////////////////////////////
// Arduino Bluetooth Interface with Brainsense
// 
// This is example code provided by Pantech Prolabs. and is provided
// license free.
////////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
WiFiServer server(80);
IPAddress IP(192,168,4,15);
IPAddress mask = (255, 255, 255, 0);

byte ledPin = 2;


#define BAUDRATE 57600
#define DEBUGOUTPUT 0

long temp=0 ;
// checksum variables
byte generatedChecksum = 0;
byte checksum = 0; 
int payloadLength = 0;
byte payloadData[64] = {0};
byte poorQuality = 0;
byte attention = 0;
byte meditation = 0;

// system variables

long lastReceivedPacket = 0;
boolean bigPacket = false;

//////////////////////////
// Microprocessor Setup //
//////////////////////////
void setup() 

{
  Serial.begin(BAUDRATE);           // USB
WiFi.mode(WIFI_AP);
  WiFi.softAP("ThinkMe","ThinkMe123");
  WiFi.softAPConfig(IP, IP, mask);
  server.begin();
  pinMode(ledPin, OUTPUT);
  Serial.println();
 
  Serial.println("Server started.");
  Serial.print("IP: ");    Serial.println(WiFi.softAPIP());
  Serial.print("MAC:");    Serial.println(WiFi.softAPmacAddress());



}

////////////////////////////////
// Read data from Serial UART //
////////////////////////////////
byte ReadOneByte() 

{
  int ByteRead;
  while(!Serial.available());
  ByteRead = Serial.read();

#if DEBUGOUTPUT  
  Serial.print((char)ByteRead);   // echo the same byte out the USB serial (for debug purposes)
#endif

  return ByteRead;
}

/////////////
//MAIN LOOP//
/////////////
void loop() 

{
  // Look for sync bytes
  if(ReadOneByte() == 170) 
  {
    if(ReadOneByte() == 170) 
    {
        payloadLength = ReadOneByte();
      
        if(payloadLength > 169)                      //Payload length can not be greater than 169
        return;
        generatedChecksum = 0;        
        for(int i = 0; i < payloadLength; i++) 
        {  
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
        }   

        checksum = ReadOneByte();                      //Read checksum byte from stream      
        generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

        if(checksum == generatedChecksum) 
        {    
          poorQuality = 200;
          attention = 0;
          meditation = 0;

          for(int i = 0; i < payloadLength; i++) 
          {                                          // Parse the payload
          switch (payloadData[i]) 
          {
          case 2:
            i++;            
            poorQuality = payloadData[i];
            bigPacket = true;            
            break;
          case 4:
            i++;
            attention = payloadData[i];                        
            break;
          case 5:
            i++;
            meditation = payloadData[i];
            break;
          case 0x80:
            i = i + 3;
            break;
          case 0x83:
            i = i + 25;      
            break;
          default:
            break;
          } // switch
        } // for loop

#if !DEBUGOUTPUT

        // *** Add your code here ***




        if(bigPacket) 
        { 

          data_send();
          Serial.print("PoorQuality: ");
          Serial.print(poorQuality, DEC);
          Serial.print(" Attention: ");
          Serial.print(attention, DEC);
          Serial.print("meditation : ");
          Serial.print(meditation, DEC);
          Serial.print(" Time since last packet: ");
          Serial.print(millis() - lastReceivedPacket, DEC);
          lastReceivedPacket = millis();
          Serial.print("\n");

                 
        }
#endif        
        bigPacket = false;        
      }
      else {
        // Checksum Error
      }  // end if else for checksum
    } // end if read 0xAA byte
  } // end if read 0xAA byte
}


void data_send()
{
  String request;
 WiFiClient client = server.available();
  if (!client) {return;}
  digitalWrite(ledPin, LOW);
  if(attention>=50)
   request="HIGH";
 else if(attention<30)
   request="LOW";
  else
  request="MEDIUM";
    
    Serial.println("********************************");
     //Serial.println("From the station: " + request);
      client.flush();
  Serial.print("Byte sent to the station: ");
  Serial.println(client.println(request + "" + "\r"));
  digitalWrite(ledPin, HIGH);

}



