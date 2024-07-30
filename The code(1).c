#define F_CPU 8000000
#include <util/delay.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include<string.h>
#include<stdint.h>
char keypad_get_key(void); //prototype keypad function
void my_delay(int ms);


volatile char pressed_key = '\0'; //global variable pressed key


ISR(INT0_vect) { //interrupt to get key pressed
    pressed_key = keypad_get_key();
   my_delay(20); // Debounce delay
}

char keypad_get_key(void) { //function te get key pressed
    char keys[4][4] = { //values of char in keypad
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    unsigned char encoder_output = (PIND & 0xF0) >> 4; // Read the upper 4 bits of PORTD
    unsigned char row = (encoder_output & 0b1100) >> 2; //know the row char
    unsigned char col = encoder_output & 0b0011; //get the column char

    return keys[row][col]; //return values of row column to know the keys pressed
}


void epulse() //epulse to enable lcd and have pulse in it
{PORTB|=(1<<5); //rs off :command mode
   my_delay(5); 
   PORTB&=~(1<<5); //rs: data mode
   my_delay(5);
}
void sendchar(char D) //function to send char to LCD
{
   PORTB&=0xF0; //get 4 msb
   PORTB|=(D>>4); //shif 4 msb
   epulse(); //send enable
  
   PORTB&=0xF0; //  4 lsb 
   PORTB|=(D&0x0F); // let port b equal the 4 lsb
   epulse(); //send enaple pulse
}
void resetLCD() // function to reset lcd
{
   PORTB&=~(1<<4); //rs command mode 
   sendchar(0x03);  //values from slides
   sendchar(0x03);
   sendchar(0x03);
   sendchar(0x02);
   PORTB|=(1<<4);} // rs data mode
   void initLCD() //function to set up lcd
   {
      PORTB&=~(1<<4); //rs command mode
      sendchar(0x28); //values from slides
      sendchar(0x0C);
      sendchar(0x06);
      PORTB|=(1<<4); //rs data mode
   }
   void cleardisplay() //function to clear the display of lcd
   {
      PORTB&=~(1<<4); //rs command mode
      sendchar(0x01); // value to clear lcd
      PORTB|=(1<<4); //rs data mode
   }

   void DisplayText (char string[16], uint8_t LineNo) //function to display the text
{
      int len,count;
      PORTB &= ~(1<<4);	// Switch to Command mode
      if(LineNo==1)
      sendchar(0x80); 	// Move cursor to Line 1
      else
      sendchar(0xC0); 	// Move cursor to Line 2
      PORTB |= (1<<4);           // Switch to Data mode	
      len = strlen(string);
      for (count=0; count<len; count++) //loop to sedd the text
		sendchar(string[count]);	
}

void my_delay(int ms) //function of delay
{
int i; 
for(i=0; i< ms ; i++) //loop to get the delay wanted
_delay_ms(1);
}
//---------------------------------------------------
void initUART()
{            /* Student: set registries for baud rate=2400 */
UBRRH = 0;// high byte of UBRR
UBRRL = 208;// low byte of UBRR
UCSRC = 0x86;// Mode
UCSRB = 0x18;// enable RX and TX
}

void sendByte(char x)
{
    while((UCSRA & 0x20) == 0); //ready to transmit

    UDR = x; // UDR = the character sent

}
void print_string(char *s)  //function to get the text of vertual terminal
{
int i;
for (i = 0; s[i] != '\0'; i++) 
sendByte(s[i]);
}
void print_decimal(unsigned char x)//function to get the decimal of vertual terminal
{
char h, t;// get the msb and middel and lsb of the number
h = x / 100;
x = x - h * 100;
t = x / 10;
x = x - t * 10;
if (h > 0) {
sendByte(h + '0');
sendByte(t + '0');
sendByte(x + '0');
} else if (t > 0) {
sendByte(t + '0');
sendByte(x + '0');
} else
sendByte(x + '0');
}
void print_newline() //function to print line in the vertiual terminal
{
sendByte(10);
sendByte(13);
}


int main() { //main code
    DDRB=0xFF; //set port b to output
DDRC=0xF8; //set port c ob11111000  
    DDRD &= 0x0F;  //set port d ob00001111 
    resetLCD(); //call reset the function of lcd
    initLCD(); //set up the lcd
   unsigned char temperature; //variable to calculate temperature
  
    // Configure PORTD for encoder input

   

    // Configure INT0 interrupt
    MCUCR |= (1 << ISC01); //intilize interrupt for keypad int 0
    GICR |= (1 << INT0); //enable interrupt int0
    sei(); //enable global interrupt
 initUART();

    while (1) { // main loop always true
  if (pressed_key != '\0') { //if function to know if any char is pressed
     while(pressed_key=='1'){ //if function to display temp if  char 1 is pressed

    ADMUX = (1 << ADLAR)|(1<<MUX0); //set ad mux to left aligned and pc1
    ADCSRA  = 0x84; // enable adc 
        ADCSRA |= (1<<ADSC); //start transfaring

        while((ADCSRA&(1<<ADIF))==0); //check stop conversion
        ADCSRA |= (1<<ADIF); 



temperature = (1.964*ADCH) + 1.1373;//calculating temperature
  print_string("Temperature : ");
        print_decimal(temperature);
       print_newline();

	if (temperature>24.0)  //check room temp
	   PORTC|=(1<<3); //turn on ac if temp higher than 24
	else
	   PORTC&=~(1<<3); //turn ac off

        // Display sensor reading on LCD
        DisplayText("Temperature:", 1); //display temperature on lcd
       
        sendchar((temperature / 100) + '0'); //msb digits to ascii
       sendchar(((temperature % 100) / 10) + '0'); //second digits to ascii
      sendchar((temperature % 10) + '0'); //lsb digits to ascii
        my_delay(1000); //call delay

	cleardisplay();
	        PORTC&=~(1<<4); //turn off water system
		        PORTC&=~(1<<5); //turn off warning

	break;
	
     }
     
  while(pressed_key=='2'){
	ADMUX = (1 << ADLAR); //mux = pc0, left aligned
    ADCSRA  = 0x84;
         ADCSRA |= (1<<ADSC); //enable adc
        PORTC&=~(1<<3); //turn off temperature LED

        while((ADCSRA&(1<<ADIF))==0); //start transfaring information of the analog sensor
        ADCSRA |= (1<<ADIF); //stop transfaring
	
	

	unsigned char humidity=((0.617*ADCH)-23.52); //calculating humidity using the equation found on exel
	  print_string("Humidity : ");
      print_decimal(humidity);
      print_newline();


	if (humidity < 40) {
            PORTC |= (1 << 4); // Turn on the watering system
        } else {
            PORTC &= ~(1 << 4); // Turn off the watering system
        }
	if(humidity<20){
            PORTC |= (1 << 5); // Turn on the warning
        } else {
            PORTC &= ~(1 << 5); // Turn off the warning
        }

       DisplayText("humidity :",1);  //display humidity text
       sendchar((humidity / 100) +'0' ); //msb digits to ascii
       sendchar((humidity % 100/10)+'0' ); //second digit to ascii
       sendchar((humidity % 10)+'0' ); //lsb digit to ascii

       my_delay(1000); //delay function
 
	cleardisplay(); //clear lcd
	
break; //break from loop
    }while(pressed_key=='3'){
     ADMUX = (1 << ADLAR)|(1<<MUX0); //set ad mux to left aligned and pc1
    ADCSRA  = 0x84; // enable adc 
        ADCSRA |= (1<<ADSC); //start transfaring

        while((ADCSRA&(1<<ADIF))==0); //check stop conversion
        ADCSRA |= (1<<ADIF); 



unsigned char x = (1.964*ADCH) + 1.1373;//calculating temperature


	if (x>24.0)  //check room temp
	   PORTC|=(1<<3); //turn on ac if temp higher than 24
	else
	   PORTC&=~(1<<3); //turn ac off

        // Display sensor reading on LCD
        DisplayText("Monitoring Mode", 1); //display temperature on lcd
my_delay(1000);
       cleardisplay();

           
	ADMUX = (1 << ADLAR); //mux = pc0, left aligned
    ADCSRA  = 0x84;
         ADCSRA |= (1<<ADSC); //enable adc
       

        while((ADCSRA&(1<<ADIF))==0); //start transfaring information of the analog sensor
        ADCSRA |= (1<<ADIF); //stop transfaring
	
	

	unsigned char humidity=((0.617*ADCH)-23.52); //calculating humidity using the equation found on exel
	


	if (humidity < 40) {
            PORTC |= (1 << 4); // Turn on the watering system
        } else {
            PORTC &= ~(1 << 4); // Turn off watering system
        }
	if(humidity<20){
            PORTC |= (1 << 5); // Turn on the warning
        } else {
            PORTC &= ~(1 << 5); // Turn off the warning
        }

       
	break; //break of the loop
       
 }}
       
 }
    return 0;
    
} //end of code

//exel for function humidity
/*38	0
41	2
45	4
48	6
51	8
54	10
57	12
61	14
64	16
67	18
70	20
74	22
77	24
80	26
84	28
87	30
90	32
93	34
97	36
100	38
103	40
106	42
109	44
113	46
116	48
119	50
123	52
126	54
129	56
132	58
136	60
139	62
142	64
145	66
148	68
152	70
155	72
158	74
161	76
164	78
168	80
171	82
174	84
177	86
181	88
184	90
187	92
190	94
194	96
197	98
200	100



exel for function for temprature
0	0
1	3
2	6
3	9
6	12
7	15
9	18
10	21
12	24
13	27
15	30
16	33
18	36
19	39
21	42
22	45
24	48
25	51
27	54
28	57
30	60
32	63
33	66
35	69
36	72
38	75
39	78
41	81
42	84
44	87
45	90
47	93
48	96
50	99
51	102
53	105
54	108
56	111
57	114
59	117
60	120
62	123
64	126
65	129
67	132
68	135
70	138
71	141
73	144
74	147
76	150
*/
