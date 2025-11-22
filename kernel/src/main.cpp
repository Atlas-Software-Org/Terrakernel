#include <panic.hpp>
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
#include <unreal_fs/unrealfs.hpp>
#include <process/sched.hpp>

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

void proc0() {
    printf("[proc0] hello from proc0\n");
    scheduler::exit(scheduler::current_pid());
}

void proc1() {
    printf("[proc1] doing some work...\n");
    scheduler::exit(scheduler::current_pid());
}

void proc2() {
    printf("[proc2] finishing task\n");
    scheduler::exit(scheduler::current_pid());
}

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
    
    ufs::initialise();
    Log::print_status("OK", "UnrealFS Initialised");
    ufs::mkdir("/initrd");
    Log::print_status("OK", "Created INITRD directory");
    ufs::mkdir("/proc");
    Log::print_status("OK", "Created PROC directory");
    ufs::mkdir("/dev");
    Log::print_status("OK", "Created DEV directory");

    ufs::load_ustar_archive(
        module_request.response->modules[0]->address,
        module_request.response->modules[0]->size
    );
    
    printf("[main.cpp] scheduler::initialise(); at line 111 in kernel/src/main.cpp\n\r");
    scheduler::initialise();

    printf("[main.cpp] scheduler::spawn(proc0); at line 115 in kernel/src/main.cpp\n\r");
    scheduler::spawn(proc0);
    printf("[main.cpp] scheduler::spawn(proc1); at line 117 in kernel/src/main.cpp\n\r");
    scheduler::spawn(proc1);
    printf("[main.cpp] scheduler::spawn(proc2); at line 119 in kernel/src/main.cpp\n\r");
    scheduler::spawn(proc2);

    printf("[main.cpp] scheduler::begin(); at line 121 in kernel/src/main.cpp\n\r");
    scheduler::begin();

    while (1) {
        asm volatile("hlt");
    }
    
    __builtin_unreachable();
}
