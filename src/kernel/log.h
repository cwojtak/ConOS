#ifndef LOG_H
#define LOG_H

#include "../drivers/screen.h"
#include "../drivers/com.h"

void log(int level, char* message);
void logSilent(int level, char* message);

#endif
