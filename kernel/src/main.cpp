#include <panic.hpp>
#include <cstring>
#include <lib/Flanterm/ftctx.h>
#include <drivers/serial/serial.hpp>
#include <drivers/serial/printf.h>
#include <drivers/serial/print.hpp>
#include <arch/arch.hpp>
#include <mem/mem.hpp>
#include <drivers/timers/pit.hpp>
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <uacpi/tables.h>
#include <tmpfs/tmpfs.hpp>
#include <pci/pci.hpp>
#include <drivers/blockio/ahci.hpp>

#define UACPI_ERROR(name, isinit) \
if (uacpi_unlikely_error(uacpi_result)) { \
    Log::errf("uACPI %s Failed: %s", \
              name, uacpi_status_to_string(uacpi_result)); \
    asm volatile ("cli; hlt;"); \
} \
else \
    Log::printf_status("OK", "uACPI %s%s", \
                       name, ((isinit) ? "d" : " Initialised"))

#include <limine.h>
__attribute__((section(".limine_requests")))
volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
    .response = nullptr, // shut up gcc
};

extern "C" void init() {
    if (module_request.response == nullptr || module_request.response->module_count < 1) {
        asm volatile ("cli;hlt");
    }

    flanterm_initialise();
       
    serial::serial_enable();
    Log::print_status("OK", "Flanterm Initialised"); // late
    Log::print_status("OK", "Serial Initialised");
    
    arch::x86_64::cpu::gdt::initialise();
    Log::print_status("OK", "GDT Initialised");
    
    arch::x86_64::cpu::idt::initialise();
    Log::print_status("OK", "IDT Initialised");
    
    mem::pmm::initialise();
    Log::print_status("OK", "PMM Initialised");
    
    mem::vmm::initialise();
    Log::print_status("OK", "VMM Initialised");

	mem::vmm::print_mem();

    mem::heap::initialise();
    Log::print_status("OK", "Heap Initialised");
    
    driver::pit::initialise();
    Log::print_status("OK", "PIT Initialised (FREQ=300)");

    /*Log::info("Disabling COM1 serial output, falling back to graphical interface");
    serial::serial_disable();
    Log::print_status("OK", "Serial Disabled");*/

    asm("sti");

    uacpi_status uacpi_result = uacpi_initialize(0);
    UACPI_ERROR("Initialise", 1);
    
    uacpi_result = uacpi_namespace_load();
    UACPI_ERROR("namespace loade", 1);
    
    uacpi_result = uacpi_namespace_initialize();
    UACPI_ERROR("namespace", 0);

    uacpi_result = uacpi_finalize_gpe_initialization();
    UACPI_ERROR("GPE", 0);

    asm("cli");

	tmpfs::initialise();
    tmpfs::mkdir("/dev", 0777);
    int stdin = tmpfs::open("/dev/stdin", O_CREAT | O_RDWR);
    int stdout = tmpfs::open("/dev/stdout", O_CREAT | O_RDWR);
    int stderr = tmpfs::open("/dev/stderr", O_CREAT | O_RDWR);
    Log::print_status("OK", "TMPFS Initialised");

    pci::initialise();
    Log::print_status("OK", "Detected all PCI devices");

    ahci::initialise();
    Log::print_status("OK", "AHCI Initialised");

    uint8_t buf[512];
    ssize_t num = ahci::ahci_driver_read(0, 0, 1, buf);
    for (int i = 0; i < sizeof(buf); i++) {
        printf("%c", buf[i]);
    }

	arch::x86_64::syscall::initialise();
    Log::print_status("OK", "Syscalls Initialised");

	asm("sti");
    while (1) {
    	asm volatile("hlt");
    }
    
    __builtin_unreachable();
}
