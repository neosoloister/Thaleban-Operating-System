#ifndef IDE_H
#define IDE_H

#include <stdint.h>

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

#define ATA_PRIMARY    0
#define ATA_SECONDARY  1
#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30

void ide_wait_ready();
void ide_select_drive(uint8_t bus, uint8_t drive);
void ide_read_disk(uint8_t bus, uint8_t drive, uint32_t lba, uint8_t *buffer);
void ide_write_disk(uint8_t bus, uint8_t drive, uint32_t lba, uint8_t *buffer);

#endif