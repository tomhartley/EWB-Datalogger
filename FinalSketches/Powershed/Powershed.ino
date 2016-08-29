#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

#include <max6675.h>
int thermoMISO = 12;
int thermoSS = 10;
int thermoSCK = 13;

MAX6675 thermocouple(thermoSCK, thermoSS, thermoMISO);

#include <SoftwareSerial.h>//rx and tx pin
SoftwareSerial RadioSerial(2, 3);//RX,TX

int voltPinStatus = 0; //Not sure == no LEDs on
                       //1 = too low
                       //2 = perfect
                       //3 = too high


int voltLowPin = A1;
int voltHighPin = A5;
int voltPerfectPin = A4;

volatile unsigned int curTemp, maxVrms, minVrms, maxIrms, minIrms, minPower, maxPower;
volatile float maxVolt, minVolt, maxCur, minCur, avgTemp, avgdepth, avgFreq, minFreq, maxFreq;
volatile long Cur[161];

//Frequency Measurement Stuff//
volatile bool fFlag = false;
volatile unsigned int fCycles = 0;
float fSecondsTimer;
volatile unsigned long fStartTime = 0;
volatile float freq= 0.0;
//End Freq Measurement//

long prevSecondTimer = 0;
volatile long startTime = 0;

volatile bool count = true;

int depth = 0;

unsigned int curPos = 0;
char mode = 0; //0 is recording current, 1 is processing current, 2 is recording voltage, 3 is processing voltage


long i = 0, j = 0;

//Pin setting
const unsigned char voltPin = 2;                  //Analog Pin 2 for voltage measurement
const unsigned char currentPin = 3;               //Analog Pin 3 for voltage measurement
volatile unsigned char currentAdcChannel;         //temporary storage for the pin number

unsigned const int realCurrRmsSqArray [255] PROGMEM = {22753, 22563, 22373, 22184, 21996, 21902, 21716, 21530, 21344, 21160, 20976, 20794, 20703, 20521, 20340, 20160, 19981, 19803, 19625, 19536, 19360, 19184, 19010, 18836, 18662, 18490, 18404, 18233, 18063, 17893, 17724, 17556, 17389, 17306, 17140,
                                                       16974, 16810, 16646, 16484, 16322, 16241, 16080, 15920, 15761, 15603, 15445, 15288, 15210, 15054, 14900, 14746, 14592, 14440, 14288, 14213, 14063, 13913, 13764, 13616, 13469, 13323, 13250, 13104, 12960, 12816, 12674, 12532, 12390, 12320, 12180, 12041,
                                                       11903, 11765, 11628, 11492, 11424, 11290, 11156, 11022, 10890, 10758, 10628, 10563, 10433, 10304, 10176, 10049, 9923, 9797, 9734, 9610, 9486, 9364, 9242, 9120, 9000, 8940, 8821, 8703, 8585, 8468, 8352, 8237, 8180, 8066, 7952, 7840, 7728, 7618, 7508,
                                                       7453, 7344, 7236, 7129, 7023, 6917, 6812, 6760, 6656, 6554, 6452, 6350, 6250, 6150, 6101, 6003, 5905, 5808, 5712, 5617, 5523, 5476, 5382, 5290, 5198, 5108, 5018, 4928, 4884, 4796, 4709, 4623, 4537, 4452, 4368, 4326, 4244, 4162, 4080, 4000, 3920, 3842,
                                                       3803, 3725, 3648, 3572, 3497, 3423, 3349, 3312, 3240, 3168, 3098, 3028, 2958, 2890, 2856, 2789, 2723, 2657, 2592, 2528, 2465, 2434, 2372, 2310, 2250, 2190, 2132, 2074, 2045, 1988, 1932, 1877, 1823, 1769, 1716, 1690, 1638, 1588, 1538, 1488, 1440, 1392,
                                                       1369, 1323, 1277, 1232, 1188, 1145, 1103, 1082, 1040, 1000, 960, 922, 884, 846, 828, 792, 757, 723, 689, 656, 624, 608, 578, 548, 518, 490, 462, 436, 423, 397, 372, 348, 325, 303, 281, 270, 250, 230, 212, 194, 176, 160, 152, 137, 123, 109, 96, 84, 73,
                                                       68, 58, 48, 40, 32, 26, 20, 17, 12, 8, 5, 3
                                                      };

unsigned const long realVrmsSqArray [301] PROGMEM = {90000,89401,88804,88209,87616,87025,86436,85849,85264,84681,84100,83521,82944,82369,81796,81225,80656,80089,79524,78961,78400,77841,77284,76729,76176,75625,75076,74529,73984,73441,72900,72361,71824,71289,70756,70225,69696,69169,68644,68121,67600,67081,
                                                    66564,66049,65536, 65025, 64516, 64009, 63504, 63001, 62500, 62001, 61504, 61009, 60516, 60025, 59536, 59049, 58564, 58081, 57600, 57121, 56644, 56169, 55696, 55225, 54756, 54289, 53824, 53361,
                                                    52900, 52441, 51984, 51529, 51076, 50625, 50176, 49729, 49284, 48841, 48400, 47961, 47524, 47089, 46656, 46225, 45796, 45369, 44944, 44521, 44100, 43681, 43264, 42849, 42436, 42025, 41616, 41209, 40804, 40401, 40000, 39601, 39204, 38809, 38416, 38025,
                                                    37636, 37249, 36864, 36481, 36100, 35721, 35344, 34969, 34596, 34225, 33856, 33489, 33124, 32761, 32400, 32041, 31684, 31329, 30976, 30625, 30276, 29929, 29584, 29241, 28900, 28561, 28224, 27889, 27556, 27225, 26896, 26569, 26244, 25921, 25600, 25281,
                                                    24964, 24649, 24336, 24025, 23716, 23409, 23104, 22801, 22500, 22201, 21904, 21609, 21316, 21025, 20736, 20449, 20164, 19881, 19600, 19321, 19044, 18769, 18496, 18225, 17956, 17689, 17424, 17161, 16900, 16641, 16384, 16129, 15876, 15625, 15376, 15129,
                                                    14884, 14641, 14400, 14161, 13924, 13689, 13456, 13225, 12996, 12769, 12544, 12321, 12100, 11881, 11664, 11449, 11236, 11025, 10816, 10609, 10404, 10201, 10000, 9801, 9604, 9409, 9216, 9025, 8836, 8649, 8464, 8281, 8100, 7921, 7744, 7569, 7396, 7225, 7056,
                                                    6889, 6724, 6561, 6400, 6241, 6084, 5929, 5776, 5625, 5476, 5329, 5184, 5041, 4900, 4761, 4624, 4489, 4356, 4225, 4096, 3969, 3844, 3721, 3600, 3481, 3364, 3249, 3136, 3025, 2916, 2809, 2704, 2601, 2500, 2401, 2304, 2209, 2116, 2025, 1936, 1849, 1764, 1681, 1600,
                                                    1521, 1444, 1369, 1296, 1225, 1156, 1089, 1024, 961, 900, 841, 784, 729, 676, 625, 576, 529, 484, 441, 400, 361, 324, 289, 256, 225, 196, 169, 144, 121, 100, 81, 64, 49, 36, 25, 16, 9, 4, 1, 0
                                                   };
//Scale
//####### Manual Calibration ####### MUST BE DONE FOR ACCURATE VOLTAGE & CURRENT MEASUREMENT!!
//####### 1) Vm of mains, 2) Vm from ACAC adaptor, 3) Vm from op amp output ########
// scaleFactorSqV EQUATION:
//float scaleFactorSqV = [ACAC adaptor MAX/(op amp output Vm-VDCoffset)]^2 * (Mains MAX/ACAC adaptor MAX)^2
//float scaleFactorSqV = (230*sqrt(2)/(Vm-VDCoffset))^2.00;
//ACAC adaptor max, measured from circuit, see excel spreadsheet 20160517_MecLabPowSocket
//Vm=Vmax, measured from circuit, see excel spreadsheet 20160517_Scope6onReal_1k8_18k (oscilloscope measurement)
//Mains max, estimate from online doc: 230*1.1(-6% +10% tolerance), in Manila: measured directly at the mains using multimeter

float VDCoffset = 2.617;                        //Using oscilloscope to measured the voltage circuit, VDCoffset depends on the signal wave output from voltage circuit, eg mains magnitude, acac magnitude, ardunio output magnitude
float arduinoSuppV = 5.005;
float sensorVScaler = arduinoSuppV/1023;

float sensor2MainsScaler = 136.99;
float sensor2MainsOffset = -0.603;

unsigned char Vrms = 220;                        //Vrms at the mains socket, measured by user
float scaledVrms = 0.0066 * Vrms + 0.0666;       //Experimental result, see "Test 2 voltage measurement calibration and accuracy.docx"
float scaleFactorV = Vrms / scaledVrms;          //Vrms and Vm should not matter because scaleFactor is only a scale as long as it is consistently Vm for Mains, ACAC and op amp output
float scaleFactorSqV = scaleFactorV * scaleFactorV; //Sq the scaleFactor as the methodogy of the code works in this way.


//float CurrOffset = 2.3976;                        //[V], Using oscilloscope to measured the current circuit, CurrOffset depends V+ into op amp.
float CurrOffset = -0.00003 * Vrms + 2.4045;     //Calibration using oscilloscope for different Mains Vrms, see "Test 1 Current measurement calibration and accuracy.docx"
//float R2 = 0.09333;                              //Burden resistor in secondary circuit, [m-ohm] to avoid current is too small thus so that the current is in unit of [mA]
float oneOverR2 = 10.715;

float scaleFactorSqA = 2 * 2;                    //I2*2000=I1, ratio relationship given by specificatiopn of current sensor(SCT-015-000)
//Because the burden resistor is in the unit of [m-ohm], so we just multiple by 2.
//ISR
//voltage

volatile unsigned long voltMCounter = 0;                    //Counter for voltage measurement points, count up to MSIZE
//Current measuremnt
volatile unsigned long currentMCounter = 0;                 //Counter for current measurement points, count up to MSIZE

//Mainloop
const unsigned int MSIZE = 1919;                  //1920 measurement points(1second of signal) to be stored in oneSecVoltSq and oneSecCurrSq.
const float oneOverTotalM = 1.0 / (1 + MSIZE);
unsigned int SCounter = 0;                        //Counter for number of VrmsSq(1s-averaged) and IrmsSq(1s-averaged) to be stored in sumRmsVoltSq and sumRmsCurrSq,counting starts from 0 up to SSize,for 6 min(360s),@60Hz.
const unsigned int SSIZE = 120;                   //360 of VrmsSq(1s-averaged) and IrmsSq(1s-averaged) respectively, to be stored into sumRmsVoltSq and sumRmsCurrSq.
const float oneOverSSIZE = 1.0 / (1 + SSIZE); //1/(1+SSIZE)
byte rmsCounter = 0;                              //Counter for number of VrmsSq(6min-averaged) and IrmsSq(6min-averaged) respectively,to be stored into avgRmsVoltSq and avgRmsCurrSq,counting starts from 0 up to rmsSizw,for 30min,@60Hz,
const byte rmsSIZE = 4;                           //5 of VrmsSq(6min-averaged) and IrmsSq(6min-averaged)respectively, to be stored into avgRmsVoltSq and avgRmsCurrSq.
const float oneOverTotalrms = 1.0 / (1 + rmsSIZE);

//Voltage operation

volatile unsigned long oneSecVoltSq = 0;                   //Sum of voltage measurement from Analog pin 3.
unsigned long sumRmsVoltSq = 0;                   //Sum of Vrms(1s-averaged) for 6 minutes, 360 data points
unsigned int sumAvgRmsVoltSq = 0;                 //Sum of Vrms(6min-averaged) for 30 minutes, 5 data points

//Current operation
volatile unsigned long oneSecCurrSq = 0;                    //Sum of current measurement from Analog pin 2.
double sumRmsCurrSq = 0;                            //SSum of Irms(1s-averaged) for 6 minutes, 360 data points.
double sumAvgRmsCurrSq = 0;                         //Sum of Irms(6min-averaged) for 30 minutes, 5 data points.

long sumPower = 0;
float sumTemp = 0;
float sumFreq = 0;
int sumDepth = 0;

int VoltageRMS (unsigned long VoltSq) {
  //Serial.print("VoltSq: ");
  //Serial.println(VoltSq);
  if (VoltSq > pgm_read_dword_near(realVrmsSqArray + 0)) {
    //Write into SD card "OVER230"
    return 999;
  }
  int k = 0;
  
  unsigned long curVal, prevVal;
  while (true) {
    prevVal = curVal;
    curVal = pgm_read_dword_near(realVrmsSqArray + k);
    if (curVal <= VoltSq) {
        break;
        
    }
    k+=1;
  }
  
  if (VoltSq - curVal > prevVal - VoltSq)
    return (301 - k);
  else
    return (300 - k);
}

int CurrentRMS (unsigned long CurrSq) {


  if (CurrSq > pgm_read_word_near(realCurrRmsSqArray + 0)) {                                            //Compare realOneSecCurrSqLCD with realCurrRmsin look up table,ie realCurrRmsSqArray.
    return 999;                                    //LCD display for measurement larger than the analog pin can support.
  }
  else if (CurrSq < pgm_read_word_near(realCurrRmsSqArray + 254)) {
    return 0;                                                                 //LCD display for measurement smaller than 1A which is insignificant compare to 45A for a 10kW generator, generating 220V.
  }
  else {
    byte k = 0;
    while (pgm_read_word_near(realCurrRmsSqArray + k) > CurrSq) k++;
    if (CurrSq - pgm_read_word_near(realCurrRmsSqArray + k) > pgm_read_word_near(realCurrRmsSqArray + k-1) - CurrSq) {
      return int((258 - k) * 1.857143) ;//*13/7
    }
    else {
      return int((257 - k) * 1.857143) ;
    }
  }
}


//If there is data in the softwareserial buffer then read the Forebar water depth
void RadioRead() //FUNCTION
{
  while (RadioSerial.available())
  {
    byte callsign = RadioSerial.read();
    byte firstByte, secondByte;
    if (callsign == 0xFB ) {
      //TIMSK2 = 0b00000000;
        firstByte = (RadioSerial.read());
        secondByte = (RadioSerial.read());
        //Serial.println((byte)firstByte);
        //Serial.println((byte)secondByte);
        depth = ((unsigned)((byte)firstByte) << 8) | ((byte)secondByte);
      
        //Serial.println(depth);
    //TIMSK2 = 0b00000010;
    }
  }

}
void initiateTimer(){
 // Timer 2 - Initiate ADC, 8bit TCNT2 and OCR2A (max count state: 256)
  ASSR = 0;                                        //Asynchronous Status Register(ASSR): for external clock source.
  TCCR2A = 0b00000010;                             //CTC mode, WGM21 WGM20 = 10
  TCCR2B = 0b00000100;                             //64 prescaler, CS22 CS21 CS20 ( 100 ) for timer2
  //for changing the frequency of the triggering point
  TCNT2 = 0;                                       //start counting from 0
  OCR2A = 129;                                     //interrupt trigger to create 32 measurement points per cycle @60Hz,  (1/60)/32 = 0.52ms/mpt
  //setting duty cycle, should not affect our sketch, remain the same while varying the TCCR2B
  TIMSK2 = 0b00000010;                             //sbi: OCIE2A, Output Compare A Match Interrupt Enable, interrupt on COMPARATOR A   
}
void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(A5, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A1, OUTPUT);
  //Setup ADC convertor
  DIDR0 = 0B00111111;                               //diable digital pin function on all analog pin
  ADCSRA = 0B10000101;                              //enable conversion, start conversion,
  //Prescaler (division factor)32(101), ADC clock = 16M/32=500kHz
  //Serial.println("Start2");

  currentAdcChannel = currentPin;                      //set up which analog input will be read first
  ADMUX = 0B01000000 | currentAdcChannel;           //Vref: AVcc with external capacitor at the AREF pin is used as Vref
  //ADLAR : switched off; so result is right justified
  //Analog pin 2 for input reading (0010)
  //Serial.println("ADC Setup complete");
  // Set up timers
  initiateTimer();
  RadioSerial.begin(9600);//set the data rate for SERIALPINS port
  //Serial.println("Timer2 Setup complete");
  //Interrupt-flag in the Status Register is set (interrupts globally enabled)
  lcd.begin(16, 2);

}


void loop() {

  //Number of measurement points in 1s = 32 mpt per cycle*60 cycle/s=1920 mpt/s
  //Happens every 1s: MSIZE = 1919 number, store VrmsSq(1s-averaged) and IrmsSq(1s-averaged)
  //into sumRmsVoltSq and sumRmsCurrSq respectively, LCD display for Vrms(1s-averaged) and Irms(1s-averaged) //
  //voltMCounter++;
  //currentMCounter++;
  
  if ((voltMCounter > MSIZE)  && (currentMCounter > MSIZE)) {
    

    
    //delay(500);
    //TIMSK2 = 0b00000000;
    if (voltMCounter < MSIZE) {
        Serial.println(voltMCounter);
        Serial.println("Voltage less");
    }
    
    long x = millis();
    long ttaken = x - prevSecondTimer;
    prevSecondTimer = x;
    if ((ttaken<997) || (ttaken>1003)) {
        Serial.println();
        Serial.println("PROBS:");
        Serial.println(ttaken);
        Serial.println();
    }

    /*for (i = 0; i<161;i++){
        Serial.println(Cur[i]);
        
    }*/
    //Serial.println(oneSecVoltSq);
    
    //Copy sums of rms
    unsigned long voltRMSTemp = oneSecVoltSq;
    unsigned long currRMSTemp = oneSecCurrSq;
        
    //Reset counters and rms sums
    voltMCounter = 0;                            
    currentMCounter = 0;
    oneSecCurrSq = 0;
    oneSecVoltSq = 0;

    //Convert arduino VrmsSq(1s-averaged) to real Vrms(eg.VrmsSq=48400=220*220). Insignificant decimal places, rounding is allowed.
    //unsigned int elRmsVoltSq = voltRMSTemp * oneOverTotalM;   //VrmsSq(1s-averaged)to be stored into sumRmsVoltSq. Decimal point wont affect the result significantly, eg 121081.233 vs 121081[BS]
    //unsigned long realOneSecVoltSqLCD = round (elRmsVoltSq * 0.004888 * scaleFactorSqV); // unsigned int realOneSecVoltSqLCD = round (elRmsVoltSq*(5.0/1023.0)*scaleFactorSqV);
    //int realVrms = VoltageRMS(realOneSecVoltSqLCD);
    unsigned long avgVSq =round (voltRMSTemp* oneOverTotalM);
    int realVrms = VoltageRMS(avgVSq);
    if (realVrms > maxVrms) {                           // Sets max and min values of Vrms
        maxVrms = realVrms;
    } else if ( realVrms < minVrms) {
        minVrms = realVrms;
    }

    if (realVrms <150) {
        voltPinStatus = 1;
    } else if (realVrms > 240) {
        voltPinStatus = 3;
    } else {
        voltPinStatus = 2;
    }

    if (voltPinStatus == 1) {
        digitalWrite(voltLowPin, HIGH);
        digitalWrite(voltHighPin, LOW);
        digitalWrite(voltPerfectPin, LOW);
    } else if (voltPinStatus == 2) {
        digitalWrite(voltLowPin, LOW);
        digitalWrite(voltHighPin, LOW);
        digitalWrite(voltPerfectPin, HIGH);
    } else if (voltPinStatus == 3) {
        digitalWrite(voltLowPin, LOW);
        digitalWrite(voltHighPin, HIGH);
        digitalWrite(voltPerfectPin, LOW);
    } else {
        digitalWrite(voltLowPin, LOW);
        digitalWrite(voltHighPin, LOW);
        digitalWrite(voltPerfectPin, LOW);
    }
    
    //Convert arduino IrmsSq sum to real IrmsSq
    float elRmsCurrSq = currRMSTemp * oneOverTotalM;  //IrmsSq(1s-averaged) to be stored in sumRmsCurrSq. Allow decimal points.
    unsigned long realOneSecCurrSqLCD = round(elRmsCurrSq * scaleFactorSqA * 10); //*10 as the elements in realCurrRmsSqArray is scaled up by 10 in order to show one deciaml point.           
    float realIrms = CurrentRMS(realOneSecCurrSqLCD) * 0.1;
    if (realIrms > maxIrms) {                           // Sets max and min values of current
        maxIrms = realIrms;
    } else if ( realIrms < minIrms) {
        minIrms = realIrms;
    }

    //Calculate Power
    int power = round(realVrms*realIrms);
    if (power > maxPower) {                           // Sets max and min values of current
        maxPower = power;
    } else if ( power < minPower) {
        minPower = power;
    }

    //Calculate Max and Min Voltage
    if (freq > maxFreq) {                           // Sets max and min values of current
        maxFreq = freq;
    } else if ( freq < minFreq) {
        minFreq = freq;
    }
    
    //Read Temp add to sum
    float Temp = thermocouple.readCelsius();


    //Read and update Depth
    RadioRead();
    lcd.setCursor(8, 1);
    lcd.print(depth);
    lcd.print(" m");

    //Update LCD values
    lcd.clear();
    
    //Temp
    lcd.setCursor(8, 0);
    lcd.print(power);
    lcd.print(" W");
    
    //Display Voltage
    lcd.setCursor(0, 0);
    lcd.print(realVrms);
    lcd.print(" V");
    //Serial.println(realVrms);

    //Display current
    lcd.setCursor(0, 1);
    lcd.print(realIrms, 1);
    lcd.print(" A");

    //Frequency
    lcd.setCursor(8, 1);
    lcd.print(freq,1);
    lcd.print(" Hz");

    Serial.print("Voltage: ");
    Serial.println(realVrms);
    Serial.print("Current: ");
    Serial.println(realIrms);
    Serial.print("Frequency: ");
    Serial.println(freq);
    Serial.print("Depth: ");
    Serial.println(depth);
    Serial.print("Temperature: ");
    Serial.println(Temp);
    Serial.println("------------------------------------------");

    
    sumRmsVoltSq += realVrms;               //Sum of 360 VrmsSq(1s-averaged)
    sumRmsCurrSq += realIrms;               //Sum of 360 IrmsSq(1s-averaged)
    sumTemp += Temp;
    sumDepth += depth;
    sumPower += power;
    sumFreq += freq;
    
    SCounter++;                                //increase by 1 of the Counter for every storage of VrmsSq(1s-averaged) and IrmsSq(1s-averaged) into sumRmsVoltSq and sumRmsCurrSq respectively.
                             //Set variable oneSecVoltSq to zero for another cycle(second) of storing 1920 VSq measurements
    //Serial.println("PROCESS TIME:");
    //Serial.println(millis()-x);
  
  }

  if (SCounter > SSIZE) {                   //Happens every 6min:rmsSIZE = 360, find real Vrms and Irms for from VrmsSq(30min-averaged) and IrmsSq(30min-averaged) measurements from arduino.
    Serial.println("Transmitting...");
    //     byte totalRmsPt =  rmsSIZE+1;                //number of 6min interval in 30min, 1 avg rms value/6min interval * 5 interval/30min = 5avg rms value/30 min interval
    int avgRmsVoltSq = sumRmsVoltSq * oneOverSSIZE; //Averaged of 5 x VrmsSq(6min-averaged),stored into avgRmsVoltSq. Decimal point wont affect the result significantly, eg 121081.233 vs 121081[BS]
    int avgRmsCurrSq = round (sumRmsCurrSq * oneOverSSIZE*10);      //Averaged of 5 x IrmsSq(6min-averaged),stored into avgRmsCurrSq. Allow deciaml places.
    int avgTemp = round (sumTemp * oneOverSSIZE);
    int avgDepth = round (sumDepth * oneOverSSIZE);
    int avgFreq = round (sumFreq * oneOverSSIZE);
    int power = round(sumPower * oneOverSSIZE);
    int sendMaxVolt = round(maxVolt *10);
    int sendMinVolt = round(minVolt *10);
    int SendMaxCur = round(maxCur *10);
    int sendMinCur = round(minCur*10);
    int sendRmsCur = round(avgRmsCurrSq * 10);
    int sendAvgFreq = round(avgFreq * 10);
    int sendMinFreq = round(minFreq * 10);
    int sendMaxFreq = round(maxFreq * 10);
    int sendAvgTemp = round(avgTemp * 10);
    int sendMaxIrms = round(maxIrms * 10);
    int sendMinIrms = round(minIrms * 10);

    
    
    
    //unsigned int realRmsVoltSq = round (avgRmsVoltSq * 0.004888 * scaleFactorSqV); //Insigificant deciaml places, rounding is allowed.
    //unsigned int realRmsCurrSq = round (avgRmsCurrSq * scaleFactorSqA * 10);


    ////#################### Compare to  realVrmsSqArray to obtain real Vrms #######################     //Same structure as above "For LCD display"
    ////############################################################################################
    //////_____________________________Voltage
    //int realVrms = VoltageRMS(realRmsVoltSq);
    //Serial.println(realVrms);
    //////_________________________Current_________________________________________
    //float realIrms = CurrentRMS(realRmsCurrSq) * 0.1;
    //Serial.println(realIrms);
    
    // Things to send:  avgRmsVoltSq | minVolt | maxVolt | avgRmsCurrSq | minCur | maxCur | avgFreq | minFreq | maxFreq | Power | avgDepth | avgTemp |
    //          Sizes:  int 2        | int 2   | int 2   | int 2        | int 2  | int 2  | int 2   | int 2   | int 2   | int 2 | int 2    | char 1  |

    char data[40];

    data[0] = 0xEB; //Callsign 1
    
    //Channel 1------------------------
    data[1] = avgRmsVoltSq >> 8;      // Average RMS Voltage Most Significant Bits (MSB)
    data[2] = 0xFF & avgRmsVoltSq;    // Average RMS Voltage Least Significant Bits (LSB)
    data[3] = avgRmsCurrSq >> 8;      // Average RMS Current MSBs
    data[4] = 0xFF & avgRmsCurrSq;    // Average RMS Voltage LSBs
    data[5] = sendAvgFreq >> 8;          // Average Frequency MSBs
    data[6] = 0xFF & sendAvgFreq;        // Average Frequency LSBs
    data[7] = power >> 8;            // Power MSBs
    data[8] = 0xFF & power;          // Power LSBs
    data[9] = avgDepth >> 8;         // Average Depth MSBs
    data[10] = 0xFF & avgDepth;       // Average Depth LSBs
    data[11] = sendAvgTemp >> 8;          // Average Temperature MSBs
    data[12] = 0xFF & sendAvgTemp;        // Average Temperature LSBs

    //Channel 2-----------------------
    data[13] = maxVrms >> 8;
    data[14] = 0xFF & maxVrms;
    data[15] = minVrms >> 8;
    data[16] = 0xFF & minVrms;
    data[17] = sendMaxIrms >> 8;
    data[18] = 0xFF & sendMaxIrms;
    data[19] = sendMinIrms >> 8;
    data[20] = 0xFF & sendMinIrms;
    data[21] = sendMaxFreq >> 8;          // Maximum Frequency MSBs
    data[22] = 0xFF & sendMaxFreq;        // Maximum Frequency LSBs
    data[23] = sendMinFreq >> 8;          // Minimum Frequency MSBs
    data[24] = 0xFF & sendMinFreq;        // Minimum Frequency LSBs
    data[25] = maxPower >> 8;
    data[26] = 0xFF & maxPower;
    data[27] = minPower >> 8;
    data[28] = 0xFF & minPower;

    //Channel 3----------------------
    
    data[29] = sendMaxVolt >> 8;           // Maximum Voltage MSBs
    data[30] = 0xFF & sendMaxVolt;         // Maximum Voltage LSBs
    data[31] = sendMinVolt >> 8;           // Minimum Voltage MSBs
    data[32] = 0xFF & sendMinVolt;         // Minimum Voltage LSBs
    data[33] = SendMaxCur >> 8;           // Maximum Current MSBs
    data[34] = 0xFF & SendMaxCur;         // Maximum Current LSBs
    data[35] = sendMinCur >> 8;            // Minimum Current MSBs
    data[36] = 0xFF & sendMinCur;         // Minimum Current LSBs
    
    

    //RadioSerial.begin(9600);
    Serial.println("Raw Data:");
    String toSend;
    for (int i = 0; i<40; i++) {
        Serial.print((byte)data[i]);
        Serial.print(", ");
        toSend+=data[i];
    }
    Serial.println("");
    TIMSK2 = 0b00000000;
    RadioSerial.print(toSend);
    TIMSK2 = 0b00000010;
    //Serial.println("Bytes Transmitted: " + String(x));

    ////############### End of comparion, value to be uploaded and written into SD card ####################
    ////####################################################################################################
    SCounter = 0;                             //clear the counter to restart the counting from 0.
    sumRmsVoltSq = 0;                        //Set variable sumAvgRmsVoltSq to zero for another 30-min interval of storing 5 VrmsSq(6min-averaged).
    sumRmsCurrSq = 0;                        //Set variable sumAvgRmsCurrSq to zero for another 30-min interval of storing 5 IrmsSq(6min-averaged).
    sumTemp = 0;
    sumDepth = 0;

    oneSecCurrSq = 0;
    oneSecVoltSq = 0;
    currentMCounter = 0;
    voltMCounter = 0;

    long x = millis();
    prevSecondTimer = x;
    
    sumRmsVoltSq = 0;
    sumRmsCurrSq = 0;
    sumTemp = 0;
    sumDepth = 0;
    sumFreq = 0;
    maxVolt = 0;
    minVolt = 0;
    maxCur = 0;
    minCur = 0;
    minFreq = 0;
    maxFreq = 0;
    maxIrms = 0;
    minIrms = 0;
    sumPower= 0;
    minPower = 0;
    maxPower = 0;
    
  }


}


//Interrupt
//Voltage and current measurement
ISR(TIMER2_COMPA_vect) {
  ADCSRA = B11001101;                            //successfully activate ADC conversion
}


ISR(ADC_vect) { //This interrupt is called 3,840 times per second.
  //Alternate invocations run either current or voltage measurement, giving each of those 1920 samples/second or 32 samples/sec at 60Hz waveform.
  unsigned int lowerbits = ADCL;
  unsigned int sensorVal = lowerbits | ((unsigned int) ADCH << 8);    //variable "sesnsorVal" remains in interrupt to represents the measurements from analog pin 2 and 3.
  sei();
  //Current measurement
  if (currentAdcChannel == currentPin) {
    //startTime = micros();
    //Discrete voltage is stored for processing after current measurement
    curTemp = sensorVal;
        
    currentMCounter++;//increase by 1 for every storage of ISq into oneSecCurrSq
    currentAdcChannel = voltPin;                              //set the next ADC to be Analog pin 3 for current measurement.
    ADMUX = 0B01000000 | currentAdcChannel;                   //set Analog pin 2 for voltage measurement
    ADCSRA = B11001101;                                //start ADC conversion for current, so goes back line with "ISR(ADC_vect)" since ADIF is flagged!
  }

  //Voltage measurement is triggered immediately after one Current mpt from ADC
  //Processing is done in time window after voltage measurement
  else {

    
    if (voltMCounter > 161){
        count = false;
    }
    //Serial.println(sensorVal);
    //long startTime = millis();
    //Voltage value is has offset removed and frequency calculation is made
    float sensorVoltOffset = ((float)sensorVal) * sensorVScaler - VDCoffset;
    //This is the voltage at the arduino input pin [theoretically]
    
    //Frequency measurement is made by starting by setting a flag every time the voltage goes through zero so that
    //number of cycles can be counted. Then using a timer to measure how long 200 cycles took and dividing for frequency
    
    if ((fFlag==false) && sensorVoltOffset>0) {         // sets flags and starts timer
        fCycles+=1;
        fFlag = true;
        if (fStartTime == 0) {
            fCycles = 0;
            fStartTime = millis();
        }    
    } else if ((fFlag==true) && (sensorVoltOffset<0)) {
        fFlag = false;
    }

    if (fCycles==200) { //equal to 200              //ends timer and records length
        unsigned long fEndTime = millis();
        float timeTaken = fEndTime-fStartTime;
        freq = 200/timeTaken*1000;
        fStartTime = fEndTime;
        fCycles=0;
    }

    //Voltage value is processed
    float mainsVolt = sensorVoltOffset*sensor2MainsScaler + sensor2MainsOffset;     //This should be the *real* mains voltage [discrete not RMS]
    float oneSecVoltSqinV = mainsVolt * mainsVolt; //VSq = square of the V mpt in [V]
    
    //    oneSecVoltSqinState = (oneSecVoltSqinV/5.0)*1023.0;  //Vsq = square of the voltage measurement in [state]
    //float oneSecVoltSqinState = oneSecVoltSqinV * 204.6;     //1023/5 = 204.6, to avoid division
    
    //Serial.println(oneSecVoltSqinState);
    oneSecVoltSq += round (oneSecVoltSqinV);

    
    if (mainsVolt > maxVolt) {                           // Sets max and min values of current
        maxVolt = mainsVolt;
    } else if ( mainsVolt < minVolt) {
        minVolt = mainsVolt;
    }
    
    //Process current value and adds to RMS sum
    double sensorCurrOffset = curTemp * sensorVScaler - CurrOffset;
    double sensorCurrOffsetInA = sensorCurrOffset * oneOverR2;           //V=IR, Convert [V] to [mA] (need to multiply R2^2 if continue using [V]}.
    //Serial.println(sensorCurrOffsetInA);
    double oneSecCurrSqInA = sensorCurrOffsetInA * sensorCurrOffsetInA;  //ISq = square of the I measurement in [mA^2]
    //Serial.println(oneSecCurrSqInA);
    oneSecCurrSq += round (oneSecCurrSqInA);                     //Insignificant decimal places, rounding is allowed.
    //Serial.println(oneSecCurrSq);

    float mainsCurr = sensorCurrOffsetInA*2;
    if (mainsCurr> maxCur) {                           //Sets Max and min values of voltage
        maxCur = mainsCurr;
    } else if ( mainsCurr < minCur) {
        minCur = mainsCurr;
    }
    /*if (count) {

        Cur[currentMCounter] = oneSecCurrSq;
    }*/
    
    voltMCounter++;                                    //increase by 1 for every storage of VSq into oneSecVoltSq
    currentAdcChannel = currentPin;                    //set the next ADC for Analog pin 3 for votlage measurement.
    ADMUX = 0B01000000 | currentAdcChannel;            //set Analog pin 3 for current measurement.
    //Serial.println("PROCESS TIME:");
    //Serial.println(micros()-startTime);
  }

}


