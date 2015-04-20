#include "CapacitiveSensor.h"
#define SERIAL_DEBUG 1
const unsigned int statusBit = 13;

// DAC and waveform variables
const unsigned int outputResolution = 256;
const unsigned int waveformSteps = 64;  // The length of the waveform lookup table
unsigned int waveformOffset = 0;
byte wave[waveformSteps];   // Storage for the waveform

// Pitch variables 
float current_frequency = 440;
float target_frequency = 440;
const unsigned int min_pitch = 50;
const unsigned int max_pitch = 10000;

// Sensor and input variables
CapacitiveSensor pitch_sensor = CapacitiveSensor(8,9);        // 10M resistor between pins 8 & 9, pin 9 is sensor pin, add a wire and or foil
CapacitiveSensor volume_sensor = CapacitiveSensor(10,11);        // 10M resistor between pins 10 & 11, pin 11 is sensor pin, add a wire and or foil
const unsigned int sample_size = 30; // Currently the default provided by the example


const unsigned long clockSpeed = 16000000;
void setFrequency(float hz) {
  if(hz < min_pitch) {
    hz = min_pitch;
  }
  else if(hz > max_pitch) {
    hz = max_pitch;
  }
  
  int tickCount = clockSpeed / ( waveformSteps * hz) ;
  OCR1A = tickCount;
}

void adjustFrequency() {
  if(target_frequency > current_frequency) {
    setFrequency(++current_frequency);
  }
  else if(target_frequency < current_frequency) {
    setFrequency(--current_frequency);
  }
}

void initPins() {
  pinMode(statusBit, OUTPUT);
  
  // This maps to PORTD, which is being used below
  for(int i = 0; i < 8; i++) {
    pinMode(i, OUTPUT);   
  }
  
  const unsigned long timeout = 1500 * 310;
  pitch_sensor.set_CS_Timeout_Millis(timeout);
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
  setFrequency(current_frequency);

  // Reenable interrupts
  sei();
}

void setup() {
  initPins();

  digitalWrite(statusBit, HIGH);

#ifdef SERIAL_DEBUG
  Serial.begin(9600);
#endif

  initWaveform();
  initClock();
 
  digitalWrite(statusBit, LOW);
}

void loop() {
  // Make the light blink or something...
  
  long pitch_raw = pitch_sensor.capacitiveSensorRaw(sample_size);
  //long volume_raw = volume_sensor.capacitiveSensor(sample_size);
  
  if(pitch_raw > 0) {
    target_frequency = (pitch_raw - 4500); // / 2;
  }
  else {
   target_frequency = max_pitch; 
  }

#ifdef SERIAL_DEBUG
  // Serial output is disrupted by the timer.
  // This disables global interrupts
  cli();
  Serial.print("Raw Pitch: ");
  Serial.print(pitch_raw);
  Serial.print("\tPitch: ");
  Serial.println(target_frequency);
  // Reenable interrupts
  sei();
#endif

  delay(10);
}

// Use the timer based interrupt configured in setup to update
// the output port. The interrupt is configured so that it is
// called at the rate of one step of our waveform lookup.
ISR(TIMER1_COMPA_vect)
{
  // Use PORTD instead of direct digitalWrite() for speed.
  // PORTD maps to pins 0-7
  PORTD = wave[waveformOffset++];

  if(waveformOffset >= waveformSteps) {
     waveformOffset = 0;
     adjustFrequency();
  }
}
