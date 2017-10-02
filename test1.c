//MH****************** Copyright (c) by GC *****************************
// Filename 		: init1.c
// Owner			: Gigaset Communications Polska Sp. z o.o.
//-------------------------------------------------------------------------
// Function			: Initialization of BeagleBone Black's ports -gpio(62,63), pwm(42,22)
// Special cases	: Only for Debian Embedded version BeagleBone Black Borad
//-------------------------------------------------------------------------
//*************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

//***** Module Specific Defines and Enums *********************************
//***** Module Specific Macros and Typedefs *******************************
//***** Definition of Global Variables ************************************

#define LED3_PATH 	"/sys/class/leds/beaglebone:green:usr3"
#define L_PWM 		"pwm_test_P9_42.15"
#define P_PWM 		"pwm_test_P9_22.16"
#define L_GPIO 		"/sys/class/gpio/gpio49"

//***** Definition of Static Variables ************************************
//***** Prototypes of Static Functions ************************************

static void filecopy(FILE *, FILE *);
static void writeLED(char filename[], char value[]);
static void removeTrigger();

static void gpio_init(char pin[]);
static void gpio_uninit(char pin[]);
static void gpio_output(char pin[]);
static void gpio_input(char pin[]);
static void gpio_setvalue(char pin[], char *value);

static void am33xx_pwm();
static void pwm_init(char pin[]);
static void pwm_uninit(char number[]);
static void pwm_setup(char pin[], char set[], char *value);

static void turnleft();
static void turnright();
static void go();
static void stop();
static void turn_around();

static void delay(unsigned int mseconds);
static void circle();

int main(void) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	int n = 0;
	unsigned char buf = '\0';
	int USB = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY/* | O_NDELAY*/);

	char response[1024];
	memset(response, '\0', sizeof response);

	if(USB == -1)
	{
		perror("Unable to open the port");
	}

	//***** Error handler *****************************************************
	if (tcgetattr (USB,&tty)!=0)
	{
		printf("Error %d from tcgetattr: %s \n",errno,strerror(errno));
	}
	//***** Save old tty parameters *******************************************

	cfsetospeed(&tty,(speed_t)B38400);
	cfsetispeed(&tty,(speed_t)B38400);
	//***** Setting other port stuff ******************************************

	tty.c_cflag 	&= 	~PARENB;
	tty.c_cflag 	&= 	~CSTOPB;
	tty.c_cflag 	&=	~CSIZE;
	tty.c_cflag		|=	CS8;

	tty.c_cflag 	&=	~CRTSCTS;		//	no flow control
	tty.c_cc[VMIN]	= 	1;				//	read doesn't block
	tty.c_cc[VTIME] = 	5;				//	0.5 seconds read timeout
	tty.c_cflag		|=	CREAD | CLOCAL;	//	turn on READ & ignore ctrl lines


	//***** Make raw **********************************************************
	cfmakeraw(&tty);

	//***** Flush Port, then applies attributes *******************************
	tcflush(USB, TCIFLUSH);
	if(tcsetattr (USB, TCSANOW, &tty) != 0)
	{
		printf("Error %d from tcsetattr \n", errno);
	}


	pwm_setup("pwm_test_P9_42.15","duty","5000");
	pwm_setup("pwm_test_P9_42.15","period","10000");
	pwm_setup("pwm_test_P9_42.15","run","0");

	pwm_setup("pwm_test_P9_22.16","duty","5000");
	pwm_setup("pwm_test_P9_22.16","period","10000");
	pwm_setup("pwm_test_P9_22.16","run","0");

	//***** Test of pwm settings - make circle
	circle();
	//***** Reading from serial port loop ************************************

	while(1)
	{
		n = read(USB, &buf, 1);
		sprintf(response, "%c", buf);

		if (n == -1)
		{
			printf("Error reading from serial port: %s\n", strerror(errno));
			break;
		}
		else if (n == 0)
		{
			printf("No data\n");
		}
		else
		{
			buf = '\0';
			printf("%s",response);
			if(strncmp(response,"C",1)==0)
			{
				printf("\n");
				turnleft();
			}
			else if(strncmp(response,"A",1)==0)
			{
				printf("\n");
				go();
			}
			else if(strncmp(response,"D",1)==0)
			{
				printf("\n");
				turnright();
			}
			else if(strncmp(response,"B",1)==0)
			{
				printf("\n");
				stop();
			}
			else if((strncmp(response,"\0",1)==0)||(strncmp(response,"\n",1)==0))
			{
				printf("last char of response");
			}
			else
			{
				printf("Error reading response - turns\n");
				printf("%s", response);
			}
		}

	}

}
//PH****************** Copyright (c) by GC *****************************
static void filecopy(FILE *ifp, FILE *ofp)
{
	int c;
	while ((c =getc(ifp)) != EOF)
	{
		putc(c, ofp);
	}
}
//PH****************** Copyright (c) by GC *****************************
static void writeLED(char filename[], char value[])
{
	FILE* fp;
	char fullFileName[100];

	sprintf(fullFileName, LED3_PATH "%s", filename);
	fp = fopen(fullFileName, "w");
	fprintf(fp, "%s", value);
	fclose(fp);

}
//PH****************** Copyright (c) by GC *****************************
static void removeTrigger()
{
	writeLED("/trigger", "none");
}
//PH****************** Copyright (c) by GC *****************************
static void gpio_init
(
char pin[]			//number of initialized pin
)
// * FUNCTION		: Initialization of GPIO
// *				: (e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{

	FILE *GPIOHandle = NULL;
	char *GPIOPath = "/sys/class/gpio/export";
	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "%s", pin);
	fclose(GPIOHandle);

	printf("GPIO Init - GPIO_%s \n", pin);
	printf("GPIO Init end\n");

}
//PH****************** Copyright (c) by GC *****************************
static void gpio_uninit
(
char pin[]			//number of uninitialized pin
)
// * FUNCTION		: Uninitialization of GPIO
// *				: (e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *GPIOHandle = NULL;
	char *GPIOPath = "/sys/class/gpio/unexport";
	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "%s", pin);
	fclose(GPIOHandle);

	printf("GPIO Uninit - GPIO_%s - unexported\n", pin);
	printf("GPIO Uninit end \n");
}
//PH****************** Copyright (c) by GC *****************************
static void gpio_output
(
char pin[]				// number of GPIO pin for output set
)
// * FUNCTION		: Setting output configuration for GPIO
// *				: ( pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *GPIOHandle = NULL;
	char GPIOPath[50] = "/sys/class/gpio";
	sprintf(GPIOPath, "%s/gpio%s/direction",GPIOPath, pin);

	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "out");
	fclose(GPIOHandle);

	printf("GPIO Out - GPIO_%s \n", pin);

}
//PH****************** Copyright (c) by GC *****************************
static void gpio_input
(
char pin[]				// number of GPIO pin for input set
)
// * FUNCTION		: Setting input configuration for GPIO
// *				: ( pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *GPIOHandle = NULL;
	char GPIOPath[50] = "/sys/class/gpio";
	sprintf(GPIOPath, "%s/gpio%s/direction",GPIOPath, pin);

	GPIOHandle = fopen(GPIOPath, "w");
	fprintf(GPIOHandle, "in");
	fclose(GPIOHandle);

	printf("GPIO In - GPIO_%s \n", pin);

}
//PH****************** Copyright (c) by GC *****************************
static void gpio_setvalue
(
char pin[], 			// number of GPIO pin for value set
char *value				// value
)
// * FUNCTION		: Setting value (1 or 0) for GPIO output configuration
// *				: (pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first and set output configuration
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	if ((strncmp(value,"1",1)==0)||(strncmp(value,"0",1))==0)
	{
		FILE *GPIOHandle = NULL;
		char GPIOPath[50] = "/sys/class/gpio";
		sprintf(GPIOPath, "%s/gpio%s/value",GPIOPath, pin);

		GPIOHandle = fopen(GPIOPath, "w");
		fprintf(GPIOHandle, value);
		fclose(GPIOHandle);

		printf("GPIO_%s value=%s \n",pin,value);
	}
	else
		printf("Wrong value\n");
}
//PH****************** Copyright (c) by GC *****************************
static void am33xx_pwm()
// * FUNCTION		: Initialization of am33xx_pwm
// *				:
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	//pin - f.e. bone_pwm_P9_22
	//pin bone_pwm_P9_42
	FILE *PWMHandle = NULL;
	char *SLOTS = "/sys/devices/bone_capemgr.9/slots";

	PWMHandle = fopen(SLOTS, "w");
	fprintf(PWMHandle, "am33xx_pwm");
	fclose(PWMHandle);

	printf("am33xx_pwm Init end\n");
}
//PH****************** Copyright (c) by GC *****************************
static void pwm_init
(
char pin[]			// pwm path f.e. bone_pwm_P9_22
)
// * FUNCTION		: Initialization of pwm
// *				:
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *PWMHandle = NULL;
	char *SLOTS = "/sys/devices/bone_capemgr.9/slots";

	PWMHandle = fopen(SLOTS, "w");
	fprintf(PWMHandle, "%s", pin);
	fclose(PWMHandle);

	printf("PWM Init - %s \n", pin);
	printf("PWM Init end\n");
}
//PH****************** Copyright (c) by GC *****************************
static void pwm_uninit
(
char number[]		// cat $SLOTS - check the number in system
)
// * FUNCTION		: Uninitialization of pwm
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	FILE *PWMHandle = NULL;
	char *SLOTS = "/sys/devices/bone_capemgr.9/slots";

	PWMHandle = fopen(SLOTS, "w");
	fprintf(PWMHandle, "-%s", number);
	fclose(PWMHandle);

	printf("PWM Uninit - %s \n", number);
	printf("PWM Uninit end\n");
}
//PH****************** Copyright (c) by GC *****************************
static void pwm_setup
(
char pin[],			// pwm path f.e. bone_pwm_P9_22
char set[], 		// f.e duty, period, run
char *value			// value of setting
)
// * FUNCTION		: Setting value of pwm parameters
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{

	FILE *PWMHandle = NULL;
	char GPIOPath[80] = "/sys/devices/ocp.3";
	sprintf(GPIOPath, "%s/%s/%s",GPIOPath, pin, set);

	PWMHandle = fopen(GPIOPath, "w");
	fprintf(PWMHandle, value);
	fclose(PWMHandle);

	printf("GPIO_%s %s=%s \n",pin,set,value);
}
//PH****************** Copyright (c) by GC *****************************
static void turnleft()
// * FUNCTION		: Turning left with the motors
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	//lewy 0, prawy 1
	pwm_setup(P_PWM,"run","1");
	pwm_setup(L_PWM,"run", "0");
}
//PH****************** Copyright (c) by GC *****************************
static void turnright()
// * FUNCTION		: Turning right with the motors
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	//lewy 1, prawy 0
	pwm_setup(P_PWM,"run","0");
	pwm_setup(L_PWM,"run", "1");
}
//PH****************** Copyright (c) by GC *****************************
static void go()
// * FUNCTION		: Both motors on
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	pwm_setup(P_PWM,"run","1");
	pwm_setup(L_PWM,"run", "1");
}
//PH****************** Copyright (c) by GC *****************************
static void stop()
// * FUNCTION		: Both motors off
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	pwm_setup(P_PWM,"run","0");
	pwm_setup(L_PWM,"run", "0");
}
//PH****************** Copyright (c) by GC *****************************
static void turn_around()
// * FUNCTION		: Turning around
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{

}
//PH****************** Copyright (c) by GC *****************************
static void delay(unsigned int mseconds)
// * FUNCTION		: System delay
// *				:
// * PRECONDITION	: ###
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
//PH****************** Copyright (c) by GC *****************************
static void circle()
// * FUNCTION		: Circle for test motors
// *				:
// * PRECONDITION	: PWM must be initialized before
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	go();
	sleep(1);
	turnleft();
	sleep(1);
	go();
	sleep(1);
	turnleft();
	sleep(1);
	go();
	sleep(1);
	turnleft();
	sleep(1);
	go();
	sleep(1);
	turnleft();
	sleep(1);
	stop();
}
