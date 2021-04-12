/**
 * @file sx126x-debug.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Unified debug output for all platforms
 * 		Set LIB_DEBUG to 1 to enable debug output
 *      - either here in this header files (Arduino IDE)
 *      - or globale with build_flags = -DLIB_DEBUG=1 in platformio.ini (PIO)
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// If not on PIO or not defined in platformio.ini
#ifndef LIB_DEBUG
// Debug output set to 0 to disable app debug output
#define LIB_DEBUG 0
#endif

#if LIB_DEBUG > 0
#define LOG_LIB(tag, ...)                \
	do                                   \
	{                                    \
		if (tag)                         \
			Serial.printf("<%s> ", tag); \
		Serial.printf(__VA_ARGS__);      \
		Serial.printf("\n");             \
	} while (0)
#else
#define LOG_LIB(...)
#endif
