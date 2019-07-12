#include "pic.h"

#include <hal.h>

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

#define PIC1_REG_COMMAND	0x20
#define PIC1_REG_STATUS		0x20
#define PIC1_REG_DATA		0x21
#define PIC1_REG_IMR		0x21

#define PIC2_REG_COMMAND	0xA0
#define PIC2_REG_STATUS		0xA0
#define PIC2_REG_DATA		0xA1
#define PIC2_REG_IMR		0xA1


#define PIC_ICW1_MASK_IC4 	0x01
#define PIC_ICW1_MASK_SNGL 	0x02
#define PIC_ICW1_MASK_ADI 	0x04
#define PIC_ICW1_MASK_LTIM 	0x08
#define PIC_ICW1_MASK_INIT 	0x10
		
#define PIC_ICW4_MASK_UPM 	0x01
#define PIC_ICW4_MASK_AEOI 	0x02
#define PIC_ICW4_MASK_MS 	0x04
#define PIC_ICW4_MASK_BUF 	0x08
#define PIC_ICW4_MASK_SFNM 	0x10


#define PIC_ICW1_IC4_EXPECT 			0x01
#define PIC_ICW1_SNGL	 				0x02
#define PIC_ICW1_ADI_CALLINTERVAL4 		0x04
#define PIC_ICW1_ADI_CALLINTERVAL8 		0x00
#define PIC_ICW1_LTIM_LEVELTRIGGERED 	0x08
#define PIC_ICW1_LTIM_EDGETRIGGERED 	0x00
#define PIC_ICW1_INIT	 				0x10

#define PIC_ICW4_UPM_86MODE			0x01
#define PIC_ICW4_UPM_MCSMODE		0x00
#define PIC_ICW4_AEOI_AUTOEOI		0x02
#define PIC_ICW4_MS_BUFFERMASTER	0x0C
#define PIC_ICW4_MS_BUFFERSLAVE		0x08
#define PIC_ICW4_BUF_MODEYES		0x08
#define PIC_ICW4_SFNM_NESTEDMODE	0x10

uint8_t pic_read_data(uint8_t pic_num)
{
	if (pic_num > 1) return 0;

	uint16_t reg = pic_num ? PIC2_REG_DATA : PIC1_REG_DATA;
	return inportb(reg);
}

void pic_send_data(uint8_t data, uint8_t pic_num)
{
	if (pic_num > 1) return;

	uint16_t reg = pic_num ? PIC2_REG_DATA : PIC1_REG_DATA;
	outportb(reg, data);
}

void pic_send_command(uint8_t cmd, uint8_t pic_num)
{
	if (pic_num > 1) return;

	uint16_t reg = pic_num ? PIC2_REG_COMMAND : PIC1_REG_COMMAND;
	outportb(reg, cmd);
}

void pic_init(uint8_t base0, uint8_t base1)
{
	uint8_t icw = 0;

	disable_int();

	//ICW 1
	icw = (icw & ~PIC_ICW1_MASK_INIT) | PIC_ICW1_INIT;
	icw = (icw & ~PIC_ICW1_MASK_IC4) | PIC_ICW1_IC4_EXPECT;

	pic_send_command(icw, 0);
	pic_send_command(icw, 1);

	//ICW 2
	pic_send_data(base0, 0);
	pic_send_data(base1, 1);

	//ICW 3
	pic_send_data(0x04, 0);
	pic_send_data(0x02, 1);

	//ICW 4
	icw = 0;
	icw = (icw & ~PIC_ICW4_MASK_UPM) | PIC_ICW4_UPM_86MODE;

	pic_send_data(icw, 0);
	pic_send_data(icw, 1);

	pic_send_data(0, 0);
	pic_send_data(0, 1);

#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: PIC initialized.\n");
#endif
}