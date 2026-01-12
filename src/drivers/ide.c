#include "ide.h"
#include "../cpu/ports.h"

typedef struct ide_device {
    uint16_t base_port;
    uint8_t drive_type;
    uint8_t exists;
}   ide_device_t;

void ide_wait_ready() {
    while (port_byte_in(0x1F7) & ATA_SR_BSY);
}

void ide_select_drive(uint8_t bus, uint8_t drive) {
    uint16_t base = (bus == ATA_PRIMARY) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    // 0xE0: LBA mode (Bit 6), Reserved (Bit 5,7 set to 1)
    // drive << 4: Select Master (0) or Slave (1)
    port_byte_out(base + 6, 0xE0 | ((drive & 0x01) << 4));
}

void ide_read_sector(uint32_t lba, uint8_t *buffer) {
    ide_wait_ready();
    ide_select_drive(ATA_PRIMARY, 0); // Always selecting Primary Master for now
    
    port_byte_out(ATA_PRIMARY_IO + 2, 1);   // Sector count
    port_byte_out(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    port_byte_out(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    port_byte_out(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PRIMARY_IO + 7, ATA_CMD_READ_PIO);

    ide_wait_ready();

    uint16_t *ptr = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = port_word_in(ATA_PRIMARY_IO);
    }
}

void ide_write_sector(uint32_t lba, uint8_t *buffer) {
    ide_wait_ready();
    ide_select_drive(ATA_PRIMARY, 0);

    port_byte_out(ATA_PRIMARY_IO + 2, 1);   // Sector count
    port_byte_out(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    port_byte_out(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    port_byte_out(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PRIMARY_IO + 7, ATA_CMD_WRITE_PIO);

    ide_wait_ready();

    uint16_t *ptr = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        port_word_out(ATA_PRIMARY_IO, ptr[i]);
    }
}