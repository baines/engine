#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include "common.h"
#include <memory>
#include <string>
#include <map>
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

	ResourceSystem(const char* argv0);

	ResourceHandle load(const char* name);

	size_t getUseCount(const char* name);
	
	template<size_t N>
	void addImmediate(const str_const& name, const char (&data)[N]){
		resources.emplace(name.hash, make_resource(data));
	}
	
	template<class T, class... Args>
	struct Cache {
	
		bool get(const char* name, std::shared_ptr<T>& ptr, const std::tuple<Args...>& args){
			DEBUGF("Checking resource cache for %s... ", name);
			auto it = entries.find(std::tuple_cat(std::make_tuple(str_hash(name)), args));
		
			if(it != entries.end() && !it->second.expired()){
				DEBUGF("\t[Found!]");
				ptr = it->second.lock();
				return true;
			} else {
				DEBUGF("\t[Not Found]");
				return false;
			}
		}
		
		void put(const char* name, const std::shared_ptr<T>& ptr, const std::tuple<Args...>& args){
			auto tup = std::tuple_cat(std::make_tuple(str_hash(name)), args);
			auto it = entries.find(tup);
			
			if(it == entries.end() || it->second.expired()){
				DEBUGF("Adding %s to resource cache.", name);
				entries[tup] = ptr;
			}
		}
		
		typedef std::tuple<uint32_t, Args...> CacheTuple;
		std::map<CacheTuple, std::weak_ptr<T>> entries;
	};
	
	template<class T, class... Args>
	Cache<T, Args...>& cache(){
		static Cache<T, Args...> cache;
		return cache;
	}

private:
	ResourceHandle import(const char* name);
	std::map<uint32_t, ResourceHandle> resources;
};

//template<class T, class... Args> std::map<std::tuple<std::string, Args...>, T> ResourceSystem::Cache<T, Args...>::entries{};

#endif
