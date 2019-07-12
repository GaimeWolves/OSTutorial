#ifndef PIC_H
#define PIC_H

#include <stdint.h>

//PIC devices
#define PIC_IRQ_TIMER		0
#define PIC_IRQ_KEYBOARD	1
#define PIC_IRQ_SERIAL2		3
#define PIC_IRQ_SERIAL1		4
#define PIC_IRQ_PARALLEL2	5
#define PIC_IRQ_DISKETTE	6
#define PIC_IRQ_PARALLEL1	7

#define PIC_IRQ_CMOSTIMER	0
#define PIC_IRQ_CGARETRACE	1
#define PIC_IRQ_AUXILIARY	4
#define PIC_IRQ_FPU			5
#define PIC_IRQ_HDC			6

//Command Word 2
#define	PIC_OCW2_MASK_L1		0x01
#define	PIC_OCW2_MASK_L2		0x02
#define	PIC_OCW2_MASK_L3		0x04
#define	PIC_OCW2_MASK_EOI		0x20
#define	PIC_OCW2_MASK_SL		0x40
#define	PIC_OCW2_MASK_ROTATE	0x80

//Command Word 3
#define	PIC_OCW3_MASK_RIS		0x01
#define	PIC_OCW3_MASK_RIR		0x02
#define	PIC_OCW3_MASK_MODE		0x04
#define	PIC_OCW3_MASK_SMM		0x20
#define	PIC_OCW3_MASK_ESMM		0x40
#define	PIC_OCW3_MASK_D7		0x80

extern uint8_t pic_read_data(uint8_t pic_num);
extern void pic_send_data(uint8_t data, uint8_t pic_num);
extern void pic_send_command(uint8_t cmd, uint8_t pic_num);
extern void pic_init(uint8_t base0, uint8_t base1);

#endif