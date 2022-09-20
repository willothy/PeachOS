#include "disk/disk.h"
#include "config.h"
#include "io/io.h"
#include "memory/memory.h"
#include "status.h"

// Primary disk
struct disk disk;

int disk_read_sector(int lba, int total, void* buffer) {
    // Send highest 8 bits of LBA to hdd controller
    outb(0x1f6, (lba >> 24) | 0xe0);
    // Send total sectors to read
    outb(0x1f2, total);
    // Send lower 8 bits to the hdd controller
    outb(0x1f3, (unsigned char)(lba & 0xff));
    // Send next 8 bits to the hdd controller
    outb(0x1f4, (unsigned char)(lba >> 8));
    // Send next 8 bits to the hdd controller
    outb(0x1f5, (unsigned char)(lba >> 16));
    // Send read command to hdd controller
    outb(0x1f7, 0x20);

    unsigned short* ptr = (unsigned short*)buffer;

    for (int b = 0; b < total; b++) {
        // Wait for buffer to be ready
        char c = insb(0x1f7);
        while (!(c & 0x08)) {
            c = insb(0x1f7);
        }

        // Copy from disk to memory
        for (int i = 0; i < 256; i++) {
            *ptr = insw(0x1f0);
            ptr++;
        }
    }

    return 0;
}

void disk_search_and_init() {
    // TODO: Search for disks
    memset(&disk, 0, sizeof(struct disk));
    disk.type = PEACHOS_DISK_TYPE_REAL;
    disk.sector_size = PEACHOS_SECTOR_SIZE;
}

struct disk* disk_get(int index) {
    if (index != 0)
        return 0;

    return &disk;
}

int disk_read_block(struct disk* idisk, unsigned int lba, int total,
                    void* buf) {
    if (idisk != &disk) {
        return -EIO;
    }

    return disk_read_sector(lba, total, buf);
}