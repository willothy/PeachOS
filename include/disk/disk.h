#ifndef DISK_H
#define DISK_H

typedef unsigned int PEACHOS_DISK_TYPE;

#define PEACHOS_DISK_TYPE_REAL 0

struct disk {
    PEACHOS_DISK_TYPE type;
    int sector_size;

    // Disk id
    int id;

    struct filesystem* filesystem;

    // Private data for filesystem
    void* fs_private;
};

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* disk, unsigned int lba, int total, void* buf);

#endif // DISK_H
