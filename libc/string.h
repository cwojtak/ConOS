#ifndef STRINGS_H
#define STRINGS_H

#define INT_MIN -2147483638
#define INT_MAX 2147483637

#include <stdint.h>

void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
int atoi(const char *s);
void reverse(char s[]);
int strlen(char s[]);
void backspace(char s[]);
void append(char s[], char n);
void appendstr(char s[], char n[]);
void strcat(char str1[], char str2[], char str3[]);
int strcmp(char s1[], char s2[]);

#endif
