#ifndef STATUS_H
#define STATUS_H

#define PEACHOS_ALL_OK 0
#define AOK 0
#define EIO 1
#define EINVARG 2
#define ENOMEM 3
#define EBADPATH 4
#define EFSNOTUS 5
#define ERDONLY 6

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
#define ISNOTOK(value) ((int)value != PEACHOS_ALL_OK)

#endif // STATUS_H
