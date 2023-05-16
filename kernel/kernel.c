#include "kernel.h"

void setEntryPoint(){
/* Keeps the kernel_entry.asm from jumping to kernel.c:0x00 */
}

void kernel_main() {
	clear_screen();
	kprint_at("32-bit switch successful!\n", 0, 0);
	kprint("Kernel starting...\n");
	isr_install();
	irq_install();
	kprint("CPU and kernel interrupts loaded!\n");

	kprint(" .d8888b.                     .d88888b.   .d8888b.  \n");
	kprint("d88P  Y88b                   d88P   Y88b d88P  Y88b \n");
	kprint("888    888                   888     888 Y88b.      \n");
	kprint("888         .d88b.  88888b.  888     888   Y888b.   \n");
	kprint("888        d88  88b 888  88b 888     888      Y88b. \n");
	kprint("888    888 888  888 888  888 888     888        888 \n");
	kprint("Y88b  d88P Y88..88P 888  888 Y88b. .d88P Y88b  d88P \n");
	kprint("  Y8888P     Y88P   888  888   Y88888P     Y8888P   \n");


	kprint("You are currently in the protected mode shell, type END to stop the machine.\n>");
}

void user_input(char *input)
{
    if (strcmp(input, "END") == 0)
	{
        kprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }
    else if (strcmp(input, "HELP") == 0)
	{
		kprint("Available commands:\n");
		kprint("HELP - This help menu\n");
		kprint("END - Stop the CPU\n");
		kprint("PAGE - Allocate 1000 bytes in memory\n");
		kprint("\n>");
	}
    else if (strcmp(input, "PAGE") == 0)
	{
		uint32_t phys_addr;
		uint32_t page = kmalloc(1000, 1, &phys_addr);
		char page_str[16] = "";
		hex_to_ascii(page, page_str);
		char phys_str[16] = "";
		hex_to_ascii(phys_addr, phys_str);
		kprint("Page: ");
		kprint(page_str);
		kprint(", physical address: ");
		kprint(phys_str);
		kprint("\n>");
    }
    else
	{
		kprint("Invalid Command: ");
		kprint(input);
		kprint("\nTry HELP for a list of commands.");
		kprint("\n> ");
    }
}
