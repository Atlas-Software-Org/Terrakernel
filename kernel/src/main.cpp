#include <panic.hpp>
#include <cstring>
#include <lib/Flanterm/ftctx.h>
#include <drivers/serial/serial.hpp>
#include <drivers/serial/printf.h>
#include <cstdio>
#include <arch/arch.hpp>
#include <mem/mem.hpp>
#include <drivers/timers/pit.hpp>
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <uacpi/tables.h>
#include <tmpfs/tmpfs.hpp>
#include <pci/pci.hpp>
#include <drivers/blockio/ahci.hpp>
#include <exec/elf.hpp>
#include <sched/sched.hpp>
#include <drivers/input/ps2k/ps2k.hpp>
#include <drivers/input/ps2k/ps2k_key_event.hpp>
#include <pcie/pcie.hpp>

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

void line_discipline_test(const key_event& ev, void* userdata) {
	printf("Got line discipline callback, ev.keycode = 0x04%X; ev.state = %s\n\r", ev.keycode, ev.state == key_state::RELEASED ? "RELEASED" : "PRESSED");
}

extern "C" void init() {
    if (module_request.response == nullptr || module_request.response->module_count < 1) {
        asm volatile ("cli;hlt");
    }

    flanterm_initialise();
       
    serial::serial_enable();
    Log::printf_status("OK", "Flanterm Initialised"); // late
    Log::printf_status("OK", "Serial Initialised");
    
    arch::x86_64::cpu::gdt::initialise();
    Log::printf_status("OK", "GDT Initialised");
    
    arch::x86_64::cpu::idt::initialise();
    Log::printf_status("OK", "IDT Initialised");
    
    mem::pmm::initialise();
    Log::printf_status("OK", "PMM Initialised");
    
    mem::vmm::initialise();
    Log::printf_status("OK", "VMM Initialised");

	//mem::vmm::print_mem();

    mem::heap::initialise();
    Log::printf_status("OK", "Heap Initialised");
    
    drivers::timers::pit::initialise();
    Log::printf_status("OK", "PIT Initialised (FREQ=300)");

    Log::info("Disabling COM1 serial output, falling back to graphical interface");
    serial::serial_putc('\033');
    serial::serial_putc('[');
    serial::serial_putc('2');
    serial::serial_putc('J');
    serial::serial_putc('\033');
    serial::serial_putc('[');
    serial::serial_putc('H');
    //serial::serial_disable();
    Log::printf_status("OK", "Serial Disabled");

    asm("sti");
    
    uacpi_status uacpi_result = uacpi_initialize(0);
    UACPI_ERROR("Initialise", 1);
    
    uacpi_result = uacpi_namespace_load();
    UACPI_ERROR("namespace loade", 1);
    
    uacpi_result = uacpi_namespace_initialize();
    UACPI_ERROR("namespace", 0);

    uacpi_result = uacpi_finalize_gpe_initialization();
    UACPI_ERROR("GPE", 0);

	tmpfs::initialise();
    tmpfs::mkdir("/dev", 0777);
    int stdin = tmpfs::open("/dev/stdin", O_CREAT | O_RDWR);
    int stdout = tmpfs::open("/dev/stdout", O_CREAT | O_RDWR);
    int stderr = tmpfs::open("/dev/stderr", O_CREAT | O_RDWR);
    tmpfs::load_initrd(module_request.response->modules[0]->address, module_request.response->modules[0]->size);
    Log::printf_status("OK", "TMPFS Initialised");

    uint64_t npci = pci::initialise();
    Log::printf_status("OK", "Detected %zu PCI devices (Normal PCI is deprecated, use PCIe)", npci);

	uint64_t npcie = pcie::initialise();
	Log::printf_status("OK", "Detected %zu PCIe devices", npcie);

    ahci::initialise();
    Log::printf_status("OK", "AHCI Initialised");

	arch::x86_64::syscall::initialise();
    Log::printf_status("OK", "Syscalls Initialised");

	drivers::input::ps2k::initialise();
	Log::printf_status("OK", "PS2K Initialised");

	drivers::input::ps2k::flush_events();
	drivers::input::ps2k::set_event_callback((event_callback_fn)line_discipline_test, nullptr);

    sched::initialise();
    Log::printf_status("OK", "Scheduler Initialised");

    while (1) {
        asm volatile("hlt");
    }
    
    __builtin_unreachable();
}
