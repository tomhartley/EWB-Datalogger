#include <SoftwareSerial.h>//rx and tx pin
#include <SPI.h>
#include <SD.h>
#include <String.h>

SoftwareSerial RadioSerial(2, 3); //RX,TX

SoftwareSerial GSMSerial(7, 8);

String baseString = "AT+HTTPPARA=\"URL\",api.thingspeak.com/update.json?api_key=";
String key1 = "3YRDENGBTPMM67Y7";
String key2 = "NQS7Y4I947PW612T";
String key3 = "ANGZAZB1VEZO52YC";

int succ = 0; //-1 = fail, 1 = succ, 0 = ??

int redLED = A5;
int greenLED = A4;

unsigned long long lastUpload = 0;

char emptyURL[256] = {0};
int URLPos = 0;
//unsigned long PrevMillis;
void RadioRead(char data[40]) //FUNCTION
{
  RadioSerial.listen();
  if (RadioSerial.peek() == 0xEB) {
    int arrNum = 0;
    while (RadioSerial.available())
    {
      data[arrNum] = (char) RadioSerial.read();
      delay(1);
      arrNum++;
    }
  }
  return;
}

void initURL(int channelNo) {
  URLPos = 0;
  for (int i = 0; i < sizeof(emptyURL); i++) {
    emptyURL[i] = 0;
  }
  int baseLength = baseString.length();
  for (int i = 0; i < baseLength; i++) {
    emptyURL[i] = baseString[i];
  }
  URLPos += baseLength;
  if (channelNo == 1) {
    int baseLength = key1.length();
    for (int i = 0; i < baseLength; i++) {
      emptyURL[i + URLPos] = key1[i];
    }
    URLPos += baseLength;
  } else if (channelNo == 2) {
    int baseLength = key2.length();
    for (int i = 0; i < baseLength; i++) {
      emptyURL[i + URLPos] = key2[i];
    }
    URLPos += baseLength;
  } else if (channelNo == 3) {
    int baseLength = key2.length();
    for (int i = 0; i < baseLength; i++) {
      emptyURL[i + URLPos] = key3[i];
    }
    URLPos += baseLength;
  }

}

void appendString(String str) {
  int strLength = str.length();
  for (int i = 0; i < strLength; i++) {
    emptyURL[i + URLPos] = str[i];
  }
  URLPos = URLPos + strLength;
}



void InitializeSDCard() {

  Serial.print(F("Initializing SD card..."));

  while (!SD.begin(4)) {
    Serial.println(F("Failed!"));
    //return;
  }
  Serial.println(F("Done."));
  return;

}
void SDWrite(String *FileName, String *dataString) {
  File dataFile = SD.open(*FileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(*dataString);
    dataFile.close();
    //Serial.print(*dataString);
    // print to the serial port too:
  }
}
void SDWriteln(String (*dataString)) {
  File dataFile = SD.open("Fail.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(*dataString);
    dataFile.close();
    // print to the serial port too:
    //Serial.println(*dataString);
  }
}
//void SDWriteHead(String *FileName) {
//  File dataFile = SD.open(*FileName, FILE_WRITE);
//
//  // if the file is available, write to it:
//  if (dataFile) {
//    dataFile.println("Date,Time,Voltage(V),Current(I),Temperature,Water Level(m)");
//    dataFile.close();
//    // print to the serial port too:
//  }
//}
void ShowSerialData()
{
  while (GSMSerial.available() != 0)
    Serial.write(GSMSerial.read());
}
void powerUp() {
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(1000);
  digitalWrite(9, HIGH);
  delay(1500);
  digitalWrite(9, LOW);
  delay(3000);
}

void GSMSetup() {
  bool powered = false;
  //Serial.println("Oi");
  while (powered == false) {

    powerUp();
    GSMSerial.begin(9600);
    delay(500);

    GSMSerial.println(F("AT+CLTS=1"));
    delay(4000);
    ShowSerialData();


    GSMSerial.println(F("AT+CGATT=1"));
    delay(1000);
    ShowSerialData();

    GSMSerial.println(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""));//setting the SAPBR, the connection type is using gprs
    delay(3000);

    String ACTreply = GSMSerial.readString();
    Serial.println(ACTreply + "OK!");
    if (ACTreply.indexOf("OK") != -1) {
      //
      Serial.println(ACTreply.indexOf("OK") + " true!");
      powered = true;
    }
    GSMSerial.println(F("AT+SAPBR=3,1,\"APN\",\"internet.globe.com.ph\""));//setting the APN, the second need you fill in your local apn server internet.globe.com.ph 17 in
    delay(4000);
    ShowSerialData();

    GSMSerial.println(F("AT+SAPBR=4,1"));//setting the SAPBR, for detail you can refer to the AT command mamual
    delay(10000);
    ShowSerialData();
    delay(6000);
    GSMSerial.println(F("AT+HTTPINIT")); //init the HTTP request
    delay(2000);
    ShowSerialData();

    /*GSMSerial.println(F("AT+CMGF=1"));
    delay(1000);
    ShowSerialData();

    GSMSerial.println(F("AT+CMGS=\"8888\""));
    delay(1000);
    GSMSerial.println(F("GOSURF STATUS"));
    delay(1000);
    GSMSerial.println((char)26);
    delay(1000);
    ShowSerialData();

    GSMSerial.println(F("AT+CNMI=0,2,0,0,0"));
    delay(5000);
    ShowSerialData();

    GSMSerial.println(F("AT+CMGR=0"));
    delay(5000);
    ShowSerialData();*/
  }
}

int  UploadThingSpeak() {
  //CheckGSMConnection();
  //GSMSerial.println("AT+CSQ"); //get the signal quality
  //delay(100); //need to be long enough so response not to mixed up and read as CGATTreply
  //ShowSerialData();
  GSMSerial.println(F("AT+SAPBR=1,1"));//setting the SAPBR, for detail you can refer to the AT command mamual
  delay(1000);
  ShowSerialData();

  GSMSerial.println(F("AT+SAPBR=2,1"));//setting the SAPBR, for detail you can refer to the AT command mamual
  delay(1000);
  ShowSerialData();
  Serial.println(emptyURL);
  GSMSerial.println(emptyURL);

  delay(5000);
  ShowSerialData();

  GSMSerial.println(F("AT+HTTPACTION=0"));//submit the request
  delay(5000);//the delay is very important, the delay time is base on the return from the website, if the return datas are very large, the time required longer.
  String ACTreply = GSMSerial.readString(); //
  while ((GSMSerial.available() != 0) && (ACTreply.length() < 42))ACTreply += GSMSerial.read();
  Serial.println(ACTreply);


  if (ACTreply.substring(39, 42) != "200") {
    //
    return 0;
  }

  //while(GSMSerial.available());
  //  ShowSerialData();
  ////
  ////  GSMSerial.println("AT+HTTPREAD");// read the data from the website you access
  ////  delay(1000);
  ////  ShowSerialData();
  ////
  ////  GSMSerial.println("");
  ////  delay(100);
  ////  Serial.println("HTTP request finished");
  ////  Serial.println(ACTreply.substring(39, 42));
  return 1;
}

/*
  void ReUpload() {
  String URL = "";
  File dataFile = SD.open("Fail.txt");
  while (dataFile.peek() != -1) {
    while (dataFile.peek() != '\n') {
      URL += char(dataFile.read());
    }

    if (UploadThingSpeak() == 0) { //FIXME
      return;
    }
    else {
      char a = dataFile.read();
      URL = "";
    }
  }
  dataFile.close();
  SD.remove("Fail.txt");
  return;
  }
*/


void setup() {
  // put your setup code here, to run once:
  Serial.begin (9600);
  while (!Serial)
  {
    //wait for serial port to connect
  }

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  Serial.println(F("Start"));
  //GSMSerial.begin(9600);               // the GPRS baud rate
  GSMSetup();
  //PrevMillis = millis();
  RadioSerial.begin(9600);
  InitializeSDCard();
  delay (100);

}

float addFloat2URL(char MSBs, char LSBs, int dataNo) {
  int data = ((unsigned)((byte)MSBs) << 8 | (byte)LSBs);
  float dataFloat = data * 0.1;
  String dataString = String(dataFloat, 1);
  String tempString = String("&" + String(dataNo) + "=" + dataString);
  appendString(tempString);
  return dataFloat;
}

int addInt2URL(char MSBs, char LSBs, int dataNo) {
  int data = ((unsigned)((byte)MSBs) << 8 | (byte)LSBs);
  String tempString = String("&" + String(dataNo) + "=" + String(data));
  appendString(tempString);
  return data;
}

void loop() {
  unsigned long long currentTime = millis();
  if ((currentTime-lastUpload)>(1000*60*3)) {
    succ = -1;
    Serial.println(F("Didn't upload in 8 minutes"));
  }
  
  if (succ == 1) {
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
  } else if (succ == 0) {
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, LOW);
  } else { //succ == -1
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
  }

  initURL(1);
  char data[40] = {0};
  RadioRead(data);

  if ((byte)data[0] == 0xEB) //Call sign or start bit(v for village)
  {
    //for (int i = 13; i<13; i++) {
    //    Serial.print((byte)data[i]);
    //    Serial.print(", ");
    //}
    //Serial.println("");


    GSMSerial.listen();



    int winCount = 0;


    //Channel 1---------------------------------
    initURL(1);
    int avgRmsVoltSq = addInt2URL(data[1], data[2] , 1);
    float avgRmsCurrSq = addFloat2URL(data[3], data[4], 2);
    float avgFreq = addFloat2URL(data[5], data[6], 3);
    int avgPower = addInt2URL(data[7], data[8], 4);
    int avgDepth = addInt2URL(data[9], data[10], 5);
    float avgTemp = addFloat2URL(data[11], data[12], 6);
    //appendString(tempString);
    winCount += UploadThingSpeak(); //1 on success

    initURL(2);
    int maxVrms = addInt2URL(data[13], data[14], 1);
    int minVrms = addInt2URL(data[15], data[16], 2);
    float maxIrms = addFloat2URL(data[17], data[18], 3);
    float minIrms = addFloat2URL(data[18], data[20], 4);
    float maxFreq = addFloat2URL(data[19], data[22], 5);
    float minFreq = addFloat2URL(data[21], data[24], 6);
    int maxPower =  addInt2URL(data[23], data[26], 7);
    int minPower =  addInt2URL(data[25], data[28], 8);
    //appendString(tempString);
    winCount += UploadThingSpeak();

    initURL(3);
    float maxVolt = addFloat2URL(data[27], data[30], 1);
    float minVolt = addFloat2URL(data[29], data[32], 2);
    float maxCurr = addFloat2URL(data[31], data[34], 3);
    float minCurr = addFloat2URL(data[33], data[36], 4);
    //appendString(tempString);
    winCount += UploadThingSpeak();




    //Serial.println(URL);

    //URL.replace(URL.substring(110, 132), Timereply.substring(0, 22));
    //Serial.println(URL);

    if (winCount == 3) {
      //      if (SD.exists("Fail.txt"))
      //        ReUpload();
      Serial.println(F(">>>Uploading Succeeded"));
      succ = 1;
      lastUpload = millis();
    }
    else {
      //SDWriteln(&URL);
      Serial.println(F(">>>Uploading Failed"));
      succ = -1;

    }

    //PrevMillis = millis();
    //Serial.println(PrevMillis);
  }
}



