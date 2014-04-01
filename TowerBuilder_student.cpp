// ******************************************************************************************* //
//  File Name  : TowerBuilder.cpp
//  Description: This is the main file for the TowerBuilder Project.
//				 This file contains the main function (task scheduler), object declarations,
//                  a simple line tracking function, and the interrupt functions.
//
// ******************************************************************************************* //

#include "Global.h"

//Simple line tracking function
void lineTracking();


//Menu function
void menu();

void first();
void RightTurn();
//void TurnTil();
void LeftTurn();
void CheckStop();
void Forward();
void BackToLine();

//Variables to control different tasks in the scheduler
volatile unsigned char taskNumber = 0;
volatile unsigned char taskDone = TRUE;
volatile unsigned char updateChannel = FALSE;

//variables declared by us
volatile unsigned char condition = 0;
volatile unsigned char nextCondition = 0;
volatile unsigned char check1 = 0;
volatile unsigned char check2 = 0;
volatile unsigned char check3 = 0;

//variables for crossing
volatile unsigned char SPEED = 160;
volatile unsigned char crossing = 0;
volatile unsigned char leftSpeed = SPEED;
volatile unsigned char rightSpeed = SPEED;

//Hardware Objects
Timer3		t3;
Servo		servo;
Hardware	hardware;
Sensor		sensor;
LCD			lcd;
ADConverter adc;
Button		button;
Wheels		wheels;

int main(void)
{

	POWEROFF();
	delayU(100);
	POWERUP();

	lcd.setLineOne("Welcome to");
	lcd.setLineTwo("TowerBuilder");
	sei();        //Enable global interrupt

	while(1)
	{
		//Send 1 character to LCD if LCD is ready
		if(lcd.ready())
		lcd.print();

		//Task scheduler
		if(taskDone == FALSE) {

			switch(taskNumber++) {

				case 0:
				button.readButtons();
				menu();
				break;

				case 1:
				adc.convertPOT();
				break;

				case 2:
				sensor.readSensors();
				break;

				case 3:
				first();
				break;

				case 4:

				break;

				case 5:

				break;

				case 6:

				break;

				default:
				taskNumber = 0;
				break;
			}

			taskDone = TRUE;

			//Update servo's channel after each task finished
			while(updateChannel == FALSE);
			servo.servoPWM();
			updateChannel = FALSE;

		}
	}
}

void fake_turnRight() {
	while(1) {
		wheels.turnRight(150,150);
		delayU(500);
		if(sensor.middle_sensor == 0xff) {
			break;
		}
	}
}

void first(){
    switch(condition) {
        case 0:
		lineTracking();
        switch(sensor.middle_sensor) {
            case 0b00001111:
            case 0b00011111:
            case 0b00111111:
            case 0b01111111:
            case 0b00011110:
            case 0b00111110:
                condition = 1;
        }
        break;

        case 1:
		wheels.stop();
		RightTurn();
        break;

        case 2: CheckStop(); break;

        case 3: Forward(); break;
        case 4: BackToLine(); break;
        case 5:
		lineTracking();
		if(sensor.front_sensor == 0b00111100 || sensor.front_sensor == 0b00011100 || sensor.front_sensor == 0b00111000)
		{

			wheels.forward(0,0);
			condition = 6;
		}
        break;
    }
}
/*void TurnTil() {
	lineTracking();
	if((sensor.front_sensor & 0b00111100)== 0){
		wheels.stop();
		condition = nextCondition;
	}
}

*/
void logchr(unsigned char i) {
    switch(nextCondition) {
        case 1: check1 = i; break;
        case 2: check2 = i; break;
        case 3: check3 = i; break;
    }
}

void calibrateBySideSensor() {
	unsigned char ADJ = 15;

    unsigned char S = sensor.front_sensor;
    unsigned char Left = S & 0b1100000;
    unsigned char Right = S & 0b00000011;

    if(Left != 0 && Right == 0) {
        leftSpeed = SPEED - ADJ;
        rightSpeed = SPEED + ADJ;
        logchr(0x01);
    } else if(Left == 0 && Right != 0) {
        leftSpeed = SPEED + ADJ;
        rightSpeed = SPEED - ADJ;
        logchr(0x02);
    } else {
        leftSpeed = SPEED;
        rightSpeed = SPEED;
        logchr(0x03);
    }
}

void Forward(){
	wheels.forward(leftSpeed, rightSpeed);
    if(crossing == 0 && (sensor.front_sensor & 0b11000011) != 0) {
		nextCondition++;
		crossing = 1;
        calibrateBySideSensor();
    } else if(crossing == 1 && (sensor.front_sensor & 0b11000011) == 0) {
		crossing = 0;
	}
	if (nextCondition >= 4 && sensor.middle_sensor !=0)
	{
		wheels.forward(0,0);
		condition = 4;

		// Be a good citizen, Reset global variable
		leftSpeed = SPEED;
		rightSpeed = SPEED;
		crossing = 0;
	}
}
void CheckStop() {
	if(sensor.back_sensor != 0x00) {
		wheels.backward(120,120);
	}
	if(sensor.middle_sensor != 0xFF){
		wheels.turnLeft(175,165);
	}

	condition = 3;
}



void RightTurn(){
	wheels.turnRight(180,180);
	if((sensor.front_sensor & 0x80) !=0)
		condition = 2;
}

void BackToLine(){
	if(sensor.middle_sensor != 0)
	{
		wheels.backward(120,120);
	}
	wheels.backward(0,0);
	condition = 5;
}
void menu() {

	//Add your code here
	//You may add more functions or volatile variables
	//But DO NOT modify anything else

	//Press S1 to display the first group member's student id
	//Press S1 again to display the second group member's student id
	//Debouncing need to be done for S1 to work properly
	//16 char for each row LCD
	char row1[]= "F12  M67  B12   ";
	char row2[]= "                ";


	char MidS= sensor.middle_sensor;
	char MidL= MidS & 0xF0;
	MidL= MidL>>4;
	char MidR= MidS & 0x0F;

	char FrontS= sensor.front_sensor;
	char FrontL= FrontS & 0xF0;
	FrontL= FrontL>>4;
	char FrontR= FrontS & 0x0F;

	char BackS= sensor.back_sensor;
	char BackL= BackS & 0xF0;
	BackL= BackL>>4;
	char BackR= BackS & 0x0F;


	//////////////////////////////////convert to ascii number
	if ( MidL <10 )
	{
		MidL += 0x30;
	}
	else
	{
		MidL +=55;
	}

	if (MidR <10)
	{
		MidR += 0x30;
	}
	else
	{
		MidR +=55;
	}

	row1[6]= MidL;
	row1[7]= MidR;

	////////////////////////// c TO d, 3 TO b/f
	if ( FrontL <10 )
	{
		FrontL += 0x30;
	}
	else
	{
		FrontL +=55;
	}

	if (FrontR <10)
	{
		FrontR += 0x30;
	}
	else
	{
		FrontR +=55;
	}

	row1[1]= FrontL;
	row1[2]= FrontR;
	//////////////////////////
	if ( BackL <10 )
	{
		BackL += 0x30;
	}
	else
	{
		BackL +=55;
	}

	if (BackR <10)
	{
		BackR += 0x30;
	}
	else
	{
		BackR +=55;
	}

	row1[11]= BackL;
	row1[12]= BackR;



	lcd.setLineOne(row1);
	row2[0] = condition + 48;
	row2[2] = nextCondition + 48;

	char check1L = check1 & 0xF0;
	char check1R = check1 & 0x0F;
	check1L = check1L >> 4;
	if(check1L < 10) check1L += 0x30;
	else check1L += 55;
	if(check1R < 10) check1R += 0x30;
	else check1R += 55;

	char check2L = check2 & 0xF0;
	char check2R = check2 & 0x0F;
	check2L = check2L >> 4;
	if(check2L < 10) check2L += 0x30;
	else check2L += 55;
	if(check2R < 10) check2R += 0x30;
	else check2R += 55;

	char check3L = check3 & 0xF0;
	char check3R = check3 & 0x0F;
	check3L = check3L >> 4;
	if(check3L < 10) check3L += 0x30;
	else check3L += 55;
	if(check3R < 10) check3R += 0x30;
	else check3R += 55;

	row2[4] = check1L;
	row2[5] = check1R;

	row2[7] = check2L;
	row2[8] = check2R;

	row2[10] = check3L;
	row2[11] = check3R;

	lcd.setLineTwo(row2);



}

//Timer 3 overflow interrupt function
ISR(TIMER3_OVF_vect) {
	//Trigger the task scheduler to do the next task
	taskDone = FALSE;
}

//Timer 3 Output Compare B Match interrupt function
ISR(TIMER3_COMPB_vect) {
	//Trigger variable to update servo's channel
	updateChannel = TRUE;
}

void lineTracking() {

	unsigned char SMALL_DIFFERENCE  = 5; //define the small correction force
	unsigned char MEDIUM_DIFFERENCE = 15; //define the medium correction force
	unsigned char LARGE_DIFFERENCE  = 20; //define the large correction force
	unsigned char left_speed, right_speed, tracking_speed;

	tracking_speed = 150;
	left_speed = tracking_speed;
	right_speed = tracking_speed;

	switch (sensor.middle_sensor) {

		case 0b00000000: // cannot find a line
		left_speed = 0;
		right_speed = 0;
		break;

		case 0b00111100:
		case 0b00011000:
		case 0b00001000:
		case 0b00010000: //line at center
		left_speed = tracking_speed;
		right_speed = tracking_speed;
		break;

		case 0b00111000:
		case 0b00110000: //line slightly shift to right
		left_speed = tracking_speed - SMALL_DIFFERENCE;
		right_speed = tracking_speed + SMALL_DIFFERENCE;
		break;

		case 0b00011100:
		case 0b00001100: //line slightly s shift to left
		left_speed = tracking_speed + SMALL_DIFFERENCE;
		right_speed = tracking_speed - SMALL_DIFFERENCE;
		break;

		case 0b01110000:
		case 0b01100000: //line shift to right
		left_speed = tracking_speed - MEDIUM_DIFFERENCE;
		right_speed = tracking_speed + MEDIUM_DIFFERENCE;
		break;

		case 0b00001110:
		case 0b00000110: //line shift to left
		left_speed = tracking_speed + MEDIUM_DIFFERENCE;
		right_speed = tracking_speed - MEDIUM_DIFFERENCE;
		break;

		case 0b11100000:
		case 0b11000000:
		case 0b10000000: //line shift to right seriously
		left_speed = tracking_speed - LARGE_DIFFERENCE;
		right_speed = tracking_speed + LARGE_DIFFERENCE;
		break;

		case 0b00000111:
		case 0b00000011:
		case 0b00000001: //line shift to left seriously
		left_speed = tracking_speed + LARGE_DIFFERENCE;
		right_speed = tracking_speed - LARGE_DIFFERENCE;
		break;

		case 0b01111110:
		case 0b11111110:
		case 0b01111111:
		case 0b11111111: // Junction crossing

		break;

		default:
		break;
	}

	//set motor speed based on middle sensor's value
	wheels.forward(left_speed, right_speed);

}
