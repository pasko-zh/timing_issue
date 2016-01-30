extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "gpio.h"
	#include "user_interface.h"
}

byte YELLOW_pin = 4;
byte BLUE_pin = 12;

int dummy;

uint32_t YELLOW_BITMASK;
uint32_t BLUE_BITMASK;

uint32_t volatile * PIN_OUT_SET = (uint32_t volatile *) 0x60000304;
uint32_t volatile * PIN_OUT_CLEAR = (uint32_t volatile *) 0x60000308;


void toggle_setup__C(byte sda_pin, byte scl_pin) {
	pinMode(sda_pin, OUTPUT_OPEN_DRAIN);
	pinMode(scl_pin, OUTPUT_OPEN_DRAIN);
	YELLOW_BITMASK = (uint32_t) (1 << YELLOW_pin);
	BLUE_BITMASK = (uint32_t) (1 << BLUE_pin);
}

void toggle_blue_yellow__asm()
// yellow pin = 4  => bit mask = 16d, [0...0]10000 (32 bit)
// blue pin   = 12 => bit mask = 4096d, [0...0]1 0000 0000 0000 (32 bit)
// PIN_OUT_SET   : 0x60000304
// PIN_OUT_CLEAR : 0x60000308
//
{
	asm volatile (
	"movi    a2, 0x60000304;" // SET Address in a2
	"movi    a3, 0x60000308;" // CLEAR Address in a3
	"movi    a4, 16;"         // yellow pin bit mask in a4
	"movi    a5, 4096;"       // blue pin bit mask in a5

	"movi    a6, 12;"         // delay loop counter
	"s32i    a4, a3, 0;"      // set yellow to LOW
	"memw;"
	"s32i    a5, a2, 0;"      // set blue to HIGH
	"memw;"
	"label1:"
	"addi    a6, a6, -1;"
	"NOP;"
	"NOP;"
	"NOP;"
	"NOP;"
	"bnez    a6, label1;"

	"movi    a6, 12;"         // delay loop counter
	"s32i    a4, a2, 0;"      // set yellow to HIGH
	"memw;"
	"s32i    a5, a3, 0;"      // set blue to LOW
	"memw;"
	"label2:"
	"addi    a6, a6, -1;"
	"NOP;"
	"NOP;"
	"NOP;"
	"NOP;"
	"bnez    a6, label2;"

	"movi    a6, 12;"         // delay loop counter
	"s32i    a4, a3, 0;"      // set yellow to LOW
	"memw;"
	"s32i    a5, a3, 0;"      // set blue to LOW
	"memw;"
	"label3:"
	"addi    a6, a6, -1;"
	"NOP;"
	"NOP;"
	"NOP;"
	"NOP;"
	"bnez    a6, label3;"

	"movi    a6, 12;"         // delay loop counter
	"s32i    a4, a2, 0;"      // set yellow to HIGH
	"memw;"
	"s32i    a5, a2, 0;"      // set blue to HIGH
	"memw;"
	"label4:"
	"addi    a6, a6, -1;"
	"NOP;"
	"NOP;"
	"NOP;"
	"NOP;"
	"bnez    a6, label4;"

	::
	: "a2", "a3", "a4", "a5", "a6", "memory"
	);
}

void toggle_blue_yellow__C() {
	int i;
	* PIN_OUT_CLEAR = YELLOW_BITMASK;  // set yellow to LOW
	* PIN_OUT_SET = BLUE_BITMASK;      // set blue to HIGH
	for (i = 12; i > 0; i--) {
		asm ("NOP; NOP; NOP; NOP":::);   // no input nor output operands and no clobbers
	}

	* PIN_OUT_SET = YELLOW_BITMASK;    // set yellow to HIGH
	* PIN_OUT_CLEAR = BLUE_BITMASK;    // set blue to LOW
	for (i = 12; i > 0; i--) {
		asm ("NOP; NOP; NOP; NOP":::);   // no input nor output operands and no clobbers
	}

	* PIN_OUT_CLEAR = YELLOW_BITMASK;  // set yellow to LOW
	* PIN_OUT_CLEAR = BLUE_BITMASK;    // set blue to LOW
	for (i = 12; i > 0; i--) {
		asm ("NOP; NOP; NOP; NOP":::);   // no input nor output operands and no clobbers
	}

	* PIN_OUT_SET = YELLOW_BITMASK;    // set yellow to HIGH
	* PIN_OUT_SET = BLUE_BITMASK;      // set blue to HIGH
	for (i = 12; i > 0; i--) {
		asm ("NOP; NOP; NOP; NOP":::);   // no input nor output operands and no clobbers
	}

}


void setup() {
	toggle_setup__C(YELLOW_pin,BLUE_pin);
	dummy = 0;
}

void loop() {
	delay(2000);
// 	ETS_INTR_LOCK();  // lock Interrupts
// 	system_soft_wdt_stop(); // stop watchdog

dummy = dummy + 1; // it will overflow after 16384 * 2 seconds	
// dummy = 38;
toggle_blue_yellow__C();
// toggle_blue_yellow__asm();	
// 	ETS_INTR_UNLOCK(); // enable Interrupts
// 	system_soft_wdt_restart();  // enable watchdog
// 	system_soft_wdt_feed();     // feed watchdog
}
