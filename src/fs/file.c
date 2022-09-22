#include "fs/file.h"
#include "config.h"
#include "disk/disk.h"
#include "fs/fat/fat16.h"
#include "fs/pparser.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "string/string.h"
#include "vga_writer.h"

struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

static struct filesystem** fs_get_free_filesystem() {
    int i = 0;
    for (i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (!filesystems[i])
            return &filesystems[i];
    }
    return 0;
}

void fs_insert_filesystem(struct filesystem* filesystem) {
    struct filesystem** fs;
    fs = fs_get_free_filesystem();
    if (!fs) {
        vga_print("Problem inserting filesystem", VGA_RED);
        while (1)
            ;
        // Panic
    }
    *fs = filesystem;
}

static void fs_static_load() {
    // Insert fat16 filesystem
    fs_insert_filesystem(fat16_init());
}

void fs_load() {
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static int file_new_descriptor(struct file_descriptor** desc_out) {
    int res = -ENOMEM;
    for (int i = 0; i < PEACHOS_MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptors[i] == 0) {
            struct file_descriptor* desc =
                kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }
    return res;
}

static struct file_descriptor* file_get_descriptor(int fd_id) {
    if (fd_id <= 0 || fd_id >= PEACHOS_MAX_FILE_DESCRIPTORS) {
        return 0;
    }

    // Descriptors start at 1
    int index = fd_id - 1;

    return file_descriptors[index];
}

struct filesystem* fs_resolve(struct disk* disk) {
    struct filesystem* fs = 0;
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            fs = filesystems[i];
            break;
        }
    }
    return fs;
}

FILE_MODE file_get_mode_by_string(const char* str) {
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0) {
        mode = FILE_MODE_READ;
    } else if (strncmp(str, "w", 1) == 0) {
        mode = FILE_MODE_WRITE;
    } else if (strncmp(str, "a", 1) == 0) {
        mode = FILE_MODE_APPEND;
    }
    return mode;
}

int fopen(const char* filename, const char* mode_str) {
    int res = 0;

    struct path_root* root_path = pathparser_parse(filename, NULL);
    if (!root_path) {
        res = -EINVARG;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    // Cannot just have a root path
    if (!root_path->first) {
        res = -EINVARG;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    // Ensure the disk exists
    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    // Ensure the disk has a filesystem
    if (!disk->filesystem) {
        res = -EIO;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID) {
        res = -EINVARG;
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    void* descriptor_private_data =
        disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data)) {
        res = ERROR_I(descriptor_private_data);
        vga_print(strcat("Error: Line ", to_numeric_string(__LINE__)), VGA_RED);
        goto out;
    }

    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (ISERR(res))
        goto out;
    desc->filesystem = disk->filesystem;
    desc->private_data = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    // fopen shouldn't return negative values
    if (ISERR(res))
        res = 0;
    return res;
}