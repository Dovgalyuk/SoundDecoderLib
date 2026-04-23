#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"

#define QUEUE_MAX   64
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

    printf("%s\n", queue[tail].message);
    tail = (tail + 1) % QUEUE_MAX;
}

void logger_get_logs(char *buffer, size_t bufsize)
{
    int t = (tail - 1 + QUEUE_MAX) % QUEUE_MAX;
    char *cur = buffer;
    while (t != tail) {
        size_t len = strlen(queue[t].message);
        if (bufsize < len + 2) {
            break;
        }
        strcpy(cur, queue[t].message);
        cur += len;
        *cur++ = '\n';
        *cur = 0;
        t = (t - 1 + QUEUE_MAX) % QUEUE_MAX;
    }
}
