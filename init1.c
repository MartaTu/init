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
//***** Module Specific Defines and Enums *********************************
//***** Module Specific Macros and Typedefs *******************************
//***** Definition of Global Variables ************************************
//***** Definition of Static Variables ************************************
//***** Prototypes of Static Functions ************************************

static void gpio_init(char pin[]);
static void gpio_output(char pin[]);
static void gpio_setvalue(char pin[], char *value);
static void am33xx_pwm();
static void pwm_init(char pin[]);


int main(void) {

	gpio_init("62");
	gpio_output("62");
	gpio_setvalue("62","1");

	gpio_init("63");
	gpio_output("63");
	gpio_setvalue("63","1");

	pwm_init("bone_pwm_P9_42");
	pwm_init("bone_pwm_P9_22");
	am33xx_pwm();


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
static void gpio_output
(
char pin[]			// number of GPIO pin for output set
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
static void gpio_setvalue
(
char pin[], 		// number of GPIO pin for value set
char *value			// value
)
// * FUNCTION		: Setting value (1 or 0) for GPIO output configuration
// *				: (pin[] - e.g. GPIO1_31 <--> (1x32 + 31 = 63)
// * PRECONDITION	: GPIO pin must be initialized first and set output configuration
// * POSTCONDITION	: ###
// * ERROR EXITS	: ###
// * SIDE EFFECTS	: ###
// ************************************************************************
{
	if ((strncmp(value,"1",1)==0)||(strncmp(value,"0",1)==0))
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
