#ifndef LOGGER_H
#define LOGGER_H

void logger_printf(const char *fmt, ...);
void logger_get_logs(char *buffer, size_t bufsize);

#endif
