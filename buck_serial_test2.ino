// Sets the maximum PWM level for each timer. Used to set its PWM frequency (f=16MHz/TIMER_TOP_n).
// Maximum for timers 0 and 2 is 255. Maximum for timer 1 is 65535.
#define TIMER_TOP_0 255 

// Sets the maximum duty cycle. The higher the duty cycle, the higher the LED current.
#define MAX_DUTY_0B 0.89

#define FEEDBACKPIN A0

// Select HIGH when connecting outputs to the boost LED driver and LOW when connecting outputs to the buck LED driver. Output is inverted when using boost mode.
boolean outputBoostMode=LOW;

// Calculates the maximum PWM level. 255 is the maximum PWM level for timers 0 and 2.
const int maxOutput = TIMER_TOP_0 * MAX_DUTY_0B; 

int maxSetAnalogReading;
int desiredAnalogReading;

const int minOutput = 0; // Soft starts the converter.

int output;
int analogReading;
float analogReadingMilliAmps;
float analogReadingMilliVolts;
byte inKey;
bool autoMode;
float maSetpoint = 2970;
float inRange = 30;
long currentMillis;
long lastMillis;
long lastMillis2;

// Pin numbers of the Arduino Uno's PWM pins where 6 and 5 are timer 0's, 9 and 10 are timer 1's, and 11 and 3 are timer 2's.
#define OUTPUTPIN 5

void setup(){
  Serial.begin(115200);
  pinMode(FEEDBACKPIN,INPUT); // Set the ADC pins as inputs
  pinMode(OUTPUTPIN,OUTPUT);
  TCCR0A = _BV(COM0A1)|_BV(COM0B1)|_BV(WGM01)|_BV(WGM00);
  TCCR0B = _BV(CS00);
  if(outputBoostMode==HIGH){
    TCCR0A|=_BV(COM0B0);
  }// Inverts the PWM outputs if boost mode is selected.
  analogReference(INTERNAL); // Uses the internal 1.1V analog reference voltage.
  output = minOutput;
  OCR0A = TIMER_TOP_0;
  autoMode = 0;
  lastMillis = 0;
  lastMillis2 = 0;
  autoMode = 1;
}

void loop(){
  // Adjusts output current based on feedback voltage.
  //output = setOutput(feedbackPin,output,desiredAnalogReading,maxOutput);
  //OCR0B = output;
  while (Serial.available() > 0) {
    inKey = Serial.read();
    if (inKey == '.' && output < maxOutput){
      output++;
      Serial.println("Up");
    }
    if (inKey == ',' && output > minOutput){
      output--;
      Serial.println("Down");
    }
    if (inKey == '('){
      output = 0;
      Serial.println("Off");
    }
    if (inKey == ')'){
      output = 221;
      Serial.println("3Amps");
    }
    if (inKey == 'a'){
      if (autoMode == 0){
        autoMode = 1;
       Serial.println("Auto on");
       }
       else
       if (autoMode == 1){
        autoMode = 0;
        output = 0;
       Serial.println("Auto off");
       }
     }
  }
  analogReading = analogRead(FEEDBACKPIN);
  analogReadingMilliVolts = map(analogReading, 0, 1023, 0, 1100);
  analogReadingMilliAmps = analogReadingMilliVolts / 0.22;
  if (autoMode == 1){
    if (analogReadingMilliAmps > (maSetpoint + inRange)){
      if (analogReadingMilliAmps > maSetpoint && output > minOutput){
        output--;
      }
    }
    if (analogReadingMilliAmps < (maSetpoint - inRange)){
      if (analogReadingMilliAmps < maSetpoint && output < maxOutput){
        output++;
      }
    }
  }
  if (output > maxOutput){
    output = maxOutput;
    Serial.println("Output Higher than maxOutput");
  }
  if (output > 128 && analogReading < 16){
    output = 0;
    currentMillis = millis();
      if ((currentMillis - lastMillis2) > 12800){
        Serial.println("No Load");
        lastMillis2 = currentMillis;
      }
  }
  OCR0B = output;
  currentMillis = millis();
  if ((currentMillis - lastMillis) > 12800){
    Serial.print("Analog reading: ");
    Serial.print(analogReading);
    Serial.print("   ");
    Serial.print("Milliamps: ");
    Serial.print(analogReadingMilliAmps);
    Serial.print("   ");
    Serial.print("Output: ");
    Serial.println(output);
    lastMillis = currentMillis;
  }
}
