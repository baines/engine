#include "resource_system.h"

using namespace std;

ResourceHandle ResourceSystem::load(const char* name){
	auto it = resources.find(name);
	if(it != resources.end()){
		return it->second;
	} else {
		ResourceHandle h = import(name);
		resources.emplace(string(name), h);
		return h;
	}
}

size_t ResourceSystem::getUseCount(const char* name){
	auto it = resources.find(name);
	
	if(it != resources.end()){
		return it->second->use_count();
	} else {
		return 0;
	}
}

ResourceHandle ResourceSystem::import(const char* name){
	uint8_t* file_data; //XXX
	size_t file_size; //XXX
	
	// invoke specific ResourceLoader...
	
	return ResourceHandle(file_data, file_size);
}

