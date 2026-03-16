#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"

#define QUEUE_MAX   32
#define MESSAGE_LEN 127

typedef struct LogRecord {
    char message[MESSAGE_LEN + 1];
} LogRecord;

static LogRecord queue[QUEUE_MAX];
static int tail;

void logger_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t len = vsnprintf(queue[tail].message, MESSAGE_LEN, fmt, args);
    queue[tail].message[len] = 0;
    va_end(args);
}
