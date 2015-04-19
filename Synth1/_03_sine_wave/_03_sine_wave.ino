float frequency = 2500;

const unsigned int waveformSteps = 64;  // The length of the waveform lookup table
unsigned int waveformOffset = 0;
byte wave[waveformSteps];   // Storage for the waveform

const unsigned int outputResolution = 256;
const unsigned int statusBit = 13;

const unsigned long clockSpeed = 16000000;
void setFrequency(float hz) {
  
  int tickCount = clockSpeed / ( waveformSteps * hz) ;
  OCR1A = tickCount;

}

void initPins() {
  pinMode(statusBit, OUTPUT);
  
  // This maps to PORTD, which is being used below
  for(int i = 0; i < 8; i++) {
    pinMode(i, OUTPUT);   
  }
}

void initWaveform() {
  // Populate the waveform lookup table with a sine wave
  int dcOffset = outputResolution / 2 - 1;

  for (int i=0; i < waveformSteps ; i++) {
    float radians = (i * 2 * PI) / waveformSteps;
    const double amplitude = sin(radians);
    
    // map -1 to 1 values to 0 - dcOffset
    wave[i] = int( (dcOffset * amplitude) + dcOffset); 
  }  
}

void initClock() {
  
  // This disables global interrupts
  cli();
 
  // The TCCRx[AB] (Timer/Counter Control Register) registers map to the 
  // timers on the AVR chip. Set them to a blank state and then configure them.
  // Timer 1 is 16-bit (timers 0 and 2 are 8-bit)
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Set the timer to Clear Timer on Compare Match mode. In this mode
  // the timer will count until it's value matches OCRxA (see set_frequency())
  TCCR1B |= (1 << WGM12);

  // The TIMSK1 (Timer/Counter1 Interrupt Mask Register) register controls
  // which interrupts will fire. Make timer1 call ISR(TIMER1_COMPA_vect) on
  // comparison with OCRxA (see set_frequency())
  TIMSK1 |= (1 << OCIE1A);
  // Set CS10 bit so timer runs at clock speed:
  TCCR1B |= (1 << CS10);

  // Set OCR1A here
  setFrequency(frequency);

  // Reenable interrupts
  sei();
}

void setup() {
  initPins();

  digitalWrite(statusBit, HIGH);  
  //Serial.begin(9600);


  initWaveform();
  initClock();
 
  digitalWrite(statusBit, LOW);
}

void loop() {
  // Make the light blink or something...
}

// Use the timer based interrupt configured in setup to update
// the output port. The interrupt is configured so that it is
// called at the rate of one step of our waveform lookup.
ISR(TIMER1_COMPA_vect)
{
  // Use PORTD instead of direct digitalWrite() for speed.
  // PORTD maps to pins 0-7
  PORTD = wave[waveformOffset++];

  // CR set frequency here?
  if(waveformOffset >= waveformSteps) {
     waveformOffset = 0;
  }
}
