#ifndef AHCI_HPP
#define AHCI_HPP 1

#include <cstdint>
#include <cstddef>
#include <types.hpp>

typedef struct fis_reg_h2d {
    uint8_t fis_type;
    uint8_t pmport:4;
    uint8_t reserved0:3;
    uint8_t c:1;
    uint8_t command;
    uint8_t featurel;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;

    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;

    uint8_t reserved1[4];
} fis_reg_h2d;

typedef struct hba_prdt_entry {
    uint32_t dba;
    uint32_t dbau;
    uint32_t dbc:22;
    uint32_t reserved:9;
    uint32_t i:1;
} hba_prdt_entry;

#define MAX_PRDT_ENTRIES 32

typedef struct hba_cmd_table {
    fis_reg_h2d cfis;
    uint8_t acmd[16];
    uint8_t reserved[48];
    hba_prdt_entry prdt_entry[MAX_PRDT_ENTRIES];
    int prdt_entry_count;
} hba_cmd_table;

typedef struct hba_cmd_header {
    uint8_t cfl:5;
    uint8_t a:1;
    uint8_t w:1;
    uint8_t p:1;
    uint8_t r:1;
    uint8_t b:1;
    uint8_t c:1;
    uint8_t reserved0:1;
    uint8_t prdtl;
    volatile uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t reserved1[4];
} hba_cmd_header;

struct hba_port {
    uint32_t PxCommandListBaseAddress;
    uint32_t PxCommandListBaseAddressUpper;
    uint32_t PxFISBaseAddress;
    uint32_t PxFISBaseAddressUpper;
    uint32_t PxInterruptStatus;
    uint32_t PxInterruptEnable;
    uint32_t PxCommandAndStatus;
    uint32_t Reserved0;
    uint32_t PxTaskFileData;
    uint32_t PxSignature;
    uint32_t PxSATAStatus;
    uint32_t PxSATAControl;
    uint32_t PxSATAError;
    uint32_t PxSATAActive;
    uint32_t PxCommandIssue;
    uint32_t PxSATANotification;
    uint32_t PxFISBasedSwitchingControl;
    uint32_t PxDeviceSleep;
    uint32_t Reserved1;
    uint32_t PxVendorSpecific;
};

struct hba_mem {
    uint32_t HostCapabilities;
    uint32_t GlobalHostControl;
    uint32_t InterruptStatus;
    uint32_t PortsImplemented;
    uint32_t Version;
    uint32_t CommandCompletionCoalescingControl;
    uint32_t CommandCompletionCoalsecingPorts; // ig intel spelled it wrong when writing the spec :>
    uint32_t EnclosureManagementLocation;
    uint32_t EnclosureManagementControl;
    uint32_t HostCapabilitiesExtended;
    uint32_t BIOS_OSHandoffControlAndStatus;

    uint8_t reserved[0x74];

    hba_port ports[32];
};

enum port_type {
    PORT_TYPE_SATA,
    PORT_TYPE_ATAPI,
    PORT_TYPE_PM,
    PORT_TYPE_EM,
    PORT_TYPE_NONE,
};

struct ahci_port {
    hba_port* port;
    uint8_t port_id;
    port_type type;
};

namespace ahci {

void initialise();

ssize_t ahci_driver_read(int port_id, uint64_t lba, size_t sector_count, void* outbuf);
ssize_t ahci_driver_write(int port_id, uint64_t lba, size_t sector_count, void* outbuf);

}

#endif