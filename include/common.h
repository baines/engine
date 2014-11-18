#ifndef COMMON_H_
#define COMMON_H_
#include <array>
#include <cstdint>

#ifdef DEBUG
	#define DEBUGF(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#else
	#define DEBUGF(fmt, ...) do { } while(0)
#endif

#endif

