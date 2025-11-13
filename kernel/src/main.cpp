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

extern "C" void init() {
    flanterm_initialise();
    
    serial::serial_enable();
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
    Log::print_status("OK", "PIT Initialised");

    /*Log::info("Disabling COM1 serial output, falling back to graphical interface");
    serial::serial_disable();
    Log::print_status("OK", "Serial Disabled");*/

    asm volatile("sti");

    uacpi_status uacpi_result = uacpi_initialize(0);
    if (uacpi_unlikely_error(uacpi_result)) {
        Log::errf("uACPI Initialisation Failed: %s", uacpi_status_to_string(uacpi_result), uacpi_result);
        asm volatile("cli; hlt;");
    }
    Log::print_status("OK", "uACPI Initialised");

    uacpi_result = uacpi_namespace_load();
    if (uacpi_unlikely_error(uacpi_result)) {
        Log::errf("uACPI Namespace Load Failed: %s", uacpi_status_to_string(uacpi_result), uacpi_result);
        asm volatile("cli; hlt;");
    }
    Log::print_status("OK", "uACPI Namespace Loaded");

    uacpi_result = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(uacpi_result)) {
        Log::errf("uACPI Namespace Initialisation Failed: %s", uacpi_status_to_string(uacpi_result), uacpi_result);
        asm volatile("cli; hlt;");
    }
    Log::print_status("OK", "uACPI Namespace Initialised");

    uacpi_result = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(uacpi_result)) {
        Log::errf("uACPI GPE Initialisation Failed: %s", uacpi_status_to_string(uacpi_result), uacpi_result);
        asm volatile("cli; hlt;");
    }
    Log::print_status("OK", "uACPI GPE Initialised");

    unreal_fs::initialise();
    Log::print_status("OK", "UnrealFS Initialised");

    unreal_fs::unreal_node* vd0 = unreal_fs::create_directory("/VirtualDisk0");
    if (vd0) {
        Log::print_status("OK", "Created /VirtualDisk0 directory");
    } else {
        Log::errf("Failed to create /VirtualDisk0 directory");
    }

    unreal_fs::mount_virtual_disk(
        (void*)unreal_fs::modules::get_first_module()->address,
        unreal_fs::modules::get_first_module()->length,
        "/VirtualDisk0"
    );

    while (1) {
        asm volatile("hlt");
    }

    __builtin_unreachable();
}
