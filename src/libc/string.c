#include "string.h"

/**
 * K&R implementation
 */
void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}

void hex_to_ascii(int n, char str[]) {
    append(str, '0');
    append(str, 'x');
    char zeros = 0;

    int32_t tmp;
    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && zeros == 0) continue;
        zeros = 1;
        if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
        else append(str, tmp + '0');
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
    else append(str, tmp + '0');
}

//K&R - https://stackoverflow.com/questions/54225045/string-of-digits-into-its-numeric-equivalent-example-not-working-properly-as-p
int atoi(const char *s) {
    int n = 0, d;

    /* skip optional initial white space */
    while (*s == 32)
        s++;
    if (*s == '-') {
        /* convert negative number */
        s++;
        while (*s >= 48 && *s <= 57) {
            d = (*s++ - '0');
            /* check for potential arithmetic overflow */
            if (n < INT_MIN / 10 || (n == INT_MIN / 10 && -d < INT_MIN % 10)) {
                n = INT_MIN;
                break;
            }
            n = n * 10 - d;
        }
    } else {
        /* ignore optional positive sign */
        if (*s == '+')
            s++;
        while (*s >= 48 && *s <= 57) {
            d = (*s++ - '0');
            /* check for potential arithmetic overflow */
            if (n > INT_MAX / 10 || (n == INT_MAX / 10 && d > INT_MAX % 10)) {
                n = INT_MAX;
                break;
            }
            n = n * 10 + d;
        }
    }
    return n;
}

/* K&R */
void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* K&R */
int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void append(char s[], char n) {
    int len = strlen(s);
    s[len] = n;
    s[len+1] = '\0';
}

void appendstr(char s[], char n[]) {
    int len = strlen(s);
    int len2 = strlen(n);
    for(int i = len; i < len2 + len; i++) {
        s[i] = n[i - len];
    }
    s[len2 + len] = '\0';
}

void strcat(char s1[], char s2[], char s3[]) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);

    for(int i = 0; i < len1 + len2 + 2; i++) {
        if(i < len1) {
            s3[i] = s1[i];
        }
        else {
            s3[i] = s2[i - len1];
        }
    }
}

void backspace(char s[]) {
    int len = strlen(s);
    s[len-1] = '\0';
}

/* K&R 
 * Returns <0 if s1<s2, 0 if s1==s2, >0 if s1>s2 */
int strcmp(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}
