#include "fs/fat/fat16.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "string/string.h"
#include "vga_writer.h"

#include <stdint.h>

#define PEACHOS_FAT16_SIGNATURE 0x29
#define PEACHOS_FAT16_FAT_ENTRY_SIZE 0x02
#define PEACHOS_FAT16_BAD_SECTOR 0xFF7
#define PEACHOS_FAT16_UNUSED 0x00

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// FAT16 directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

struct fat_header_extended {
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header {
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_h {
    struct fat_header primary_header;
    union fat_h_e {
        struct fat_header_extended extended_header;
    } shared;
};

struct fat_directory_item {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t file_size;
} __attribute__((packed));

struct fat_directory {
    struct fat_directory_item* item;
    int total;
    int start_sector_pos;
    int ending_sector_pos;
};

struct fat_item {
    union {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_private {
    struct fat_h header;
    struct fat_directory root_directory;

    // Used to stream data clusters
    struct disk_stream* cluster_read_stream;

    // Used to stream the FAT
    struct disk_stream* fat_read_stream;

    // Used in situations where we stream the directory
    struct disk_stream* directory_stream;
};

struct fat_file_descriptor {
    struct fat_item* item;
    uint32_t pos;
};

int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);

struct filesystem fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open,
};

struct filesystem* fat16_init() {
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

static void fat16_init_private(struct disk* disk, struct fat_private* private) {
    memset(private, 0, sizeof(struct fat_private));
    private->cluster_read_stream = diskstream_new(disk->id);
    private->fat_read_stream = diskstream_new(disk->id);
    private->directory_stream = diskstream_new(disk->id);
}

int fat16_sector_to_absolute(struct disk* disk, int sector) {
    return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(struct disk* disk,
                                        uint32_t directory_start_sector) {
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct fat_private* fat_private = disk->fs_private;

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;

    struct disk_stream* stream = fat_private->directory_stream;
    if (diskstream_seek(stream, directory_start_pos) != AOK) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    while (1) {
        if (diskstream_read(stream, &item, sizeof(item)) != AOK) {
            res = -EIO;
            vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)),
                      VGA_RED);
            goto out;
        }

        if (item.filename[0] == 0x00) {
            // End of directory
            break;
        }

        if (item.filename[0] == 0xE5) {
            // Unused file
            continue;
        }

        i++;
    }

    res = i;

out:
    return res;
}

int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private,
                             struct fat_directory* directory) {
    int res = 0;

    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos =
        (primary_header->fat_copies * primary_header->sectors_per_fat) +
        primary_header->reserved_sectors;

    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = root_dir_entries * sizeof(struct fat_directory_item);
    int total_sectors = root_dir_size / disk->sector_size;

    if (root_dir_size % disk->sector_size) {
        total_sectors++;
    }

    int total_items =
        fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    struct fat_directory_item* dir = kzalloc(root_dir_size);

    if (!dir) {
        res = -ENOMEM;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    struct disk_stream* stream = fat_private->directory_stream;
    if (diskstream_seek(stream, fat16_sector_to_absolute(
                                    disk, root_dir_sector_pos)) != AOK) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    if (diskstream_read(stream, dir, root_dir_size) != AOK) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    directory->item = dir;
    directory->total = total_items;
    directory->start_sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos =
        root_dir_sector_pos + (root_dir_size / disk->sector_size);

out:
    return res;
}

int fat16_resolve(struct disk* disk) {
    int res = 0;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    struct disk_stream* stream = diskstream_new(disk->id);
    if (!stream) {
        res = -ENOMEM;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    if (diskstream_read(stream, &fat_private->header,
                        sizeof(fat_private->header)) != AOK) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    if (fat_private->header.shared.extended_header.signature != 0x29) {
        res = -EFSNOTUS;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    if (fat16_get_root_directory(disk, fat_private,
                                 &fat_private->root_directory) != AOK) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

out:
    if (stream) {
        diskstream_close(stream);
    }

    if (ISERR(res)) {
        kfree(fat_private);
        disk->fs_private = 0;
    }

    return res;
}

void fat16_to_proper_string(char** out, const char* in) {
    while (*in != 0x00 && *in != 0x20) {
        *(*out)++ = *in++;
    }

    if (*in == 0x20) {
        **out = 0x00;
    }
}

void fat16_get_full_relative_filename(struct fat_directory_item* item,
                                      char* out, int max_len) {
    memset(out, 0x00, max_len);
    char* out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*)item->filename);
    if (item->extension[0] != 0x00 && item->extension[0] != 0x20) {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*)item->extension);
    }
}

struct fat_directory_item*
fat16_clone_directory_item(struct fat_directory_item* item, int size) {
    struct fat_directory_item* item_copy = 0;
    if (size < sizeof(struct fat_directory_item))
        return 0;

    item_copy = kzalloc(size);
    if (!item_copy) {
        return 0;
    }

    memcpy(item_copy, item, size);
    return item_copy;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item* item) {
    // TODO: Check if this is correct
    return (item->high_16_bits_first_cluster) | item->low_16_bits_first_cluster;
}

static int fat16_cluster_to_sector(struct fat_private* private, int cluster) {
    return private->root_directory.ending_sector_pos +
           ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct fat_private* private) {
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct disk* disk, int cluster) {
    int res = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if (!stream)
        goto out;

    uint32_t fat_table_position =
        fat16_get_first_fat_sector(private) * disk->sector_size;

    res = diskstream_seek(stream, fat_table_position +
                                      (cluster * PEACHOS_FAT16_FAT_ENTRY_SIZE));

    if (ISERR(res))
        goto out;

    uint16_t result = 0;
    res = diskstream_read(stream, &result, sizeof(result));
    if (ISERR(res))
        goto out;

    res = result;

out:
    return res;
}

static int fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster,
                                        int offset) {
    int res = 0;

    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes =
        private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++) {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xff8 || entry == 0xfff) {
            res = -EIO;
            vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)),
                      VGA_RED);
            goto out;
        }

        // Check for bad sectors
        if (entry == PEACHOS_FAT16_BAD_SECTOR) {
            res = -EIO;
            vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)),
                      VGA_RED);
            goto out;
        }

        // Check for reserved sectors
        if (entry == 0xFF0 || entry == 0xFF6) {
            res = -EIO;
            vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)),
                      VGA_RED);
            goto out;
        }

        // Check for corruption
        if (entry == 0x00) {
            res = -EIO;
            vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)),
                      VGA_RED);
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;

out:
    return res;
}

static int fat16_read_internal_from_stream(struct disk* disk,
                                           struct disk_stream* stream,
                                           int cluster, int offset, int total,
                                           void* out) {
    int res = 0;

    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes =
        private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (ISERR(cluster_to_use)) {
        res = cluster_to_use;
        goto out;
    }

    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos =
        (starting_sector * disk->sector_size) * offset_from_cluster;
    int total_to_read =
        total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstream_seek(stream, starting_pos);
    if (ISNOTOK(res))
        goto out;

    res = diskstream_read(stream, out, total_to_read);
    if (ISNOTOK(res))
        goto out;

    total -= total_to_read;
    if (total > 0) {
        // more to read
        // TODO: Do this without rescursion to avoid stack overflow
        res = fat16_read_internal_from_stream(disk, stream, cluster,
                                              offset + total_to_read, total,
                                              out + total_to_read);
    }

out:
    return res;
}

static int fat16_read_internal(struct disk* disk, int starting_cluster,
                               int offset, int total, void* out) {
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster,
                                           offset, total, out);
}

void fat16_free_directory(struct fat_directory* directory) {
    if (!directory)
        return;

    if (directory->item)
        kfree(directory->item);

    kfree(directory);
}

void fat16_fat_item_free(struct fat_item* item) {
    if (item->type == FAT_ITEM_TYPE_DIRECTORY) {
        fat16_free_directory(item->directory);
    } else if (item->type == FAT_ITEM_TYPE_FILE) {
        kfree(item->item);
    } else {
        // TODO: Panic
    }

    kfree(item);
}

struct fat_directory*
fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item) {
    int res = 0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY)) {
        res = -EINVARG;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory) {
        res = -ENOMEM;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->item = kzalloc(directory_size);
    if (!directory->item) {
        res = -ENOMEM;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    res = fat16_read_internal(disk, cluster, 0x00, directory_size,
                              directory->item);
    if (ISNOTOK(res))
        goto out;
out:
    if (ISNOTOK(res)) {
        fat16_free_directory(directory);
        return 0;
    }
    return directory;
}

struct fat_item*
fat16_new_fat_item_for_directory_item(struct disk* disk,
                                      struct fat_directory_item* item) {
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item) {
        return 0;
    }

    if (item->attribute & FAT_FILE_SUBDIRECTORY) {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }

    f_item->type = FAT_ITEM_TYPE_FILE;
    f_item->item =
        fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
    return f_item;
}

struct fat_item* fat16_find_item_in_directory(struct disk* disk,
                                              struct fat_directory* directory,
                                              const char* name) {
    struct fat_item* f_item = 0;
    char temp_filename[PEACHOS_MAX_PATH];
    for (int i = 0; i < directory->total; i++) {
        fat16_get_full_relative_filename(&directory->item[i], temp_filename,
                                         sizeof(temp_filename));
        if (istrncmp(temp_filename, name, sizeof(temp_filename)) == 0) {
            // Found file, create new fat_item
            f_item = fat16_new_fat_item_for_directory_item(disk,
                                                           &directory->item[i]);
        }
    }

    return f_item;
}

struct fat_item* fat16_get_directory_entry(struct disk* disk,
                                           struct path_part* path) {
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item = fat16_find_item_in_directory(
        disk, &fat_private->root_directory, path->part);

    if (!root_item) {
        // No file exists
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;
    while (next_part != 0) {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
            current_item = 0;
            break;
        }

        struct fat_item* temp_item = fat16_find_item_in_directory(
            disk, current_item->directory, next_part->part);

        fat16_fat_item_free(current_item);
        current_item = temp_item;
        next_part = next_part->next;
    }

out:
    return current_item;
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode) {
    if (mode != FILE_MODE_READ) {
        return ERROR(-ERDONLY);
    }

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));

    if (!descriptor)
        return ERROR(-ENOMEM);

    descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item) {
        // TODO: Fix (this is a memory leak)
        // kfree(descriptor);
        return ERROR(-EIO);
    }

    descriptor->pos = 0;

    return descriptor;
}