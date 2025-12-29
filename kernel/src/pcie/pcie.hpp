#ifndef PCIE_HPP
#define PCIE_HPP 1

#include <cstdint>

struct pcie_device {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
    uint32_t bars[6];
    uint32_t cardbus_cis_ptr;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base;
    uint8_t capabilities_ptr;
    uint8_t reserved1[3];
    uint32_t reserved2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;

    uint8_t extended[3840];

    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t segment;
    void* config_space;
    
    struct pcie_device *next;
} __attribute__((packed));

namespace pcie {

uint64_t initialise();

pcie_device* get_device_bdf(uint8_t bus, uint8_t device, uint8_t function);
pcie_device* get_device_class_code(uint8_t class_code, uint8_t subclass, bool check_progif = false, uint8_t prog_if = 0);
pcie_device* get_device_vendor_id(uint16_t vendor_id, uint16_t device_id);

}

#endif
