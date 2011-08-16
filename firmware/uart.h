/********************************************************/
/* file: uart.h						*/
/********************************************************/

#ifndef uart_dot_h
#define uart_dot_h

#include <stdio.h>
#include <inttypes.h>

extern FILE* uart;

void uart_init(void);
extern int read_byte(uint8_t*);
extern int read_word(uint16_t*);
extern int read_long(uint32_t*);

extern int read_bytes(uint8_t*,int);

#define LOG_BEGIN(lvl)		fprintf(uart, lvl)
#define LOG_PRINTF(format,...)	fprintf(uart, format, ## __VA_ARGS__)
#define LOG_END()		fprintf(uart, "\n")

#ifndef LOG_LEVEL
#define LOG_LEVEL	1
#endif

#if LOG_LEVEL>3
#define LOG_DEBUG_BEGIN()		LOG_BEGIN("d")
#define LOG_DEBUG_PRINTF(format,...)	LOG_PRINTF(format, ## __VA_ARGS__)
#define LOG_DEBUG_END()			LOG_END()
#else
#define LOG_DEBUG_BEGIN()
#define LOG_DEBUG_PRINTF(format,...)
#define LOG_DEBUG_END()
#endif

#if LOG_LEVEL>2
#define LOG_INFO_BEGIN()		LOG_BEGIN("i")
#define LOG_INFO_PRINTF(format,...)	LOG_PRINTF(format, ## __VA_ARGS__)
#define LOG_INFO_END()			LOG_END()
#else
#define LOG_INFO_BEGIN()
#define LOG_INFO_PRINTF(format,...)
#define LOG_INFO_END()
#endif

#if LOG_LEVEL>1
#define LOG_WARNING_BEGIN()		LOG_BEGIN("w")
#define LOG_WARNING_PRINTF(format,...)	LOG_PRINTF(format, ## __VA_ARGS__)
#define LOG_WARNING_END()		LOG_END()
#else
#define LOG_WARNING_BEGIN()
#define LOG_WARNING_PRINTF(format,...)
#define LOG_WARNING_END()
#endif

#if LOG_LEVEL>0
#define LOG_ERROR_BEGIN()		LOG_BEGIN("e")
#define LOG_ERROR_PRINTF(format,...)	LOG_PRINTF(format, ## __VA_ARGS__)
#define LOG_ERROR_END()			LOG_END()
#else
#define LOG_ERROR_BEGIN()
#define LOG_ERROR_PRINTF(format,...)
#define LOG_ERROR_END()
#endif

#define LOG_DEBUG(format,...)		LOG_DEBUG_BEGIN(); LOG_DEBUG_PRINTF(format, ## __VA_ARGS__); LOG_DEBUG_END()
#define LOG_INFO(format,...)		LOG_INFO_BEGIN(); LOG_INFO_PRINTF(format, ## __VA_ARGS__); LOG_INFO_END()
#define LOG_WARNING(format,...)		LOG_WARNING_BEGIN(); LOG_WARNING_PRINTF(format, ## __VA_ARGS__); LOG_WARNING_END()
#define LOG_ERROR(format,...)		LOG_ERROR_BEGIN(); LOG_ERROR_PRINTF(format, ## __VA_ARGS__); LOG_ERROR_END()

extern void fail(void);
extern void success(void);

#endif

