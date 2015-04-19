float radians;
long frequency = 440;
const int lsbPin = 2;
const int msbPin = 9;
const int outputResolution = 64 ;// 256;
const int statusBit = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 
  for(int i = lsbPin; i <= msbPin; i++) {
    pinMode(i, OUTPUT); 
  }
  pinMode(statusBit, OUTPUT);

}

void dacWrite(int outputAmplitude) {
  Serial.print(F("DAC Value: x"));
  Serial.println(outputAmplitude, BIN);

  int bitMask = 1;
  for(int i = lsbPin; i <= msbPin; i++) {
    const int isHigh = outputAmplitude & bitMask;
    
    digitalWrite(i, isHigh ? HIGH : LOW);
    bitMask = bitMask * 2;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

//  const int seconds = millis() / 1000;
//  frequency = 110 * (seconds / 10);


  const long periodMicros = 1000000 / frequency;
  
  radians = (micros() % periodMicros) * PI / periodMicros;
  const double amplitude = sin(radians);
  
  const int outputAmplitude = amplitude * outputResolution; // for 8 bit resolution
  dacWrite(outputAmplitude);



/*
  for(int i = 0; i < 8; i++) {
    digitalWrite(statusBit, HIGH);
    dacWrite(i);
    
    delay(250);
    digitalWrite(statusBit, LOW);
    delay(250);
  }
*/
  // CR put a delay in here and just light up some LEDs to check that we have something working

}
