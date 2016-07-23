#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include "common.h"
#include "util.h"
#include <memory>
#include <map>

//extern template class std::shared_ptr<uint8_t>;
//extern template class std::map<strhash_t, ResourceHandle>;

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

	template<size_t N>
	void addImmediate(const str_const& name, const char (&data)[N]){
		resources.emplace(name.hash, make_resource(data));
	}
	
private:
	ResourceHandle import(const char* name);
	std::map<strhash_t, ResourceHandle> resources;
};

#endif
