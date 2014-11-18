#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include <memory>
#include <string>
#include <map>
#include "common.h"
#include "util.h"

struct ResourceHandle {
	ResourceHandle()
	: data_handle(nullptr, ArrayDeleter())
	, _size(0) {
	
	}
	
	ResourceHandle(uint8_t* ptr, size_t sz)
	: data_handle(ptr, ArrayDeleter())
	, _size(sz) {
	
	}
	
	size_t size() const {
		return _size;
	}
	
	uint8_t* data() const {
		return data_handle.get();
	}
	
	operator bool() const {
		return data_handle.get() != nullptr;
	}

	std::shared_ptr<uint8_t>* operator->(){ return &data_handle; }
private:
	std::shared_ptr<uint8_t> data_handle;
	size_t _size;
};

template<class T>
ResourceHandle make_resource(const T& data){
	uint8_t* new_data = new uint8_t[sizeof(data)];
	memcpy(new_data, data, sizeof(data));
	return ResourceHandle(new_data, sizeof(data));
}

struct ResourceSystem {

	ResourceHandle load(const char* name);
	size_t getUseCount(const char* name);
	
	template<size_t N>
	void addImmediate(const char* name, const char (&data)[N]){
		resources.emplace(name, make_resource(data));
	}
	
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
	ResourceHandle import(const char* name);
	std::map<std::string, ResourceHandle> resources;
};

template<class T> std::map<std::string, T> ResourceSystem::Cache<T>::entries{};

#endif
