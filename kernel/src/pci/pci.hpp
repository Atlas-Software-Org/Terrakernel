#ifndef PCI_HPP
#define PCI_HPP 1

#include <cstdint>
#include <cstddef>

struct pci_device {
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
    uint32_t reserved;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;

    uint8_t bus, device, function;

    pci_device* next;
};

enum pci_get_type : uint32_t {
    GET_VIA_VENDOR_ID_DEVICE_ID           = 0x1,
    GET_VIA_CLASS_CODE_SUBCLASS_CODE     = 0x2,
    GET_VIA_BUS_DEVICE_FUNCTION_ADDRESS  = 0x4,
    GET_CHECK_PROG_IF                     = 0x4000
};

struct pci_query {
    uint16_t vendor_id  = 0;
    uint16_t device_id  = 0;
    uint8_t  class_code = 0;
    uint8_t  subclass_code = 0;
    uint8_t  bus = 0;
    uint8_t  device = 0;
    uint8_t  function = 0;
    pci_get_type type;
    uint8_t prog_if = 0;
};

namespace pci {

uint64_t initialise();

pci_device* get_device(const pci_query& query);
pci_device* get_device_bdf(uint8_t bus, uint8_t device, uint8_t function);
pci_device* get_device_class_code(uint8_t class_code, uint8_t subclass, bool check_progif = false, uint8_t prog_if = 0);
pci_device* get_device_vendor_id(uint16_t vendor_id, uint16_t device_id);

}

#endif
