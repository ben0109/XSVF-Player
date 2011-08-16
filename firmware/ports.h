/*******************************************************/
/* file: ports.h */
/* abstract: This file contains extern declarations */
/* for providing stimulus to the JTAG ports.*/
/*******************************************************/
#ifndef ports_dot_h
#define ports_dot_h

#define TMS 0x10
#define TDO 0x20
#define TDI 0x40
#define TCK 0x80

void ports_init(void);

#include <stdio.h>
extern FILE* _stdout;

#define LOG_BEGIN(lvl)		fprintf(_stdout, lvl)
#define LOG_PRINTF(format,...)	fprintf(_stdout, format, ## __VA_ARGS__)
#define LOG_END()		fprintf(_stdout, "\n")

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

/* set the port "p" (TCK, TMS, or TDI) to val (0 or 1) */
extern void setPort(short p, short val);

/* read the TDO bit */
extern unsigned char readTDOBit(void);

/* make clock go down->up->down*/
extern void pulseClock(void);

extern void waitTime(long microsec);

#include <inttypes.h>
extern int readByte(uint8_t*);
extern int readWord(uint16_t*);
extern int readLong(uint32_t*);
#endif

