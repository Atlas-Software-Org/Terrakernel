#include "ahci.hpp"
#include <pci/pci.hpp>
#include <tmpfs/tmpfs.hpp>
#include <drivers/serial/print.hpp>
#include <mem/mem.hpp>

static pci_device* ahci_dev;
static int ahci_dev_fd;
static hba_mem* abar;

#define MAX_PORTS 32
#define SECTOR_SIZE 512
#define MAX_PRDT_BYTES (4 * 1024 * 1024)

ahci_port ports[MAX_PORTS];

namespace ahci {

port_type get_port_type(hba_port* port, int id) {
    uint32_t ssts = port->PxSATAStatus;
    uint32_t det  = ssts & 0x0F;
    if (det != 0x03) {
        printf("DET doesn't equal 0x03 (%02X)\n\r", det);
        return PORT_TYPE_NONE;
    }

    uint8_t ipm = (ssts & 0xF00) >> 8;
    if (ipm != 1) {
        printf("IPM is not 1 (%02X)\n\r", ipm);
        return PORT_TYPE_NONE;
    }

    printf("PORT SIG: %08llX\n\r", port->PxSignature);
    switch (port->PxSignature) {
        case 0x00000101: printf("Port %d is SATA\n\r", id); return PORT_TYPE_SATA;
        case 0xEB140101: printf("Port %d is ATAPI\n\r", id); return PORT_TYPE_ATAPI;
        case 0x96690101: printf("Port %d is Port Multiplier\n\r", id); return PORT_TYPE_PM;
        case 0xC33C0101: printf("Port %d is Enclosure Mgmt\n\r", id); return PORT_TYPE_EM;
        case 0: return PORT_TYPE_NONE;
        default: return PORT_TYPE_NONE;
    }
}

void initialise() {
    ahci_dev = pci::get_device_class_code(0x01, 0x06, true, 0x01);
    if (!ahci_dev) asm volatile("cli; hlt");

    ahci_dev_fd = tmpfs::open("/dev/ahci_ctrl", O_CREAT | O_RDWR);

    uint32_t bar5 = ahci_dev->bars[5];
    void* abar_phys = (void*)(bar5 & ~(0xF));
    void* abar_virt = (void*)mem::vmm::pa_to_va((uint64_t)abar_phys);
    mem::vmm::mmap(abar_phys, abar_virt, 1, PAGE_RW);
    abar = (hba_mem*)(abar_virt);

    printf("ABAR=%p\n\r", abar);
    Log::infof("PortsImplemented = %08X", abar->PortsImplemented);

    uint32_t pi = abar->PortsImplemented;
    int count = 0;
    for (int i = 0; i < MAX_PORTS; i++) {
        if (pi & (1 << i)) {
            ports[count].port = &abar->ports[i];
            ports[count].port_id = i;
            ports[count].type = get_port_type(&abar->ports[i], i);
            printf("Attached port at index %d (port_id=%d/type=%d)\n\r", count, i, ports[count].type);
            count++;
        }
    }
}

int find_free_slot(hba_port* port) {
    uint32_t slots = port->PxSATAActive | port->PxCommandIssue;
    for (int i = 0; i < 32; i++)
        if (!(slots & (1 << i))) return i;
    return -1;
}

ssize_t ahci_driver_read(int port_index, uint64_t lba, size_t sector_count, void* outbuf) {
    hba_port* port = ports[port_index].port;
    while (port->PxCommandAndStatus & (1 << 15));

    int slot = find_free_slot(port);
    if (slot < 0) return -1;

    hba_cmd_header* cmd_headers = (hba_cmd_header*)mem::vmm::valloc(1);
    mem::memset(cmd_headers, 0, sizeof(hba_cmd_header) * 32);

    hba_cmd_header* cmdheader = &cmd_headers[slot];
    cmdheader->cfl = sizeof(fis_reg_h2d) / sizeof(uint32_t);
    cmdheader->w = 0;
    cmdheader->prdtl = (sector_count * SECTOR_SIZE + MAX_PRDT_BYTES - 1) / MAX_PRDT_BYTES;

    hba_cmd_table* cmdtable = (hba_cmd_table*)mem::vmm::valloc(1);
    mem::memset(cmdtable, 0, sizeof(hba_cmd_table));
    cmdheader->ctba  = (uint32_t)(uintptr_t)cmdtable;
    cmdheader->ctbau = ((uintptr_t)cmdtable >> 32) & 0xFFFFFFFF;

    size_t bytes_remaining = sector_count * SECTOR_SIZE;
    uint8_t* ptr = (uint8_t*)outbuf;
    for (int i = 0; i < cmdheader->prdtl; i++) {
        size_t bytes_to_transfer = (bytes_remaining > MAX_PRDT_BYTES) ? MAX_PRDT_BYTES : bytes_remaining;
        cmdtable->prdt_entry[i].dba  = (uint32_t)(uintptr_t)ptr;
        cmdtable->prdt_entry[i].dbau = ((uintptr_t)ptr >> 32) & 0xFFFFFFFF;
        cmdtable->prdt_entry[i].dbc  = bytes_to_transfer - 1;
        cmdtable->prdt_entry[i].i    = 1;
        bytes_remaining -= bytes_to_transfer;
        ptr += bytes_to_transfer;
    }

    port->PxCommandIssue |= (1 << slot);
    while (port->PxCommandIssue & (1 << slot));

    if (port->PxSATAError) return -1;
    return sector_count * SECTOR_SIZE;
}

ssize_t ahci_driver_write(int port_index, uint64_t lba, size_t sector_count, void* inbuf) {
    hba_port* port = ports[port_index].port;
    while (port->PxCommandAndStatus & (1 << 15));

    int slot = find_free_slot(port);
    if (slot < 0) return -1;

    hba_cmd_header* cmd_headers = (hba_cmd_header*)mem::vmm::valloc(1);
    mem::memset(cmd_headers, 0, sizeof(hba_cmd_header) * 32);

    hba_cmd_header* cmdheader = &cmd_headers[slot];
    cmdheader->cfl = sizeof(fis_reg_h2d) / sizeof(uint32_t);
    cmdheader->w = 1;
    cmdheader->prdtl = (sector_count * SECTOR_SIZE + MAX_PRDT_BYTES - 1) / MAX_PRDT_BYTES;

    hba_cmd_table* cmdtable = (hba_cmd_table*)mem::vmm::valloc(1);
    mem::memset(cmdtable, 0, sizeof(hba_cmd_table));
    cmdheader->ctba  = (uint32_t)(uintptr_t)cmdtable;
    cmdheader->ctbau = ((uintptr_t)cmdtable >> 32) & 0xFFFFFFFF;

    size_t bytes_remaining = sector_count * SECTOR_SIZE;
    uint8_t* ptr = (uint8_t*)inbuf;
    for (int i = 0; i < cmdheader->prdtl; i++) {
        size_t bytes_to_transfer = (bytes_remaining > MAX_PRDT_BYTES) ? MAX_PRDT_BYTES : bytes_remaining;
        cmdtable->prdt_entry[i].dba  = (uint32_t)(uintptr_t)ptr;
        cmdtable->prdt_entry[i].dbau = ((uintptr_t)ptr >> 32) & 0xFFFFFFFF;
        cmdtable->prdt_entry[i].dbc  = bytes_to_transfer - 1;
        cmdtable->prdt_entry[i].i    = 1;
        bytes_remaining -= bytes_to_transfer;
        ptr += bytes_to_transfer;
    }

    port->PxCommandIssue |= (1 << slot);
    while (port->PxCommandIssue & (1 << slot));

    if (port->PxSATAError) return -1;
    return sector_count * SECTOR_SIZE;
}

}