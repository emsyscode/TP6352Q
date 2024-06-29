/****************************************************/
/* This is only one example of code structure       */
/* OFFCOURSE this code can be optimized, but        */
/* the idea is let it so simple to be easy catch    */
/* where can do changes and look to the results     */
/****************************************************/
//set your clock speed
#define F_CPU 16000000UL
//these are the include files. They are outside the project folder
#include <avr/io.h>
//#include <iom1284p.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define VFD_in 8// If 0 write LCD, if 1 read of LCD
#define VFD_clk 9 // if 0 is a command, if 1 is a data0
#define VFD_stb 10 // Must be pulsed to LCD fetch data of bus
#define AdjustPins    PIND // before is C, but I'm use port C to VFC Controle signals
unsigned char DigitTo7SegEncoder(unsigned char digit, unsigned char common);
/*Global Variables Declarations*/
//unsigned char day = 7;  // start at 7 because the VFD start the day on the left side and move to rigth... grid is reverse way
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char milisec = 0;
unsigned char points = 0;
unsigned char secs;
unsigned char digit;
unsigned char number;
unsigned char numberA;
unsigned char numberB;
unsigned char numberC;
unsigned char numberD;
unsigned char numberE;
unsigned char numberF;
unsigned char grid = 0x00;
unsigned int k=0;
bool flag = false;
unsigned int segments[] ={
  // Here I'm forced to use the "0" as 10, because the 7 segments sart in "1"
  // This table is inverted
  //    This not respect the table for 7 segm like "abcdefgh"  //
  //    .decgfba         
      0b01110111,      //0 
      0b00010010,      //1
      0b01101011,      //2
      0b01011011,      //3 
      0b00011110,      //4 
      0b01011101,      //5 
      0b01111101,      //6 
      0b00010011,      //7 
      0b01111111,      //8
      0b00011111,      //9 
      0b00000000       //10 // empty display
  };
unsigned long grids[] ={
  //font data
  //  The grid on this display count from left to right  // 
    0b01000000, //  Grid 1
    0b00100000, //  Grid 2
    0b00010000, //  Grid 3
    0b00001000, //  Grid 4
    0b00000100, //  Grid 5
    0b00000010, //  Grid 6
    0b00000001, //  Grid 7
  };
void pt6312_init(void){
  delayMicroseconds(200); //power_up delay
  // Note: Allways the first byte in the input data after the STB go to LOW is interpret as command!!!
  // Configure VFD display (grids)
  cmd_with_stb(0b00000010);//  (0b01000000)    cmd1 6 grids 16 segm
  delayMicroseconds(1);
  // Write to memory display, increment address, normal operation
  cmd_with_stb(0b01000000);//(BIN(01000000));
  delayMicroseconds(1);
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  cmd_with_stb(0b11000000);//(BIN(01100110)); 
  delayMicroseconds(1);
  // set DIMM/PWM to value
  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)
  delayMicroseconds(1);
}
void cmd_without_stb(unsigned char a){
  // send without stb
  unsigned char transmit = 7; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_in, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_in, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
   //digitalWrite(VFD_clk, LOW);
}
void cmd_with_stb(unsigned char a){
  // send with stb
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_in, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_in, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
void test_VFD(void){
  /* 
  Here do a test for all segments of 6 grids
  each grid is controlled by a group of 2 bytes
  by these reason I'm send a burst of 2 bytes of
  data. The cycle for do a increment of 3 bytes on 
  the variable "i" on each test cycle of FOR.
  */
  // to test 6 grids is 6*3=18, the 8 gird result in 8*3=24.
 
  clear_VFD();
      
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
        
         for (int i = 0; i < 7 ; i++){ // test base to 16 segm and 6 grids
         cmd_without_stb(0b11111111); // Data to fill table 5*16 = 80 bits
         cmd_without_stb(0b11111111); // Data to fill table 5*16 = 80 bits
         }
    
      //cmd_without_stb(0b00000010); // cmd1 Here I define the 7 grids and 15 Segments
      //cmd_with_stb((0b10001000) | 7); //cmd 4
      digitalWrite(VFD_stb, HIGH);
      delay(1);
      delay(200);  
}
void wrLED(){
  for(int led=0; led < 4; led++){
              digitalWrite(VFD_stb, LOW);
              delayMicroseconds(2);
              cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 16 Segments
              delayMicroseconds(2);
                digitalWrite(VFD_stb, LOW);
                delayMicroseconds(2);
                cmd_without_stb(0b01000001);
               switch (led){
                  case 0: cmd_without_stb(0b00001110); break; //LED 0 //Note the LED's 0,1,2,3 is reverse mode 0=On 1=Off
                  case 1: cmd_without_stb(0b00001101); break; //LED 1
                  case 2: cmd_without_stb(0b00001011); break; //LED 2
                  case 3: cmd_without_stb(0b00000111); break; //LED 3
                }
              //
              digitalWrite(VFD_stb, HIGH);
              delay(500);
  }
}
void test_VFD_chkGrids(void){
  /* 
  Here do a test for all segments of 5 grids
  each grid is controlled by a group of 2 bytes
  by these reason I'm send a burst of 2 bytes of
  data. 
  */
  // to test 6 grids is 6*2=12. Result memory addr as: 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A and respectiv pair! 0x01, 0x03, 0x05...
 
  clear_VFD();
      
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 15 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
             for (int i = 0; i < 7 ; i++){ // test base to 15 segm and 7 grids
             cmd_without_stb(0b11111111); // Data to fill table 5*16 = 80 bits
             cmd_without_stb(0b11111111); // Data to fill table 5*16 = 80 bits
             
             }
          digitalWrite(VFD_stb, HIGH);
          delayMicroseconds(1);
          cmd_with_stb((0b10001000) | 7); //cmd 4
      
        delay(1);
        delay(100);
}
void test_VFD_grid(void){
    clear_VFD();
    //
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 15 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      //
      cmd_with_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
      for (int i = 0; i < 12 ; i=i+2){ // test base to 15 segm and 6 grids
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000) | i);
        //
             cmd_without_stb(0b11111111); // Data to fill table 6*16 = 96 bits
             cmd_without_stb(0b11111111); // Data to fill table 6*16 = 96 bits
        //
             digitalWrite(VFD_stb, HIGH);
             cmd_with_stb((0b10001000) | 7); //cmd 4
        //
        delay(1);
        delay(300);
      }
}
void clear_VFD(void){
  /*
  Here I clean all registers 
  Could be done only on the number of grid
  to be more fast. The 12 * 3 bytes = 36 registers
  */
      for (int n=0; n < 14; n++){  // important be 10, if not, bright the half of wells./this on the VFD of 6 grids)
        cmd_with_stb(0b00000010); //       cmd 1 // 6 Grids & 15 Segments
        cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
            cmd_without_stb((0b11000000) | n); // cmd 3 //wich define the start address (00H to 15H)
            cmd_without_stb(0b00000000); // Data to fill table of 6 grids * 15 segm = 80 bits on the table
            //
            //cmd_with_stb((0b10001000) | 7); //cmd 4
            digitalWrite(VFD_stb, HIGH);
            delayMicroseconds(100);
     }
}
void PT6312_RunWeels(){
  int j, n;
  byte x = 0x00;
  short v = 0b0000000000000001;  // The short have a size of 16 bits(2 bytes)
            for(j = 1; j < 0x0E; j++) {  // execute 15 times the for cycle
                          //cmd1 Configure VFD display (grids) 
                          cmd_with_stb(0b00000010);//  6 grids
                          delay(1);  // 
                          
                          //cmd2 Write to memory display, increment address, normal operation 
                          cmd_with_stb(0b01000000);//Teste mode setting to normal, Address increment Fixed, Write data to display memory...
                          
                          digitalWrite(VFD_stb, LOW);
                          delay(1);
                          //cmd3 Address 00H - 15H ( total of 11*2Bytes=176 Bits)
                          cmd_without_stb(0b11000000);// Start at 0x00 address to fill wells
                          delay(1); 
                                       if(j <= 7){
                                         (x=(v << 1) & 0x00FF);
                                            cmd_without_stb(x);
                                            cmd_without_stb(0b00000000);
                                            //Serial.print("x0: "); Serial.println(x, BIN); 
                                       }
                                         else{
                                           x=(v >> 7) & 0x00FF;
                                          //Serial.print("x1: "); Serial.println(x, BIN);
                                          cmd_without_stb(0b00000000);
                                          cmd_without_stb(x);
                                          //Serial.print("v: "); Serial.println(v, BIN);
                                         }
                       v=(v << 1);                
              
              delay(1);
              digitalWrite(VFD_stb, HIGH);
              //cmd4 set DIMM/PWM to value
              cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)//0 min - 7 max  )(0b01010000)
              delay(30);
            }
}
void setup() {
  // put your setup code here, to run once:
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  seconds = 0x00;
  minutes =0x00;
  hours = 0x00;
  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  // initialize timer1 
  cli();           // disable all interrupts
  // initialize timer1 
  //noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;// This initialisations is very important, to have sure the trigger take place!!!
  TCNT1  = 0;
  // Use 62499 to generate a cycle of 1 sex 2 X 0.5 Secs (16MHz / (2*256*(1+62449) = 0.5
  OCR1A = 62498;            // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= ((1 << CS12) | (0 << CS11) | (0 << CS10));    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
 
  
  // Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
  // a little the value 62499 upper or lower if the clock have a delay or advnce on hours.
   
  //  a=0x33;
  //  b=0x01;
  CLKPR=(0x80);
  //Set PORT
  DDRD = 0xFF;  // IMPORTANT: from pin 0 to 7 is port D, from pin 8 to 13 is port B
  PORTD=0x00;
  DDRB =0xFF;
  PORTB =0x00;
  pt6312_init();
  test_VFD();
  clear_VFD();
  //only here I active the enable of interrupts to allow run the test of VFD
  //interrupts();             // enable all interrupts
  sei();
}
/******************************************************************/
/************************** Update Clock **************************/
/******************************************************************/
void send_update_clock(void){
  if (secs >=60){
    secs =0;
    minutes++;
  }
  if (minutes >=60){
    minutes =0;
    hours++;
  }
  if (hours >=24){
    hours =0;
  }
    //*************************************************************
    DigitTo7SegEncoder(secs%10);
    //Serial.println(secs, DEC);
    numberA=segments[number];
    DigitTo7SegEncoder(secs/10);
    //Serial.println(secs, DEC);
    numberB=segments[number];
    SegTo32BitsSecs();
    //*************************************************************
    DigitTo7SegEncoder(minutes%10);
    numberC=segments[number];
    DigitTo7SegEncoder(minutes/10);
    numberD=segments[number];
    SegTo32BitsMinuts();
    //**************************************************************
    DigitTo7SegEncoder(hours%10);
    numberE=segments[number];
    DigitTo7SegEncoder(hours/10);
    numberF=segments[number];
    SegTo32BitsHrs();
    //**************************************************************
}
void SegTo32BitsHrs(){
  //Serial.println(number,HEX);
  // This block is very important, it solve the difference 
  // between segments from digit 1 and digit 2 from the same grid.
  // It is necessary because segment "a" is firts bit of byte one
  // and the other is the second bit of second byte.
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb((0b11000100) | grid); //cmd 3 wich define the start address (00H to 15H)
          // Here you can adjuste which grid represent the values of clock
          // each grid use 2 bytes of memory registers
          // the next two lines of code, tell the bytes are write at memory positions 0x00 and 0x01
          
          cmd_without_stb(numberF << 1); 
          cmd_without_stb(numberE << 1);    // Only this digit on the grid got a shift because the second  //  dozens
           
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delayMicroseconds(1);
}
void SegTo32BitsMinuts(){
  //Serial.println(number,HEX);
  // This block is very important, it solve the difference 
  // between segments from digit 1 and digit 2 from the same grid.
  // It is necessary because segment "a" is firts bit of byte one
  // and the other is the second bit of second byte.
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb(0b11000110); //cmd 3 wich define the start address (00H to 15H)
          // Here you can adjuste which grid represent the values of clock
          // each grid use 2 bytes of memory registers
          // the next two lines of code, tell the bytes are write at memory positions 0x06 and 0x07
          cmd_without_stb(numberD << 1); //Note we need apply the shift of one bit! 
          cmd_without_stb(numberC << 1); //Note we need apply the shift of one bit!    
           
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delayMicroseconds(1);
}
void SegTo32BitsSecs(){
  //Serial.println(number,HEX);
  // This block is very important, it solve the difference 
  // between segments from digit 1 and digit 2 from the same grid.
  // It is necessary because segment "a" is firts bit of byte one
  // and the other is the second bit of second byte.
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb(0b11001000); //cmd 3 wich define the start address (00H to 15H)
          // Here you can adjuste which grid represent the values of clock
          // each grid use 2 bytes of memory registers
          // the next two lines of code, tell the bytes are write at memory positions 0x00 and 0x01
          cmd_without_stb(numberB << 2);  // seconds dozens
          if(flag){
            numberA = (numberA << 1);
            numberA = (numberA | 0x01); //This line of code is to force the bit 8 of grid at right at presence of digits: 0,2,3,5,6,8!!!
            cmd_without_stb(numberA);
          }
          else{
             cmd_without_stb(numberA << 1);  // seconds units 
          }
          
          //
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delayMicroseconds(1);
}
void DigitTo7SegEncoder( unsigned char digit){
  switch(digit)
  {
    case 0:   number=0; flag = true;      break;  // if remove the LongX, need put here the segments[x]
    case 1:   number=1; flag = false;     break;
    case 2:   number=2; flag = true;      break;
    case 3:   number=3; flag = true;      break;
    case 4:   number=4; flag = false;     break;
    case 5:   number=5; flag = true;      break;
    case 6:   number=6; flag = true;      break;
    case 7:   number=7; flag = false;     break;
    case 8:   number=8; flag = true;      break;
    case 9:   number=9; flag = false;     break;
  }
} 
void readButtons(){
  //Take special attention to the initialize digital pin LED_BUILTIN as an output.
  int ledPin = 13;   // LED connected to digital pin 13
  int val = 0;       // variable to store the read value
  int dataIn=0;
  byte array[8] = {0,0,0,0,0,0,0,0};
  byte together = 0;
  unsigned char data = 0; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  array[0] = 1;
  //unsigned char btn1 = 0x41;
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(2);
      cmd_without_stb(0b01000010); // cmd 2 //Read Keys;Normal operation; Set pulse as 1/16
      
  pinMode(VFD_in, INPUT);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(2);
  //PORTD != B01010100; // this will set only the pins you want and leave the rest alone at
  //their current value (0 or 1), be careful setting an input pin though as you may turn 
  //on or off the pull up resistor  
  //This don't send the strobe signal, to be used in burst data send
         for (int z = 0; z < 3; z++){
                   for (int h = 8; h > 0; h--) {
                      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
                      delayMicroseconds(2);
                     val = digitalRead(VFD_in);
                     
                           if (val & mask){ // if bitwise AND resolves to true
                            //Serial.print(val);
                            //data =data | (1 << mask);
                            array[h] = 1;
                           }
                           else{ //if bitwise and resolves to false
                            //Serial.print(val);
                            //data = data | (1 << mask);
                           array[h] = 0;
                           }
                    digitalWrite(VFD_clk, LOW);
                    delayMicroseconds(2);
                   } 
               Serial.print(z);  // All the lines of print is only used to debug, comment it, please!
               Serial.print(" - " );
                                  for (int bits = 7 ; bits > -1; bits--) {
                                      Serial.print(array[bits]);
                                   }
                        if (z==0){
                          if(array[7] == 1){
                           hours++;
                          }
                        }
                        if (z==0){
                          if(array[6] == 1){
                           hours--;
                          }
                          }
                        if (z==0){
                          if(array[5] == 1){
                           minutes++;
                          }
                        }
                        if (z==0){
                          if(array[4] == 1){
                           minutes--;
                          }
                        }
                        if (z==1){
                          if(array[3] == 1){
                           secs++;
                          }
                        }
                        if (z==0){
                          if(array[1] == 1){
                              hours = 0;
                              minutes = 0;
                             secs=0;  // Set count of secs to zero to be more easy to adjust with other clock.
                          }
                        }    
                  Serial.println();
          }  // End of "for" of "z"
      Serial.println();  // This line is only used to debug, please comment it!
 digitalWrite(VFD_stb, HIGH);
 delayMicroseconds(2);
 cmd_with_stb((0b10001000) | 7); //cmd 4
 delayMicroseconds(2);
 pinMode(VFD_in, OUTPUT);  // Important this point!  // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
 delay(1); 
}
void tst (void){
  uint8_t m = 0x00;
        
  for(uint8_t g = 0x00; g < 16; g=g+2){
    m=0x00;
                for (int s = 0x00; s < 16; s++){
                  cmd_with_stb(0b00000010); //       cmd 1 // 6 Grids & 16 Segments
                  cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
                  digitalWrite(VFD_stb, LOW);
                  delayMicroseconds(1);
                  //
                  cmd_without_stb((0b11000000 | g)); //cmd 3 wich define the start address (00H to 15H)

                      if(s < 8){
                        cmd_without_stb(0b00000001 << s); // Data to fill table 5*16 = 80 bits
                        cmd_without_stb(0b00000000); // Data to fill table 5*16 = 80 bits
                         Serial.println("s: "); Serial.println(s, BIN);
                      }
                      else{
                        cmd_without_stb(0b00000000); // Data to fill table 5*16 = 80 bits
                        cmd_without_stb(0b00000001 << m); // Data to fill table 5*16 = 80 bits
                        m++;
                          Serial.println("m: "); Serial.println(m, BIN);
                      }
                  delay(1);
                  digitalWrite(VFD_stb, HIGH);
                  //cmd4 set DIMM/PWM to value
                  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)//0 min - 7 max  )(0b01010000)      
                  delay(50);
                }
             
  }           
}
void segAndGrid(void){
                  cmd_with_stb(0b00000010); //       cmd 1 // 6 Grids & 16 Segments
                  cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
                  digitalWrite(VFD_stb, LOW);
                  delayMicroseconds(1);
                  //
                  cmd_without_stb((0b11000000 | 0x00)); //cmd 3 wich define the start address (00H to 15H). on this panel we use: 00, 02, 04, 06, 08, 0A.
  //                                        76543210
                          cmd_without_stb(0b11111110); // The weels have 13 segments
  //                                        FEDCBA98
                          cmd_without_stb(0b00111111); // The weels have 13 segments
                      
                  delay(1);
                  digitalWrite(VFD_stb, HIGH);
                  //cmd4 set DIMM/PWM to value
                  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)//0 min - 7 max  )(0b01010000)      
                  delay(50);
}
void loop() {
  // You can comment untill while cycle to avoid so many tests are running.
   wrLED();
   test_VFD();
   clear_VFD();
   
    segAndGrid();
    
      test_VFD_grid();
      delay(500);
      clear_VFD();

      tst();
    
      clear_VFD();
        for(uint8_t i = 0x00; i < 4; i++){
          test_VFD_chkGrids();
          delay(500);
        }
    
  clear_VFD();
    
    while(1){
        send_update_clock();
        //delay(150);
        readButtons();
        //delay(150);
        PT6312_RunWeels();
    }
}
ISR(TIMER1_COMPA_vect)   {  //This is the interrupt request
                            // https://sites.google.com/site/qeewiki/books/avr-guide/timers-on-the-atmega328
      secs++;
} 
