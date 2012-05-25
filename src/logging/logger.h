#ifndef DUTILS_LOGGER_H
#define DUTILS_LOGGER_H

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

void g_logger(const gchar *log_domain, GLogLevelFlags log_level,
              const gchar *format, ...);
void g_loggerv(const gchar *log_domain, GLogLevelFlags log_level,
              const gchar *format, va_list va);

void g_logger_set_globalfilter(int flags);

int g_logger_get_globalfilter();


#ifdef __cplusplus
}
#endif
#endif // DUTILS_LOGGER_H
