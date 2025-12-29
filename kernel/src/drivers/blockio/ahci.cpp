#include "ahci.hpp"
#include <pcie/pcie.hpp>
#include <cstdio>
#include <mem/mem.hpp>
#include <arch/arch.hpp>

namespace ahci {

enum port_type {
    AHCI_PORT_TYPE_NONE,
    AHCI_PORT_TYPE_SATA,
    AHCI_PORT_TYPE_SATAPI,
    AHCI_PORT_TYPE_SEMB,
    AHCI_PORT_TYPE_PM,
};

struct ahci_port {
    HBAPort* port;
    int portnum;
    port_type type;
};

pcie_device* AHCI;
HBAMem* ABAR;
ahci_port* ports[32];

#define SATA_SIG_ATA    0x00000101
#define SATA_SIG_ATAPI  0xEB140101
#define SATA_SIG_SEMB   0xC33C0101
#define SATA_SIG_PM     0x96690101

#define HBA_PORT_IPM_ACTIVE  0x01
#define HBA_PORT_DET_PRESENT 0x03

port_type get_port_type(HBAPort* port) {
    uint32_t ssts = port->PxSATAStatus;
    uint8_t det = (ssts >> 8) & 0x0F;
    uint8_t ipm = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) {
        Log::errf("HBA port DET not present");
        return AHCI_PORT_TYPE_NONE;
    }

    if (ipm != HBA_PORT_IPM_ACTIVE) {
        Log::errf("HBA port IPM not active");
        return AHCI_PORT_TYPE_NONE;
    }

    switch (port->PxSignature) {
        case SATA_SIG_ATA:
            return AHCI_PORT_TYPE_SATA;
        case SATA_SIG_ATAPI:
            return AHCI_PORT_TYPE_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_PORT_TYPE_SEMB;
        case SATA_SIG_PM:
            return AHCI_PORT_TYPE_PM;
        default:
            Log::warnf("Could not determine port type (%08X)", port->PxSignature);
            return AHCI_PORT_TYPE_NONE;
    }
}

void setup_port(HBAPort* port) {
    port_type type = get_port_type(port);
    if (type != AHCI_PORT_TYPE_SATA && type != AHCI_PORT_TYPE_SATAPI) {
        Log::infof("Skipping port... port is neither SATA nor SATAPI");
        return;
    }
}

bool ATA_MODE = false;

void initialise() {
    AHCI = pcie::get_device_class_code(0x01, 0x06, true, 0x01);
    if (!AHCI) {
        Log::errf("Failed to get AHCI device... Halting");
        asm volatile ("cli;hlt;");
    }

    uint32_t bar5 = AHCI->bars[5] & ~0xF;
    ABAR = (HBAMem*)(uint64_t)bar5;

    printf("ABAR= %p\n\r", ABAR);
    
    const uint32_t pi = ABAR->PortsImplemented;
    int last_implemented = -1;
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) last_implemented = i;
    }
    if (last_implemented == -1) {
        Log::errf("No ports implemented... PI = %08X", pi);
        asm volatile ("cli;hlt;");
    }

    size_t abar_size = sizeof(HBAMem) + (last_implemented * sizeof(HBAPort));
    mem::vmm::mmap((void*)ABAR, (void*)ABAR, (abar_size + 0xFFF) / 0x1000, PAGE_PRESENT | PAGE_RW | PAGE_PCD);

    if (!(ABAR->GlobalHostControl & 0x80000000)) {
        Log::warnf("IDE Emulation mode...");
        ATA_MODE = true;
    }

    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            HBAPort* port = &ABAR->Ports[i];
            setup_port(port);
        }
    }
}

#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_FEATURES   0x1F1
#define ATA_SECCOUNT0  0x1F2
#define ATA_LBA0       0x1F3
#define ATA_LBA1       0x1F4
#define ATA_LBA2       0x1F5
#define ATA_HDDEVSEL   0x1F6
#define ATA_COMMAND    0x1F7
#define ATA_STATUS     0x1F7
#define ATA_SECCOUNT1  0x1F2
#define ATA_LBA3       0x1F3
#define ATA_LBA4       0x1F4
#define ATA_LBA5       0x1F5
#define ATA_CONTROL    0x3F6

#define ATA_CMD_READ_PIO_EXT  0x24
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_SR_BSY  0x80
#define ATA_SR_DRQ  0x08

ssize_t ata_transfer(int port_id, uint64_t lba, size_t sector_count, void* buffer, bool is_write_op) {
    uint8_t device = 0x40;

    uint8_t lba_bytes[6];
    for (int i = 0; i < 6; i++) {
        lba_bytes[i] = (lba >> (8 * i)) & 0xFF;
    }

    uint8_t sector_bytes[2];
    sector_bytes[0] = sector_count & 0xFF;
    sector_bytes[1] = (sector_count >> 8) & 0xFF;

    auto wait_bsy_clear = []() {
        while (arch::x86_64::io::inb(ATA_STATUS) & ATA_SR_BSY) {}
    };

    wait_bsy_clear();

    arch::x86_64::io::outb(ATA_HDDEVSEL, device);
    arch::x86_64::io::outb(ATA_SECCOUNT1, sector_bytes[1]);
    arch::x86_64::io::outb(ATA_LBA3, lba_bytes[3]);
    arch::x86_64::io::outb(ATA_LBA4, lba_bytes[4]);
    arch::x86_64::io::outb(ATA_LBA5, lba_bytes[5]);

    arch::x86_64::io::outb(ATA_SECCOUNT0, sector_bytes[0]);
    arch::x86_64::io::outb(ATA_LBA0, lba_bytes[0]);
    arch::x86_64::io::outb(ATA_LBA1, lba_bytes[1]);
    arch::x86_64::io::outb(ATA_LBA2, lba_bytes[2]);

    arch::x86_64::io::outb(ATA_COMMAND, is_write_op ? ATA_CMD_WRITE_PIO_EXT : ATA_CMD_READ_PIO_EXT);

    uint16_t* buf16 = reinterpret_cast<uint16_t*>(buffer);

    for (size_t i = 0; i < sector_count; i++) {
        while (!(arch::x86_64::io::inb(ATA_STATUS) & ATA_SR_DRQ)) {}

        if (is_write_op) {
            for (int w = 0; w < 256; w++) {
                arch::x86_64::io::outw(ATA_DATA, buf16[i * 256 + w]);
            }
        } else {
            for (int w = 0; w < 256; w++) {
                buf16[i * 256 + w] = arch::x86_64::io::inw(ATA_DATA);
            }
        }
    }

    wait_bsy_clear();
    return sector_count * 512;
}

ssize_t ahci_transfer(int port_id, uint64_t lba, size_t sector_count, void* buffer, bool is_write_op) {

}

ssize_t ahci_driver_read(int port_id, uint64_t lba, size_t sector_count, void* buffer) {
    if (ATA_MODE) {
        return ata_transfer(port_id, lba, sector_count, buffer, false);
    } else {
        return ahci_transfer(port_id, lba, sector_count, buffer, false);
    }
}

ssize_t ahci_driver_write(int port_id, uint64_t lba, size_t sector_count, void* buffer) {
    if (ATA_MODE) {
        return ata_transfer(port_id, lba, sector_count, buffer, true);
    } else {
        return ahci_transfer(port_id, lba, sector_count, buffer, true);
    }
}

}
