#include "log.h"

void log(int level, char* message) {
    if(level == 0) {
        kprint("DEBUG: ");
        kprint(message);
        kprint("\n");
        write_string_serial("DEBUG: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 1) {
        kprint("INFO: ");
        kprint(message);
        kprint("\n");
        write_string_serial("INFO: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 2) {
        kprint("NOTE: ");
        kprint(message);
        kprint("\n");
        write_string_serial("NOTE: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 3) {
        kprint("WARN: ");
        kprint(message);
        kprint("\n");
        write_string_serial("WARN: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 4) {
        kprint("ALERT: ");
        kprint(message);
        kprint("\n");
        write_string_serial("ALERT: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 5) {
        kprint("SEVERE: ");
        kprint(message);
        kprint("\n");
        write_string_serial("SEVERE: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 6) {
        kprint("ALARM: ");
        kprint(message);
        kprint("\n");
        write_string_serial("ALARM: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 7) {
        kprint("FATAL: ");
        kprint(message);
        kprint("\n");
        write_string_serial("FATAL: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else {
        kprint("WARN: log() called with invalid level. Message: ");
        kprint(message);
        kprint("\n");
        write_string_serial("WARN: log() called with invalid level. Message: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
}

void logSilent(int level, char* message) {
    if(level == 0) {
        write_string_serial("DEBUG: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 1) {
        write_string_serial("INFO: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 2) {
        write_string_serial("NOTE: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 3) {
        write_string_serial("WARN: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 4) {
        write_string_serial("ALERT: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 5) {
        write_string_serial("SEVERE: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 6) {
        write_string_serial("ALARM: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else if(level == 7) {
        write_string_serial("FATAL: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
    else {
        write_string_serial("WARN: log() called with invalid level. Message: ");
        write_string_serial(message);
        write_string_serial("\n");
    }
}
