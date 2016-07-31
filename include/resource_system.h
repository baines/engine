#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include "common.h"
#include "util.h"
#include <unordered_map>

struct ResourceHandle {
	uint8_t* data;
	size_t size;
	ResourceSystem* res;
	operator bool(){ return data; }
	~ResourceHandle(){}
};

struct ResourceSystem {
	ResourceSystem(const char* argv0);
	ResourceHandle load(const char* name);
	size_t getUseCount(const char* name);
	~ResourceSystem();
private:
	ResourceHandle import(const char* name);
	std::unordered_map<strhash_t, ResourceHandle> resources;
};

#endif
