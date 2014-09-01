#include "resource_system.h"

using namespace std;

shared_ptr<Buffer> ResourceSystem::load(const char* name){
	auto it = resources.find(name);
	if(it != resources.end()){
		weak_ptr<Buffer>& wp = it->second;
		
		if(auto sp = wp.lock()){
			return sp;
		} else {
			return loadFromFile(name, wp);
		}
	} else {
		auto pair = resources.emplace(string(name), weak_ptr<Buffer>());
		
		return loadFromFile(name, pair.first->second);
	}
}

size_t ResourceSystem::getUseCount(const char* name){
	auto it = resources.find(name);
	
	if(it != resources.end()){
		return it->second.use_count();
	} else {
		return 0;
	}
}

shared_ptr<Buffer> ResourceSystem::loadFromFile(const char* name, weak_ptr<Buffer>& ptr){
	size_t file_size; //XXX
	uint8_t* file_data; //XXX
	
	// invoke specific ResourceLoader...
	
	auto sp = make_shared<Buffer>(file_size, file_data);
	ptr = sp;
	
	return sp;
}

