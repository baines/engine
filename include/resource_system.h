#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include <memory>
#include <string>
#include <map>
#include "util.h"

struct Buffer {
	Buffer(size_t s, uint8_t* d) : size(s), data(d){}
	const size_t size;
	const uint8_t* const data;
};

struct ResourceSystem {

	std::shared_ptr<Buffer> load(const char* name);
	size_t getUseCount(const char* name);
	
	template<class T>
	struct Cache {
		T* get(const char* name){
			auto it = entries.find(name);
		
			if(it != entries.end()){
				return &it->second;
			} else {
				return nullptr;
			}
		}
		
		void add(const char* name, const T& t){
			entries.emplace(name, t);
		}
		
		static std::map<std::string, T> entries;
	};
	
	template<class T>
	Cache<T> cache(void){
		return Cache<T>();
	}

private:
	std::shared_ptr<Buffer> loadFromFile(const char* name, std::weak_ptr<Buffer>& ptr);
	std::map<std::string, std::weak_ptr<Buffer>> resources;
};

#endif
