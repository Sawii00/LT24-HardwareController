/*
 * This example shows how to initialize and display an image from memory to the LT24 LCD
 */

#include <inttypes.h>
#include "system.h"
#include "io.h"
#include "stdio.h"
#include "stdarg.h"


// Register Map Offsets
#define IMAGE_ADDRESS 0
#define IMAGE_LENGTH 4
#define FLAGS 8
#define COMMAND_REG 10
#define N_PARAMS 12
#define PARAM(n) 14 + n * 2

/*
	Sets a specified flag within the Flags() register.
*/
void set_flag(uint16_t val)
{
	uint16_t flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);
	IOWR_16DIRECT(LCD_0_BASE, FLAGS, flags | val);
}

/*
	Sets the reset flag to initialize the reset procedure and waits for its completion
*/
void reset_lcd()
{
	set_flag(0x4);
	uint16_t flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);

	while(flags & 0x4)
	{
		flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);
	}
}

/*
	Sends the command, number of parameters and parameters to the Register File.
	Sets the send_command flag to instruct the controller to send the command and waits for its completion.
*/
void send_command(uint16_t cmd, uint16_t n, uint16_t* params)
{
	uint16_t i = 0;

	IOWR_16DIRECT(LCD_0_BASE, COMMAND_REG, cmd);
	IOWR_16DIRECT(LCD_0_BASE, N_PARAMS, n);


	while(i < n)
	{
		IOWR_16DIRECT(LCD_0_BASE, PARAM(i), params[i]);
		++i;
	}

	IOWR_16DIRECT(LCD_0_BASE, FLAGS, 0x2); //send command flag
	uint16_t flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);
	while(flags & 0x2)
	{
		flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);
	}
}

/*
	Sets the lcd_enable flag to start displaying the image and waits for the completion.
*/
void start_lcd()
{
	set_flag(0x1);
	uint16_t flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);

	while(flags & 0x1)
	{
		flags = IORD_16DIRECT(LCD_0_BASE, FLAGS);
	}
}

/*
	Sends the initialization commands to setup the display correctly.
*/
void init_lcd()
{
	send_command(0x11, 0, (uint16_t []){ 0x09, 0x0a});
	send_command(0xcf, 3, (uint16_t []){ 0x0, 0x81, 0xc0});
	send_command(0xed, 4, (uint16_t []){ 0x64, 0x03, 0x12, 0x81});
	send_command(0xe8, 3, (uint16_t []){ 0x85, 0x01, 0x0798});
	send_command(0xcb, 5, (uint16_t []){ 0x39, 0x2c, 0x00, 0x34, 0x02});
	send_command(0xf7, 1, (uint16_t []){ 0x20});
	send_command(0xea, 2, (uint16_t []){ 0x00, 0x00});
	send_command(0xb1, 2, (uint16_t []){ 0x00, 0x1b});
	send_command(0xb6, 2, (uint16_t []){ 0x0a, 0xa2});
	send_command(0xc0, 1, (uint16_t []){ 0x05});
	send_command(0xc1, 1, (uint16_t []){ 0x11});
	send_command(0xc5, 2, (uint16_t []){ 0x45, 0x45});
	send_command(0xc7, 1, (uint16_t []){ 0xa2});
	send_command(0x36, 1, (uint16_t []){ 0x48}); //RGB
	send_command(0xf2, 1, (uint16_t []){ 0x00});
	send_command(0x26, 1, (uint16_t []){ 0x01});
	send_command(0xe0, 15, (uint16_t []){ 0xf, 0x26, 0x24, 0xb, 0xe, 0x8, 0x4b, 0xa8, 0x3b, 0x0a, 0x14, 0x06, 0x10, 0x09, 0x00});
	send_command(0xe1, 15, (uint16_t []){ 0x0, 0x1c, 0x20, 0x4, 0x10, 0x8, 0x34, 0x47, 0x44, 0x05, 0xb, 0x9, 0x2f, 0x36, 0x0f});
	send_command(0x2a, 4, (uint16_t []){ 0x0, 0x0, 0x0, 0xef});
	send_command(0x2b, 4, (uint16_t []){ 0x0, 0x0, 0x01, 0x3f});
	send_command(0x3a, 1, (uint16_t []){ 0x55}); //RGB
	send_command(0xf6, 3, (uint16_t []){ 0x01, 0x30, 0x0});
	send_command(0x29, 0, (uint16_t []){ 0x09, 0x0a});
}

/*
	Sets the internal ImageAddress and ImageLength registers with the specified values.
*/
void configure_image(uint32_t image_address, uint32_t image_size)
{
	IOWR_32DIRECT(LCD_0_BASE, IMAGE_ADDRESS, image_address);
	IOWR_32DIRECT(LCD_0_BASE, IMAGE_LENGTH, image_size);

	printf("Sent address: %d\nSent length: %d\n", IORD_32DIRECT(LCD_0_BASE, IMAGE_ADDRESS), IORD_32DIRECT(LCD_0_BASE, IMAGE_LENGTH));

}

/*
	Populates memory with a default image to be displayed.
*/
void init_image(uint32_t image_address, uint32_t rows, uint32_t cols)
{
	uint32_t i = 0;
	uint32_t j = 0;

	while(i < rows * sizeof(uint16_t))
	{
		while(j < cols * sizeof(uint16_t))
		{
			IOWR_16DIRECT(image_address, i * rows + j, 0xF800);
			if(IORD_16DIRECT(image_address, i * rows + j) != 0xF800)
			{
				printf("Error writing to extender\n");
			}
			j += 2;
		}

		i += 2;
	}
}


int main()
{

	reset_lcd();

	printf("Initializing LCD\n");
	init_lcd();
	printf("LCD Initialized\n");

	uint32_t image_size = 320 * 240 * sizeof(uint16_t);//320 * 240 * sizeof(uint16_t);
	uint32_t image_address = HPS_0_BRIDGES_BASE;

	printf("Setting image address and size\n");
	init_image(image_address, 320, 240);
	printf("Image address and size set\n");

	printf("Configuring default image\n");
	configure_image(image_address, image_size);
	printf("Default image configured... starting LCD\n");
	start_lcd();
	printf("Display Stopped\n");

	return 0;
}
