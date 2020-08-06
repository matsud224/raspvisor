#include "utils.h"
#include "printf.h"
#include "timer.h"
#include "entry.h"
#include "peripherals/irq.h"

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL2",
	"IRQ_INVALID_EL2",
	"FIQ_INVALID_EL2",
	"ERROR_INVALID_EL2",

	"SYNC_INVALID_EL01_64",
	"IRQ_INVALID_EL01_64",
	"FIQ_INVALID_EL01_64",
	"ERROR_INVALID_EL01_64",

	"SYNC_INVALID_EL01_32",
	"IRQ_INVALID_EL01_32",
	"FIQ_INVALID_EL01_32",
	"ERROR_INVALID_EL01_32",

	"SYNC_ERROR",
	"HVC_ERROR",
	"DATA_ABORT_ERROR",
};

void enable_interrupt_controller()
{
	put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

void handle_irq(void)
{
	unsigned int irq = get32(IRQ_PENDING_1);
	switch (irq) {
		case (SYSTEM_TIMER_IRQ_1):
			handle_timer_irq();
			break;
		default:
			printf("Inknown pending irq: %x\r\n", irq);
	}
}
