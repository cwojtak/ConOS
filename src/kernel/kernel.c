#include "kernel.h"

static int shell_context = 0;
//0 = standard
//1 = LOG_BLOCK
//2 = ALLOC_MEM
//3 = FREE_MEM
//4 = PRINT_MEM

void kernel_main(struct multiboot_info* bootInfo, int legacyMemorySize) {
	clear_screen();
	kprint_at("32-bit switch successful!\n", 0, 0);
	kprint("Kernel starting...\n");
	isr_install();
	irq_install();
	log(1, "CPU and kernel interrupts loaded!");

	char legMemMessage[] = "Legacy memory size identified: \0          ";
	hex_to_ascii(legacyMemorySize, legMemMessage);
	log(1, legMemMessage);

    log(1, "Preparing memory manager...");
	prepare_memory_manager((struct MemoryMapEntry*)bootInfo->mmap_addr, bootInfo->mmap_length);

    log(1, "Enumerating PCI devices...");
    Array* pci_devices = prepare_kernel_pci();

	log(1, "Identified PCI devices: ");
	for(int i = 0; i < pci_devices->used; i++)
	{
		uintptr_t addr = pci_devices->array + (uintptr_t)i * sizeof(uintptr_t);
		uintptr_t struc = *(uintptr_t*)(addr);
		log_pci_device((struct PCI_DEVICE*)(struc));
	}

    log(1, "Preparing kernel filesystem...");
    prepare_kernel_fs(pci_devices);

	kprint(" .d8888b.                     .d88888b.   .d8888b.  \n");
	kprint("d88P  Y88b                   d88P   Y88b d88P  Y88b \n");
	kprint("888    888                   888     888 Y88b.      \n");
	kprint("888         .d88b.  88888b.  888     888   Y888b.   \n");
	kprint("888        d88  88b 888  88b 888     888      Y88b. \n");
	kprint("888    888 888  888 888  888 888     888        888 \n");
	kprint("Y88b  d88P Y88..88P 888  888 Y88b. .d88P Y88b  d88P \n");
	kprint("  Y8888P     Y88P   888  888   Y88888P     Y8888P   \n");


	kprint("You are currently in the protected mode shell, type END to stop the machine.\n> ");
}

void user_input(char *input)
{
	if(shell_context == 1)
	{
		mm_logblock(atoi(input));
		shell_context = 0;
		kprint("> ");
	}
	else if(shell_context == 2)
	{
		uintptr_t allocation = mm_allocate(atoi(input));

		if(allocation != (uintptr_t)NULL)
		{
			char output[100] = "";
			char tempStr[32] = "";
			hex_to_ascii(allocation, tempStr);
			strcat("Memory successfully allocated at physical address ", tempStr, output);
			appendstr(output, ".");
			log(1, output);
		}
		else
		{
			log(3, "Memory allocation failed.");
		}

		shell_context = 0;
		kprint("> ");
	}
	else if(shell_context == 3)
	{
		int i = atoi(input);
		int error = mm_free(i);
		if(error == -1)
		{
			log(3, "Memory free failed.");
		}
		else
		{
			char output[100] = "";
			char tempStr[32] = "";
			hex_to_ascii(i, tempStr);
			strcat("Memory successfully freed at physical address ", tempStr, output);
			appendstr(output, ".");
			log(1, output);
		}
		shell_context = 0;
		kprint("> ");
	}
	else if(shell_context == 4)
	{
		int i = atoi(input);
		if(i != 0)
		{
			uint32_t* addr = (uint32_t*)i;
			uint32_t value = *addr;
			char output[64] = "";
			hex_to_ascii(value, output);
			kprint(output);
		}
		shell_context = 0;
		kprint("\n> ");
	}
	else
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
			kprint("COM_TEST - Test COM1\n");
			kprint("LOG_BLOCK - Log a memory block\n");
			kprint("ALLOC_MEM - Allocate some memory\n");
			kprint("FREE_MEM - Free some memory\n");
			kprint("PRINT_MEM - Print memory at a certain address");
            kprint("LS - List all the files in the current working directory\n");
			kprint("> ");
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
			kprint("\n> ");
		}
		else if (strcmp(input, "COM_TEST") == 0)
		{
			const char testString[] = "COM_TEST";
			for(int i = 0; i < 8; i++)
			{
				write_serial(testString[i]);
			}
			kprint("\n> ");
		}
		else if (strcmp(input, "LOG_BLOCK") == 0)
		{
			shell_context = 1;
			kprint("Enter a block number: ");
		}
		else if (strcmp(input, "ALLOC_MEM") == 0)
		{
			shell_context = 2;
			kprint("Enter the size in bytes (base 10) to allocate (enter 0 to cancel): ");
		}
		else if (strcmp(input, "FREE_MEM") == 0)
		{
			shell_context = 3;
			kprint("Enter the address (base 10) to free (enter 0 to cancel): ");
		}
		else if (strcmp(input, "PRINT_MEM") == 0)
		{
			shell_context = 4;
			kprint("Enter the address (base 10) to print (enter 0 to cancel): ");
		}
        else if (strcmp(input, "LS") == 0)
        {
            struct FILE currentDirectory;
            currentDirectory.path[0] = '/';
            currentDirectory.path[1] = '\0';
            currentDirectory.isDirectory = 1; //(true)
            currentDirectory.firstCluster = 0;

            struct FILE_ENUMERATION files;
            fat12_enumerate_files(&currentDirectory, &files);
            
			kprint("Contents of ");
			kprint(currentDirectory.path);
			kprint(":\n");
			kprint("NAME, SIZE\n");
            for(uint32_t i = 0; i < files.numFiles; i++)
            {
                kprint(files.files[i].path);
				kprint(" ");
				char fileSz[32] = "";
				int_to_ascii(files.files[i].fileSize, fileSz);
				kprint(fileSz);
                kprint(" B\n");
            }
			char numEntries[32] = "";
			int_to_ascii(files.numFiles, numEntries);
			kprint(numEntries);
			kprint(" entries total");

            mm_free((uintptr_t)files.files);
            kprint("\n> ");
        }
		else
		{
			kprint("Invalid Command: ");
			kprint(input);
			kprint("\nTry HELP for a list of commands.");
			kprint("\n> ");
		}
	}
}
