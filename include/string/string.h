#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

int strlen(const char* ptr);
int strnlen(const char* ptr, int max);
int strnlen_terminator(const char* str, int max, char terminator);

char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);

int istrncmp(const char* str1, const char* str2, int n);
int strncmp(const char* str1, const char* str2, int n);

bool is_whitespace(char c);
bool is_alpha(char c);
bool is_digit(char c);

int to_numeric_digit(char c);
char* to_numeric_string(int num);
char tolower(char c);
char toupper(char c);

char* to_hex_string(unsigned long num);

#endif // STRING_H
