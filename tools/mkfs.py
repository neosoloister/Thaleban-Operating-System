import struct
import sys
import os

SECTOR_SIZE = 512
SECTORS_PER_CLUSTER = 1
RESERVED_SECTORS = 1
NUMBER_OF_FATS = 2
ROOT_ENTRIES = 512
TOTAL_SECTORS = 20480
MEDIA_DESCRIPTOR = 0xF8
SECTORS_PER_FAT = 40

def create_boot_sector():
    bs = bytearray([0xEB, 0x3C, 0x90])
    bs.extend(b'MSWIN4.1')
    bs.extend(struct.pack('<H', SECTOR_SIZE))
    bs.extend(struct.pack('<B', SECTORS_PER_CLUSTER))
    bs.extend(struct.pack('<H', RESERVED_SECTORS))
    bs.extend(struct.pack('<B', NUMBER_OF_FATS))
    bs.extend(struct.pack('<H', ROOT_ENTRIES))
    bs.extend(struct.pack('<H', TOTAL_SECTORS))
    bs.extend(struct.pack('<B', MEDIA_DESCRIPTOR))
    bs.extend(struct.pack('<H', SECTORS_PER_FAT))
    bs.extend(struct.pack('<H', 63))
    bs.extend(struct.pack('<H', 16))
    bs.extend(struct.pack('<I', 0))
    bs.extend(struct.pack('<I', 0))
    
    bs.extend(struct.pack('<B', 0x80))
    bs.extend(struct.pack('<B', 0))
    bs.extend(struct.pack('<B', 0x29))
    bs.extend(struct.pack('<I', 0x12345678))
    bs.extend(b'THALEBAN OS')
    bs.extend(b'FAT16   ')
    
    bs.extend(b'\x00' * (510 - len(bs)))
    bs.extend(b'\x55\xAA')
    
    return bs

def format_time(h, m, s):
    return (h << 11) | (m << 5) | (s // 2)

def format_date(y, m, d):
    return ((y - 1980) << 9) | (m << 5) | d

def create_root_entry(filename, ext, attr, first_cluster, size):
    entry = bytearray()
    entry.extend(filename.ljust(8, b' ')) # Preserve case for filename
    entry.extend(ext.ljust(3, b' ').lower()) # Force lowercase for extension
    entry.extend(struct.pack('<B', attr))
    entry.extend(b'\x00')
    entry.extend(b'\x00')
    entry.extend(struct.pack('<H', format_time(12, 0, 0)))
    entry.extend(struct.pack('<H', format_date(2024, 1, 1)))
    entry.extend(struct.pack('<H', format_date(2024, 1, 1)))
    entry.extend(struct.pack('<H', (first_cluster >> 16) & 0xFFFF))
    entry.extend(struct.pack('<H', format_time(12, 0, 0)))
    entry.extend(struct.pack('<H', format_date(2024, 1, 1)))
    entry.extend(struct.pack('<H', first_cluster & 0xFFFF))
    entry.extend(struct.pack('<I', size))
    return entry

def main():
    if len(sys.argv) < 2:
        print("Usage: python mkfs.py <output_image>")
        sys.exit(1)
        
    output_path = sys.argv[1]
    
    with open(output_path, 'wb') as f:
        f.write(b'\x00' * (TOTAL_SECTORS * SECTOR_SIZE))
        
    with open(output_path, 'r+b') as f:
        f.write(create_boot_sector())
        
        fat1_offset = RESERVED_SECTORS * SECTOR_SIZE
        fat2_offset = fat1_offset + (SECTORS_PER_FAT * SECTOR_SIZE)
        
        fat_sig = b'\xF8\xFF\xFF\xFF'
        
        f.seek(fat1_offset)
        f.write(fat_sig)
        f.seek(fat2_offset)
        f.write(fat_sig)
        
        for cluster in [2, 3, 4]:
            offset = cluster * 2
            f.seek(fat1_offset + offset)
            f.write(b'\xFF\xFF')
            f.seek(fat2_offset + offset)
            f.write(b'\xFF\xFF')
            
        root_offset = fat2_offset + (SECTORS_PER_FAT * SECTOR_SIZE)
        
        hello_content = b"Hello from ThaleFS!\n"
        f.seek(root_offset)
        
    print(f"Created FAT16 image at {output_path}")

if __name__ == "__main__":
    main()
