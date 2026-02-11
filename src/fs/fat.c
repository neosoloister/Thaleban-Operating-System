#include "fat.h"
#include "../drivers/vga.h"
#include "../libc/string.h"
#include "../kernel/kprintf.h"

static fat_bpb_t bpb;
static uint32_t fat_start_sector;
static uint32_t root_dir_start_sector;
static uint32_t data_start_sector;
static uint32_t current_dir_cluster = 0;

static struct {
    uint32_t start_cluster;
    uint32_t current_cluster;
    uint32_t size;
    uint32_t pointer;
} current_file;

static void read_cluster_sector(uint32_t cluster, uint32_t sector_offset, uint8_t *buffer) {
    if (cluster == 0) {
        ide_read_disk(ATA_PRIMARY, 1, root_dir_start_sector + sector_offset, buffer);
    } else {
        uint32_t cluster_start = data_start_sector + ((cluster - 2) * bpb.sectors_per_cluster);
        ide_read_disk(ATA_PRIMARY, 1, cluster_start + sector_offset, buffer);
    }
}

static void write_cluster_sector(uint32_t cluster, uint32_t sector_offset, uint8_t *buffer) {
    if (cluster == 0) {
        ide_write_disk(ATA_PRIMARY, 1, root_dir_start_sector + sector_offset, buffer);
    } else {
        uint32_t cluster_start = data_start_sector + ((cluster - 2) * bpb.sectors_per_cluster);
        ide_write_disk(ATA_PRIMARY, 1, cluster_start + sector_offset, buffer);
    }
}

static void read_sector(uint32_t sector, uint8_t *buffer) {
    ide_read_disk(ATA_PRIMARY, 1, sector, buffer);
}

static void write_sector(uint32_t sector, uint8_t *buffer) {
    ide_write_disk(ATA_PRIMARY, 1, sector, buffer);
}

static void set_fat_entry(uint32_t cluster, uint16_t value) {
    uint8_t buffer[512];
    uint32_t fat_offset = cluster * 2;
    uint32_t fat_sector = fat_start_sector + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;
    
    read_sector(fat_sector, buffer);
    *(uint16_t*)&buffer[ent_offset] = value;
    write_sector(fat_sector, buffer);
    write_sector(fat_sector + bpb.fat_size_16, buffer); 
}

static uint32_t find_free_cluster() {
    uint8_t buffer[512];
    uint32_t fat_sector = fat_start_sector;
    
    for (uint32_t s = 0; s < bpb.fat_size_16; s++) {
        read_sector(fat_sector + s, buffer);
        uint16_t *entries = (uint16_t*)buffer;
        for (int i = 0; i < 256; i++) {
            if (entries[i] == 0x0000) {
                 return (s * 256) + i;
            }
        }
    }
    return 0; 
}

static void parse_filename(const char *input, char *name, char *ext) {
    int i = 0;
    int j = 0;
    memset(name, ' ', 8);
    memset(ext, ' ', 3);
    
    while(input[i] && input[i] != '.' && j < 8) {
        char c = input[i++];
        name[j++] = c;
    }
    if (input[i] == '.') i++;
    j = 0;
    while(input[i] && j < 3) {
        char c = input[i++];
        if (c >= 'A' && c <= 'Z') c += 32;
        ext[j++] = c;
    }
}

static int find_dir_slot(uint32_t dir_cluster, fat_dir_entry_t *found_entry __attribute__((unused)), 
                         uint32_t *ret_sector_offset, uint32_t *ret_entry_offset,
                         int find_empty) {
    uint8_t buffer[512];
    uint32_t sectors_to_scan;
    
    if (dir_cluster == 0) {
        sectors_to_scan = (bpb.root_entries * 32) / 512;
    } else {
        sectors_to_scan = bpb.sectors_per_cluster;
    }
    
    for (uint32_t s = 0; s < sectors_to_scan; s++) {
        read_cluster_sector(dir_cluster, s, buffer);
        fat_dir_entry_t *entries = (fat_dir_entry_t*)buffer;
        
        for (int e = 0; e < 16; e++) {
            if (find_empty) {
                 if (entries[e].filename[0] == 0 || entries[e].filename[0] == 0xE5) {
                     *ret_sector_offset = s;
                     *ret_entry_offset = e;
                     return 0;
                 }
            } else {
            }
        }
    }
    return -1;
}

static int create_entry(uint32_t dir_cluster, const char *name, const char *ext, uint8_t attr, uint16_t cluster, uint32_t size) {
    uint32_t s_off, e_off;
    if (find_dir_slot(dir_cluster, 0, &s_off, &e_off, 1) != 0) return -1;
    
    uint8_t buffer[512];
    read_cluster_sector(dir_cluster, s_off, buffer);
    fat_dir_entry_t *entries = (fat_dir_entry_t*)buffer;
    
    memcpy(entries[e_off].filename, name, 8);
    memcpy(entries[e_off].extension, ext, 3);
    entries[e_off].attributes = attr;
    entries[e_off].reserved = 0;
    entries[e_off].create_time_tenth = 0;
    entries[e_off].create_time = 0;
    entries[e_off].create_date = 0;
    entries[e_off].last_access_date = 0;
    entries[e_off].first_cluster_high = 0;
    entries[e_off].write_time = 0;
    entries[e_off].write_date = 0;
    entries[e_off].first_cluster_low = cluster;
    entries[e_off].file_size = size;
    
    write_cluster_sector(dir_cluster, s_off, buffer);
    return 0;
}

static int find_entry(uint32_t dir_cluster, const char *name, const char *ext, fat_dir_entry_t *result_entry) {
    uint8_t buffer[512];
    uint32_t sectors_to_scan = (dir_cluster == 0) ? (bpb.root_entries * 32) / 512 : bpb.sectors_per_cluster;
    
    for (uint32_t s = 0; s < sectors_to_scan; s++) {
        read_cluster_sector(dir_cluster, s, buffer);
        fat_dir_entry_t *entry = (fat_dir_entry_t*)buffer;
        for (int e = 0; e < 16; e++) {
            if (entry[e].filename[0] == 0) return -1;
            if (entry[e].filename[0] == 0xE5) continue;
            
            if (memcmp(entry[e].filename, name, 8) != 0) continue;
            
            char entry_ext[3];
            for(int x=0; x<3; x++) {
                char c = entry[e].extension[x];
                if (c >= 'A' && c <= 'Z') c += 32;
                entry_ext[x] = c;
            }
            
            if (memcmp(entry_ext, ext, 3) == 0) {
                *result_entry = entry[e];
                return 0;
            }
        }
    }
    return -1;
}

void fat_init() {
    uint8_t buffer[512];
    read_sector(0, buffer);
    memcpy(&bpb, buffer, sizeof(fat_bpb_t));
    fat_start_sector = bpb.reserved_sectors;
    root_dir_start_sector = fat_start_sector + (bpb.num_fats * bpb.fat_size_16);
    data_start_sector = root_dir_start_sector + ((bpb.root_entries * 32) / 512); 
    current_dir_cluster = 0;
    kprintf("FAT Driver Initialized.\n");
}

int fat_open(const char *filename) {
    char name[8], ext[3];
    parse_filename(filename, name, ext);
    fat_dir_entry_t entry;
    if (find_entry(current_dir_cluster, name, ext, &entry) == 0) {
        if (entry.attributes & ATTR_DIRECTORY) return -1;
        current_file.start_cluster = entry.first_cluster_low;
        current_file.size = entry.file_size;
        current_file.current_cluster = current_file.start_cluster;
        current_file.pointer = 0;
        return current_file.size;
    }
    return -1;
}

int fat_read(char *buffer, uint32_t size) {
    if (current_file.size == 0) return 0;
    uint32_t sector = data_start_sector + ((current_file.start_cluster - 2) * bpb.sectors_per_cluster);
    uint8_t disk_buf[512];
    read_sector(sector, disk_buf);
    uint32_t to_read = size > current_file.size ? current_file.size : size;
    if (to_read > 512) to_read = 512;
    memcpy(buffer, disk_buf, to_read);
    return to_read;
}

int fat_change_dir(const char *dirname) {
    if (strcmp(dirname, ".") == 0) return 0;
    if (strcmp(dirname, "..") == 0) {
        if (current_dir_cluster == 0) return 0; 
        fat_dir_entry_t entry;
        char name[8] = "..      ";
        char ext[3] = "   ";
        if (find_entry(current_dir_cluster, name, ext, &entry) == 0) {
             current_dir_cluster = entry.first_cluster_low;
             return 0;
        }
        current_dir_cluster = 0; 
        return 0;
    }
    char name[8], ext[3];
    parse_filename(dirname, name, ext);
    fat_dir_entry_t entry;
    if (find_entry(current_dir_cluster, name, ext, &entry) == 0) {
        if (entry.attributes & ATTR_DIRECTORY) {
            current_dir_cluster = entry.first_cluster_low;
            return 0;
        }
    }
    return -1;
}

void fat_list_current_dir() {
    uint8_t buffer[512];
    uint32_t sectors_to_scan = (current_dir_cluster == 0) ? (bpb.root_entries * 32) / 512 : bpb.sectors_per_cluster;
    
    kprintf("Listing Directory\n");
    for (uint32_t s = 0; s < sectors_to_scan; s++) {
        read_cluster_sector(current_dir_cluster, s, buffer);
        fat_dir_entry_t *entry = (fat_dir_entry_t*)buffer;
        for (int e = 0; e < 16; e++) {
            if (entry[e].filename[0] == 0) return;
            if (entry[e].filename[0] == 0xE5) continue;
            if (entry[e].attributes & ATTR_LONG_NAME) continue;
            if (entry[e].filename[0] == '.') continue;
            
            char name[13];
            int k = 0;
            for(int n=0;n<8;n++) {
                if(entry[e].filename[n] != ' ') {
                     char c = entry[e].filename[n];
                     name[k++] = c;
                }
            }
            if(entry[e].extension[0] != ' ') {
                name[k++] = '.';
                for(int n=0;n<3;n++) {
                    if(entry[e].extension[n] != ' ') {
                        char c = entry[e].extension[n];
                        if (c >= 'A' && c <= 'Z') c += 32;
                        name[k++] = c;
                    }
                }
            }
            name[k] = 0;
            kprintf(" - "); kprintf(name);
            if (entry[e].attributes & ATTR_DIRECTORY) kprintf(" [DIR]");
            kprintf("\n");
        }
    }
}

int fat_touch(const char *filename) {
    char name[8], ext[3];
    parse_filename(filename, name, ext);
    
    fat_dir_entry_t exist;
    if (find_entry(current_dir_cluster, name, ext, &exist) == 0) return -1;
    
    return create_entry(current_dir_cluster, name, ext, 0x20, 0, 0);
}

int fat_mkdir(const char *dirname) {
    char name[8], ext[3];
    parse_filename(dirname, name, ext);
    
    fat_dir_entry_t exist;
    if (find_entry(current_dir_cluster, name, ext, &exist) == 0) return -1;
    
    uint32_t new_cluster = find_free_cluster();
    if (new_cluster == 0) return -1;
    
    set_fat_entry(new_cluster, 0xFFFF);
    
    uint8_t buffer[512];
    memset(buffer, 0, 512);
    fat_dir_entry_t *dot = (fat_dir_entry_t*)buffer;
    
    memcpy(dot[0].filename, ".       ", 8);
    memcpy(dot[0].extension, "   ", 3);
    dot[0].attributes = ATTR_DIRECTORY;
    dot[0].first_cluster_low = new_cluster;
    
    memcpy(dot[1].filename, "..      ", 8);
    memcpy(dot[1].extension, "   ", 3);
    dot[1].attributes = ATTR_DIRECTORY;
    dot[1].first_cluster_low = current_dir_cluster;
    
    write_cluster_sector(new_cluster, 0, buffer);
    
    return create_entry(current_dir_cluster, name, ext, ATTR_DIRECTORY, new_cluster, 0);
}

int fat_cp(const char *src, const char *dest) {
    int size = fat_open(src);
    if (size < 0) return -1;
    
    uint8_t buffer[512];
    fat_read((char*)buffer, 512); 
    
    char name[8], ext[3];
    parse_filename(dest, name, ext);
    
    uint32_t new_cluster = find_free_cluster();
    if (new_cluster == 0) return -1;
    set_fat_entry(new_cluster, 0xFFFF);
    
    write_cluster_sector(new_cluster, 0, buffer);
    
    return create_entry(current_dir_cluster, name, ext, 0x20, new_cluster, size > 512 ? 512 : size);
}

int fat_mv(const char *src, const char *dest) {
    char s_name[8], s_ext[3];
    parse_filename(src, s_name, s_ext);
    
    char d_name[8], d_ext[3];
    parse_filename(dest, d_name, d_ext);
    
    uint8_t buffer[512];
    
    uint32_t sectors_to_scan = (current_dir_cluster == 0) ? (bpb.root_entries * 32) / 512 : bpb.sectors_per_cluster;
    for (uint32_t s = 0; s < sectors_to_scan; s++) {
        read_cluster_sector(current_dir_cluster, s, buffer);
        fat_dir_entry_t *ent = (fat_dir_entry_t*)buffer;
        for (int e = 0; e < 16; e++) {
            if (ent[e].filename[0] == 0) return -1;
            if (ent[e].filename[0] == 0xE5) continue;
            if (memcmp(ent[e].filename, s_name, 8) == 0 && memcmp(ent[e].extension, s_ext, 3) == 0) {
                 memcpy(ent[e].filename, d_name, 8);
                 memcpy(ent[e].extension, d_ext, 3);
                 write_cluster_sector(current_dir_cluster, s, buffer);
                 return 0;
            }
        }
    }
    return -1;
}

int fat_delete_file(const char *filename) {
    char name[8], ext[3];
    parse_filename(filename, name, ext);
    
    fat_dir_entry_t entry;
    // uint32_t sector_offset, entry_offset;
    
    // Find the entry to get its details (cluster)
    if (find_entry(current_dir_cluster, name, ext, &entry) != 0) return -1;
    
    // Find the entry again but this time get its offset location so we can modify it
    // if (find_dir_slot(current_dir_cluster, &entry, &sector_offset, &entry_offset, 0) != 0) {
        
        uint8_t buffer[512];
        uint32_t sectors_to_scan = (current_dir_cluster == 0) ? (bpb.root_entries * 32) / 512 : bpb.sectors_per_cluster;
        
        int found = 0;
        for (uint32_t s = 0; s < sectors_to_scan; s++) {
             read_cluster_sector(current_dir_cluster, s, buffer);
             fat_dir_entry_t *entries = (fat_dir_entry_t*)buffer;
             for (int e = 0; e < 16; e++) {
                 if (entries[e].filename[0] == 0) break; 
                 if (entries[e].filename[0] == 0xE5) continue;
                 
                 if (memcmp(entries[e].filename, name, 8) == 0 && memcmp(entries[e].extension, ext, 3) == 0) {
                     // Found it! Mark as deleted
                     entries[e].filename[0] = 0xE5;
                     write_cluster_sector(current_dir_cluster, s, buffer);
                     found = 1;
                     goto cleanup_clusters;
                 }
             }
        }
        if (!found) return -1;
    // }

cleanup_clusters:;
    // Free the cluster chain
    uint32_t cluster = entry.first_cluster_low;
    while (cluster >= 2 && cluster < 0xFFF7) {
        uint32_t fat_offset = cluster * 2;
        uint32_t fat_sector = fat_start_sector + (fat_offset / 512);
        uint32_t ent_offset = fat_offset % 512;
        
        uint8_t fat_buf[512];
        read_sector(fat_sector, fat_buf);
        uint16_t next_cluster = *(uint16_t*)&fat_buf[ent_offset];
        
        // precise single entry write update
        set_fat_entry(cluster, 0x0000);
        
        cluster = next_cluster;
    }
    
    return 0;
}

int fat_write_file(const char *filename, const char *buffer, uint32_t size) {
    // simplistic implementation: delete if exists, then create new
    fat_delete_file(filename);
    
    char name[8], ext[3];
    parse_filename(filename, name, ext);
    
    // Allocate first cluster
    uint32_t start_cluster = find_free_cluster();
    if (start_cluster == 0) return -1;
    
    set_fat_entry(start_cluster, 0xFFFF);
    
    uint32_t current_cluster = start_cluster;
    uint32_t bytes_written = 0;
    
    while (bytes_written < size) {
        
        // Write to current cluster, sector 0 (assuming we step cluster by cluster)
        // If sectors_per_cluster > 1, we should fill all sectors.
        // Let's assume for now we just use 1 sector of the cluster or simply allocate a new cluster for every 512 bytes 
        // if sectors_per_cluster is small, OR validly loop sectors.
        
        // Let's try to fill the cluster.
        for (int i = 0; i < bpb.sectors_per_cluster && bytes_written < size; i++) {
             uint8_t sector_buf[512];
             memset(sector_buf, 0, 512);
             
             uint32_t chunk = size - bytes_written;
             if (chunk > 512) chunk = 512;
             
             memcpy(sector_buf, buffer + bytes_written, chunk);
             write_cluster_sector(current_cluster, i, sector_buf);
             bytes_written += chunk;
        }
        
        if (bytes_written < size) {
            // We need another cluster
            uint32_t new_cluster = find_free_cluster();
            if (new_cluster == 0) return -1; // Out of space
            
            set_fat_entry(current_cluster, new_cluster);
            set_fat_entry(new_cluster, 0xFFFF);
            current_cluster = new_cluster;
        }
    }
    
    return create_entry(current_dir_cluster, name, ext, 0x20, start_cluster, size);
}
