#include "XBee.h"
#include "queue.h"
#include <SoftwareSerial.h>
#include "DHT.h"          // DHT & AM2302 library
#define DHTPIN 2         // Data pin connected to AM2302
#define DHTTYPE DHT22       // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);   // LED pins

const int moistureAO = 0;    // select the input pin for the potentiometer
const int sensorValue = 0;  // variable to store the value coming from the sensor
const int moistureDO = 11; // digital bit of moisture sensor
const int valve = 10;

int soil = 0;
int AO = 0;
int DO = 0;
int tmp = 0;
int soill = 0;
String moisture, humidity, temperature;
unsigned int s1, s2, s3; // size of moisture, humidity, temperatur
XBee xbee;
Queue RxQ;
SoftwareSerial sserial(12,13);

// we are going to send two floats of 4 bytes each
uint8_t payload[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// union to convery float to byte string
union u_tag {
    uint8_t b[4];
    float fval;
} u;


void setup(void)
{
    sserial.begin(9600);
    pinMode(moistureAO, INPUT);
   pinMode(moistureDO, INPUT);
   pinMode(valve, OUTPUT);
   dht.begin();
   digitalWrite(valve,HIGH);
}

void loop(void)
{
// read the value from the sensor:
   tmp = analogRead(moistureAO); 
   soill = tmp;
    if ( tmp != AO ) 
    {
    AO=tmp;
    }
delay(10);
tmp=digitalRead( moistureDO );
    if ( tmp != DO ) 
    {
    DO=tmp;
    }               
delay(10);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  
  float t = dht.readTemperature();
  
  
  delay(40);
  delay(5);

 if(soill<220)
  {  
   digitalWrite(valve,HIGH);
   }

   else
   {
    if(soill>300)
       {  
          digitalWrite(valve,LOW);
       }

     else
       {
      {digitalWrite(valve,LOW);
        
       }}  
   }
  
  
  
  int queueLen = 0;
  int delPos = 0;

 // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (!isnan(t) && !isnan(h)) {
     
    // convert humidity into a byte array and copy it into the payload array
    u.fval = h;
    for (int i=0;i<4;i++){
      payload[i]=u.b[i];
    }
     
    // same for the temperature
    u.fval = t;
    for (int i=0;i<4;i++){
      payload[i+4]=u.b[i];
    }
// same for the moisture
    u.fval = soill;
    for (int i=0;i<4;i++){
      payload[i+8]=u.b[i];
    }
  }
   while (sserial.available() > 0){
        unsigned char in = (unsigned char)sserial.read();
        if (!RxQ.Enqueue(in)){
            break;
        }
    }
    queueLen = RxQ.Size();
    for (int i=0;i<queueLen;i++){
        if (RxQ.Peek(i) == 0x7E){
            unsigned char checkBuff[Q_SIZE];
            unsigned char msgBuff[Q_SIZE];
            int checkLen = 0;
            int msgLen = 0;

            checkLen = RxQ.Copy(checkBuff, i);
            
            msgLen = xbee.Receive(checkBuff, checkLen, msgBuff);
            if (msgLen > 0){
                unsigned char outMsg[Q_SIZE];
                unsigned char outFrame[Q_SIZE];
                int frameLen = 0;
                int addr = ((int)msgBuff[4] << 8) + (int)msgBuff[5];

                // 10 is length of "you sent: "
             // unsigned int siz = s1+s2+s3;
              //String newstring = moisture+humidity+temperature;
                memcpy(&outMsg,&payload,12);
                // len - (9 bytes of frame not in message content)
                memcpy(&outMsg[12], &msgBuff[8], msgLen-9);
                // 10 + (-9) = 1 more byte in new content than in previous message
                frameLen = xbee.Send(outMsg, msgLen+3, outFrame, addr);
                sserial.write(outFrame, frameLen);
                i += msgLen;
                delPos = i;    
            }else{
                if (i>0){
                    delPos = i-1;
                }
            }
        }
    }

    RxQ.Clear(delPos);
}
