//sine wave freq detection with 38.5kHz sampling rate and interrupts
//by Amanda Ghassaei
//http://www.instructables.com/id/Arduino-Frequency-Detection/
//July 2012

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
*/

//clipping indicator variables
boolean clipping = 0;

//data storage variables
byte newData = 0;
byte prevData = 0;

//freq variables
unsigned int timer = 0;//counts period of wave
unsigned int period;
unsigned int same_value_count = 0;
unsigned int cicle_count = 0;
int frequency;
int past_frequency = 0;

int colors[3];

void setup(){
  
  Serial.begin(9600);
  
  pinMode(13,OUTPUT);//led indicator pin
  
  cli();//diable interrupts
  
  //set up continuous sampling of analog pin 0
  
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();//enable interrupts
}

ISR(ADC_vect) {//when new ADC value ready

  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >=127){//if increasing and crossing midpoint
    period = timer;//get period
    timer = 0;//reset timer
  }
  
  
  if (newData == 0 || newData == 1023){//if clipping
    PORTB |= B00100000;//set pin 13 high- turn on clipping indicator led
    clipping = 1;//currently clipping
  }
  
  timer++;//increment timer at rate of 38.5kHz
}

void loop(){
  if (clipping){//if currently clipping
    PORTB &= B11011111;//turn off clippng indicator led
    clipping = 0;
  }

  frequency = 38462/period;//timer rate/period
  
  //Calculo el color
    float t = (float) frequency / 2000.00;
    
  //print results
  Serial.print(frequency);
  Serial.print(" hz ");
  Serial.print(" hue ");
  Serial.println(t);
 
  H2R_HSBtoRGBfloat(t, 1, 1, colors);

//  Serial.print(prevData);
//  Serial.print(" - ");
//  Serial.print(newData);
//  Serial.println("");

  
  if (past_frequency == frequency) {
    same_value_count++;
    
    if (same_value_count > 8) {
      //Shutdown lights
      analogWrite(5, 0);
      analogWrite(6, 0);
      analogWrite(7, 0);
      same_value_count = 0;
    }
    
  } else {
    analogWrite(5, 1 -  colors[0]);
    analogWrite(6, 1 - colors[1]);
    analogWrite(7, 1 - colors[2]);
    past_frequency = frequency; 
  }
  
  delay(100);
}


#define H2R_MAX_RGB_val 255.0

void H2R_HSBtoRGB(int hue, int sat, int bright, int* colors) {
    
  // constrain all input variables to expected range
    hue = constrain(hue, 0, 360);
    sat = constrain(sat, 0, 100);
    bright = constrain(bright, 0, 100);

  // define maximum value for RGB array elements
  float max_rgb_val = H2R_MAX_RGB_val;

  // convert saturation and brightness value to decimals and init r, g, b variables
    float sat_f = float(sat) / 100.0;
    float bright_f = float(bright) / 100.0;        
    int r, g, b;
    
    // If brightness is 0 then color is black (achromatic)
    // therefore, R, G and B values will all equal to 0
    if (bright <= 0) {      
        colors[0] = 0;
        colors[1] = 0;
        colors[2] = 0;
    } 
  
  // If saturation is 0 then color is gray (achromatic)
    // therefore, R, G and B values will all equal the current brightness
    if (sat <= 0) {      
        colors[0] = bright_f * max_rgb_val;
        colors[1] = bright_f * max_rgb_val;
        colors[2] = bright_f * max_rgb_val;
    } 
    
    // if saturation and brightness are greater than 0 then calculate 
  // R, G and B values based on the current hue and brightness
    else {
        
        if (hue >= 0 && hue < 120) {
      float hue_primary = 1.0 - (float(hue) / 120.0);
      float hue_secondary = float(hue) / 120.0;
      float sat_primary = (1.0 - hue_primary) * (1.0 - sat_f);
      float sat_secondary = (1.0 - hue_secondary) * (1.0 - sat_f);
      float sat_tertiary = 1.0 - sat_f;
      r = (bright_f * max_rgb_val) * (hue_primary + sat_primary);
      g = (bright_f * max_rgb_val) * (hue_secondary + sat_secondary);
      b = (bright_f * max_rgb_val) * sat_tertiary;  
        }

        else if (hue >= 120 && hue < 240) {
      float hue_primary = 1.0 - ((float(hue)-120.0) / 120.0);
      float hue_secondary = (float(hue)-120.0) / 120.0;
      float sat_primary = (1.0 - hue_primary) * (1.0 - sat_f);
      float sat_secondary = (1.0 - hue_secondary) * (1.0 - sat_f);
      float sat_tertiary = 1.0 - sat_f;
      r = (bright_f * max_rgb_val) * sat_tertiary;  
      g = (bright_f * max_rgb_val) * (hue_primary + sat_primary);
      b = (bright_f * max_rgb_val) * (hue_secondary + sat_secondary);
        }

        else if (hue >= 240 && hue <= 360) {
      float hue_primary = 1.0 - ((float(hue)-240.0) / 120.0);
      float hue_secondary = (float(hue)-240.0) / 120.0;
      float sat_primary = (1.0 - hue_primary) * (1.0 - sat_f);
      float sat_secondary = (1.0 - hue_secondary) * (1.0 - sat_f);
      float sat_tertiary = 1.0 - sat_f;
      r = (bright_f * max_rgb_val) * (hue_secondary + sat_secondary);
      g = (bright_f * max_rgb_val) * sat_tertiary;  
      b = (bright_f * max_rgb_val) * (hue_primary + sat_primary);
        }
        
        colors[0]=r;
        colors[1]=g;
        colors[2]=b;
    
    }
}

void H2R_HSBtoRGBfloat(float hue, float sat, float bright, int* colors) {
  if (hue > 1) hue = 1.0;
  if (sat > 1) sat = 1.0;
  if (bright > 1) bright = 1.0;
    H2R_HSBtoRGB(hue*360.0, sat*100.0, bright*100.0, colors); 
}