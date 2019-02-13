/*
 * 1REALPROJECT.c
 *
 * Created: 3/15/2018 12:08:12 AM
 * Author : Thomas
 */ 

#include <avr/io.h>
#include "timer.h"
#include "io.c"
#include <avr/eeprom.h>
#include <util/delay.h>


#define F_CPU 1000000UL

//--------Shared Variables----------------------------------------------------
unsigned char string[] = {1, ' ', ' ', ' ', ' ', 1, ' ', ' ', ' ', ' '};
unsigned char stringsize = 10;
unsigned char losestring[] = "You Lose!";
unsigned char losesize = 9;
unsigned char count = 25;
int i;
unsigned char jmpcnt = 0;
unsigned char blink = 0;

uint8_t currentscore = 0x00;
uint8_t highscore;

//float currentscore;
//float highscore;

unsigned char cactus[] = {0x4, 0x4, 0x4, 0x4, 0x15, 0x15, 0x15, 0x1f};
unsigned char dino[] = {0x7, 0x7, 0x1c, 0x1f, 0x1c, 0x1c, 0x14, 0x14};
unsigned char smiley[] = {0x0, 0xa, 0x0, 0x2, 0x1, 0x12, 0x8, 0x6};
unsigned char but1;
unsigned char but2;
unsigned char but3;
int pos = 17;

unsigned char shiftregister = 0;
unsigned char zeroled = 0x00;
unsigned char oneled = 0x01;
unsigned char twoled = 0x03;
unsigned char threeled = 0x07;

//-----FLAGS-------
unsigned char gameone = 1;
unsigned char gametwo = 0;
unsigned char gamethree = 0;
unsigned char clearscreen = 0;
unsigned char levelcounter = 0;

//--------End Shared Variables------------------------------------------------


//--------Shift Register transmit_data function -------------------------------
void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTB |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}

//--------End Shift Register transmit_data function --------------------------

//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}
//--------End find GCD function ---------------------------------------------

//--------Task scheduler data structure--------------------------------------


typedef struct _task {
	//Task's current state, period, and the time elapsed
	// since the last tick
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	//Task tick function
	int (*TickFct)(int);
} task;

//---------End Task scheduler data structure----------------------------------

//---------Begin state machines-----------------------------------------------
enum SM1_States{init, waitone, menu, playone, endone, resethighscore};
 
int SMTick1(int state){
	but1 = ~PINA & 0x01;
	but2 = ~PINA & 0x02;
	but3 = ~PINA & 0x04;

	switch(state) { //Transitions
		case init:
		state = waitone;
		break;


		case waitone:
		LCD_ClearScreen();
		if(gameone)
		{
			state = menu;
		}
		else
		{
			state = waitone;
		}
		break;

		case menu:
		if(but1)
		{
			state = playone;
		}
		else if(but2)
		{
			state = init;
		}
		else if(but3)
		{
			state = resethighscore;
		}
		else
		{
			state = menu;
		}
		break;
	
		case playone:
		if(but2)
		{	
			levelcounter = 0;
			count = 25;
			state = init;
		}
		else if(!but2)
		{	
			if((pos==count))
			{
				state = endone;
			}
			else if(levelcounter > 45)
			{
				gameone = 0;
				gametwo = 1;
				levelcounter = 0;
				count = 25;
				state = waitone;
			}
			else
			{
				state = playone;
			}
		}
		break;

		case endone:
		state = init;
		break;

		case resethighscore:
		state = init;
		break;

		default:
		state = init;
		break;

	}

	switch(state) { //Actions
		case init:
		//PORTB = 0x00;
		LCD_ClearScreen();
		break;

		case waitone:
		break;

		case resethighscore:
		highscore = 0;
		eeprom_update_byte((uint8_t*)46, highscore);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Resetting high score");
		break;
		case menu:
		//PORTB = 0x00;
		transmit_data(zeroled);
		LCD_Cursor(1);
		LCD_WriteData(2);
		LCD_DisplayString(4, "Dino Jump");
		LCD_Cursor(14);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_DisplayString(17, " Press to start!");
		break;

		case playone:
		//PORTB = 0x01;
		transmit_data(oneled);
		if(clearscreen == 0)
		{
			LCD_ClearScreen();
		}
		levelcounter++;
		if(count == 17)
		{
			currentscore++;
		}
		//currentscore++;

		
		LCD_Cursor(1);
		LCD_WriteData(0x20);
		

		
		
		if(count > 17)
		{
			count--;
		}
		else if(count == 17)
		{
			count = 25;
		}

		if(but1 && jmpcnt == 0)
		{
			pos = 1;
			jmpcnt++;

		}
		else if(but1 && jmpcnt > 0)
		{
			pos = 1;
			jmpcnt++;

			if(jmpcnt == 4)
			{
				pos = 17;
				jmpcnt = 0;
			}
		}
		else
		{
			pos = 17;
			jmpcnt = 0;
		}
		
		

		if(count != 17)
		{
			LCD_Cursor(17);
			LCD_WriteData(0x20);
		}

		LCD_Cursor(pos);
		LCD_WriteData(2);

		LCD_Cursor(count);
		for(i = 0; i < stringsize; ++i)
		{
			LCD_WriteData(string[i]);
			LCD_WriteData(0x20);
		}

		/*
		if((pos==count) || (pos == count1 + 3))
		{
			if(currentscore > highscore)
			{
				highscore = currentscore;
				eeprom_update_byte((uint8_t*)46, highscore);
			}
			
			LCD_ClearScreen();
			LCD_Cursor(1);
			for(i = 0; i < losesize; ++i)
			{
				LCD_WriteData(losestring[i]);
			}
			delay_ms(3000);
			highscore = eeprom_read_byte((uint8_t*)46);
			LCD_ClearScreen();
			LCD_DisplayString(1, "High Score:");
			LCD_Cursor(17);
			int highscoreint = (int)highscore;
			LCD_WriteData(highscoreint + '0');

			delay_ms(3000);
			currentscore = 0;
			LCD_ClearScreen();
		}
		*/

		break;

		case endone:
		//PORTB = 0x00;
		transmit_data(zeroled);
		clearscreen = 0;
		if(currentscore > highscore)
		{
			highscore = currentscore;
			eeprom_update_byte((uint8_t*)46, highscore);
		}
		
		LCD_ClearScreen();
		LCD_Cursor(1);
		for(i = 0; i < losesize; ++i)
		{
			LCD_WriteData(losestring[i]);
		}
		delay_ms(4000);
		highscore = eeprom_read_byte((uint8_t*)46);
		LCD_ClearScreen();
		LCD_DisplayString(1, "High Score:");
		LCD_Cursor(17);
		int highscoreint = (int)highscore;
		LCD_WriteData(highscore + '0');

		delay_ms(4000);
		currentscore = 0;
		LCD_ClearScreen();
		break;
		
	}



	return state;
}




enum SM2_States{inittwo, waittwo, menutwo, playtwo, endtwo};
 
int SMTick2(int state){
	but1 = ~PINA & 0x01;
	but2 = ~PINA & 0x02;

	switch(state) { //Transitions
		case inittwo:
		state = waittwo;
		break;


		case waittwo:
		if(gametwo)
		{
			state = menutwo;
		}
		else
		{
			state = waittwo;
		}
		break;

		case menutwo:
		/*
		if(but1)
		{
			state = playtwo;
		}
		else
		{
			state = menutwo;
		}
		*/
		if(but2)
		{
			gameone = 1;
			gametwo = 0;
			gamethree = 0;
			levelcounter = 0;
			count = 25;
			state = waittwo;
		}
		else
		{
			state = playtwo;
		}
		//state = playtwo;
		break;
	
		case playtwo:
		if(but2)
		{
			gameone = 1;
			gametwo = 0;
			gamethree = 0;
			levelcounter = 0;
			count = 25;
			state = waittwo;
		}
		else if(!but2)
		{
			if((pos==count))
			{
				state = endtwo;
			}
			
			else if(levelcounter > 75)
			{
				gameone = 0;
				gametwo = 0;
				gamethree = 1;
				levelcounter = 0;
				count = 25;
				state = waittwo;
			}
			
			else
			{
				state = playtwo;
			}
		}
		break;

		case endtwo:
		gameone = 1;
		gametwo = 0;
		gamethree = 0;
		levelcounter = 0;
		state = inittwo;
		break;

		default:
		state = inittwo;
		break;

	}

	switch(state) { //Actions
		case inittwo:
		//PORTB = PORTB;
		LCD_ClearScreen();
		break;

		case waittwo:
		//LCD_ClearScreen();
		break;

		case menutwo:
		//PORTB = 0x00;
		transmit_data(zeroled);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Level 2!");
		LCD_DisplayString(17, "Starts in 3...");
		delay_ms(2000);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Level 2!");
		LCD_DisplayString(17, "Starts in 2...");
		delay_ms(2000);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Level 2!");
		LCD_DisplayString(17, "Starts in 1...");
		delay_ms(2000);
		LCD_ClearScreen();
		break;

		case playtwo:
		//PORTB = 0x03;
		transmit_data(twoled);
		if(clearscreen == 0)
		{
			LCD_ClearScreen();
		}
		levelcounter++;

		if(count == 17)
		{
			currentscore++;
		}
		//currentscore++;

		
		if(pos != 1)
		{
		LCD_Cursor(1);
		LCD_WriteData(0x20);
		}
		

		
		
		if(count > 17)
		{
			count--;
		}
		else if(count == 17)
		{
			count = 25;
		}

		if(but1 && jmpcnt == 0)
		{
			pos = 1;
			jmpcnt++;

		}
		else if(but1 && jmpcnt > 0)
		{
			pos = 1;
			jmpcnt++;

			if(jmpcnt == 4)
			{
				pos = 17;
				jmpcnt = 0;
			}
		}
		else
		{
			pos = 17;
			jmpcnt = 0;
		}
		
		

		if(count != 17)
		{
			LCD_Cursor(17);
			LCD_WriteData(0x20);
		}

		LCD_Cursor(pos);
		LCD_WriteData(2);

		LCD_Cursor(count);
		for(i = 0; i < stringsize; ++i)
		{
			LCD_WriteData(string[i]);
			LCD_WriteData(0x20);
		}

		/*
		if((pos==count) || (pos == count1 + 3))
		{
			if(currentscore > highscore)
			{
				highscore = currentscore;
				eeprom_update_byte((uint8_t*)46, highscore);
			}
			
			LCD_ClearScreen();
			LCD_Cursor(1);
			for(i = 0; i < losesize; ++i)
			{
				LCD_WriteData(losestring[i]);
			}
			delay_ms(3000);
			highscore = eeprom_read_byte((uint8_t*)46);
			LCD_ClearScreen();
			LCD_DisplayString(1, "High Score:");
			LCD_Cursor(17);
			int highscoreint = (int)highscore;
			LCD_WriteData(highscoreint + '0');

			delay_ms(3000);
			currentscore = 0;
			LCD_ClearScreen();
		}
		*/
	

		break;

		case endtwo:
		//PORTB = 0x00;
		transmit_data(zeroled);
		clearscreen = 0;
		if(currentscore > highscore)
		{
			highscore = currentscore;
			eeprom_update_byte((uint8_t*)46, highscore);
		}
		
		LCD_ClearScreen();
		LCD_Cursor(1);
		for(i = 0; i < losesize; ++i)
		{
			LCD_WriteData(losestring[i]);
		}
		delay_ms(4000);
		highscore = eeprom_read_byte((uint8_t*)46);
		LCD_ClearScreen();
		LCD_DisplayString(1, "High Score:");
		LCD_Cursor(17);
		int highscoreint = (int)highscore;
		LCD_WriteData(highscore + '0');

		delay_ms(4000);
		currentscore = 0;
		LCD_ClearScreen();
		break;
		
	}



	return state;
}



enum SM3_States{init3, wait3, menu3, play3, end3};
 
int SMTick3(int state){
	but1 = ~PINA & 0x01;
	but2 = ~PINA & 0x02;

	switch(state) { //Transitions
		case init3:
		state = wait3;
		break;


		case wait3:
		if(gamethree)
		{
			state = menu3;
		}
		else
		{
			state = wait3;
		}
		break;

		case menu3:
		/*
		if(but1)
		{
			state = playtwo;
		}
		else
		{
			state = menutwo;
		}
		*/
		state = play3;
		break;
	
		case play3:
		if(but2)
		{
			gameone = 1;
			gametwo = 0;
			gamethree = 0;
			levelcounter = 0;
			count = 25;
			state = wait3;
		}
		else if(!but2)
		{
			if((pos==count))
			{
				state = end3;
			}
			/*
			else if(levelcounter > 15)
			{
				gameone = 0;
				gametwo = 1;
				levelcounter = 0;
				state = waittwo;
			}
			*/
			else
			{
				state = play3;
			}
		}
		break;

		case end3:
		gameone = 1;
		gametwo = 0;
		gamethree = 0;
		levelcounter = 0;
		count = 25;
		state = init3;
		break;

		default:
		state = init3;
		break;

	}

	switch(state) { //Actions
		case init3:
		LCD_ClearScreen();
		break;

		case wait3:
		//LCD_ClearScreen();
		break;

		case menu3:
		//PORTB = 0x00;
		transmit_data(zeroled);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Final level!");
		LCD_DisplayString(17, "Starts in 3...");
		delay_ms(2000);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Final level!");
		LCD_DisplayString(17, "Starts in 2...");
		delay_ms(2000);
		LCD_ClearScreen();
		LCD_DisplayString(1, "Final level!");
		LCD_DisplayString(17, "Starts in 1...");
		delay_ms(2000);
		LCD_ClearScreen();
		break;

		case play3:
		//PORTB = 0x07;
		transmit_data(threeled);
		if(clearscreen == 0)
		{
			LCD_ClearScreen();
		}
		levelcounter++;

		if(count == 17)
		{
			currentscore++;
		}
		//currentscore++;

		
		if(pos != 1)
		{
		LCD_Cursor(1);
		LCD_WriteData(0x20);
		}

		
		
		if(count > 17)
		{
			count--;
		}
		else if(count == 17)
		{
			count = 25;
		}

		if(but1 && jmpcnt == 0)
		{
			pos = 1;
			jmpcnt++;

		}
		else if(but1 && jmpcnt > 0)
		{
			pos = 1;
			jmpcnt++;

			if(jmpcnt == 4)
			{
				pos = 17;
				jmpcnt = 0;
			}
		}
		else
		{
			pos = 17;
			jmpcnt = 0;
		}
		
		

		if(count != 17)
		{
			LCD_Cursor(17);
			LCD_WriteData(0x20);
		}

		LCD_Cursor(pos);
		LCD_WriteData(2);

		LCD_Cursor(count);
		for(i = 0; i < stringsize; ++i)
		{
			LCD_WriteData(string[i]);
			LCD_WriteData(0x20);
		}

		/*
		if((pos==count) || (pos == count1 + 3))
		{
			if(currentscore > highscore)
			{
				highscore = currentscore;
				eeprom_update_byte((uint8_t*)46, highscore);
			}
			
			LCD_ClearScreen();
			LCD_Cursor(1);
			for(i = 0; i < losesize; ++i)
			{
				LCD_WriteData(losestring[i]);
			}
			delay_ms(3000);
			highscore = eeprom_read_byte((uint8_t*)46);
			LCD_ClearScreen();
			LCD_DisplayString(1, "High Score:");
			LCD_Cursor(17);
			int highscoreint = (int)highscore;
			LCD_WriteData(highscoreint + '0');

			delay_ms(3000);
			currentscore = 0;
			LCD_ClearScreen();
		}
		*/
	

		break;

		case end3:
		//PORTB = 0x00;
		transmit_data(zeroled);
		clearscreen = 0;
		if(currentscore > highscore)
		{
			highscore = currentscore;
			eeprom_update_byte((uint8_t*)46, highscore);
		}
		
		LCD_ClearScreen();
		LCD_Cursor(1);
		for(i = 0; i < losesize; ++i)
		{
			LCD_WriteData(losestring[i]);
		}
		delay_ms(4000);
		highscore = eeprom_read_byte((uint8_t*)46);
		LCD_ClearScreen();
		LCD_DisplayString(1, "High Score:");
		LCD_Cursor(17);
		int highscoreint = (int)highscore;
		LCD_WriteData(highscore + '0');

		delay_ms(4000);
		currentscore = 0;
		LCD_ClearScreen();
		break;
		
	}



	return state;
}


//------END STATE MACHINES-------------





int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	
	// Period for the tasks
	//unsigned long int highscore_calc = 200;
	unsigned long int SMTick1_calc = 400;
	unsigned long int SMTick2_calc = 200;
	unsigned long int SMTick3_calc = 100;
	//unsigned long int SMTick4_calc = 10;

	//Calculating GCD
	unsigned long int tmpGCD = SMTick1_calc;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	//tmpGCD = findGCD(tmpGCD, SMTick4_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	//unsigned long int SMTick4_period = SMTick4_calc/GCD;


	//Declare an array of tasks
	static task task1, task2, task3;
	//static task task1, task2;
	task *tasks[] = {&task1, &task2, &task3};
	//task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//task period.
	task1.elapsedTime = SMTick1_period;//task current elapsed time
	task1.TickFct = &SMTick1; //Function pointer for the tick

	
	// Task 2
	task2.state = -1;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &SMTick2;//Function pointer for the tick.

	// Task 3
	task3.state = -1;//Task initial state.
	task3.period = SMTick3_period;//Task Period.
	task3.elapsedTime = SMTick3_period; // Task current elasped time.
	task3.TickFct = &SMTick3; // Function pointer for the tick.

	/*

	// Task 4
	task4.state = -1;//Task initial state.
	task4.period = SMTick4_period;//Task Period.
	task4.elapsedTime = SMTick4_period; // Task current elasped time.
	task4.TickFct = &SMTick4; // Function pointer for the tick.

	*/

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();

	LCD_Build(1, cactus);
	LCD_Build(2, dino);

	LCD_init();
	LCD_ClearScreen();

	shiftregister = 0x07;

    unsigned short i; // Scheduler for-loop iterator
    while(1) {
	    // Scheduler code
	    for ( i = 0; i < numTasks; i++ ) {
		    // Task is ready to tick
		    if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			    // Setting next state for task
			    tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			    // Reset the elapsed time for next tick.
			    tasks[i]->elapsedTime = 0;
		    }
		    tasks[i]->elapsedTime += 1;
	    }
	    while(!TimerFlag);
	    TimerFlag = 0;
    }

	// Error: Program should not exit!
	return 0;
}

