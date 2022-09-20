#ifndef PPARSER_H
#define PPARSER_H

struct path_root {
    int drive_no;
    struct path_part* first;
};

struct path_part {
    const char* part;
    struct path_part* next;
};

struct path_root* pathparser_parse(const char* path,
                                   const char* current_dir_path);
void pathparser_free(struct path_root* root);

#endif // PPARSER_H
