#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include "../drivers/ide.h"

#define SECTOR_SIZE 512

typedef struct __attribute__((packed)) {
    uint8_t jump[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} fat_bpb_t;

typedef struct __attribute__((packed)) {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} fat_dir_entry_t;

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_LONG_NAME 0x0F

void fat_init();
int fat_open(const char *filename);
int fat_read(char *buffer, uint32_t size);
void fat_list_current_dir();
int fat_change_dir(const char *dirname);
int fat_touch(const char *filename);
int fat_mkdir(const char *dirname);
int fat_cp(const char *src, const char *dest);
int fat_mv(const char *src, const char *dest);

#endif
