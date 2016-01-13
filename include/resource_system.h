#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include "common.h"
#include "util.h"
#include <memory>
#include <map>

struct ResourceHandle {
	ResourceHandle() : data_handle(nullptr, ArrayDeleter()), _size(0) {}
	ResourceHandle(uint8_t* ptr, size_t sz)
	: data_handle(ptr, ArrayDeleter())
	, _size(sz) {}
	
	size_t size() const { return _size; }
	uint8_t* data() const { return data_handle.get(); }
	operator bool() const { return data_handle.get() != nullptr; }
	std::shared_ptr<uint8_t>* operator->(){ return &data_handle; }
private:
	std::shared_ptr<uint8_t> data_handle;
	size_t _size;
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
	
	template<class T, class... Args>
	struct Cache {
	
		struct Entry {
			T* ptr;
			int ref_count;
		};

		bool get(const char* name, T*& ptr, const std::tuple<Args...>& args){
			DEBUGF("Checking resource cache for %s... ", name);
			auto it = entries.find(std::tuple_cat(std::make_tuple(str_hash(name)), args));
		
			if(it != entries.end()){
				DEBUGF("%s", "[Found!]");
				ptr = it->second.ptr;
				it->second.ref_count++;
				return true;
			} else {
				DEBUGF("%s", "[Not Found]");
				return false;
			}
		}
		
		void put(const char* name, T* ptr, const std::tuple<Args...>& args){
			auto tup = std::tuple_cat(std::make_tuple(str_hash(name)), args);
			assert(entries.find(tup) == entries.end());
			
			TRACEF("Adding %s to resource cache.", name);
			entries[tup] = Entry{ ptr, 1 };
		}

		void del(const char* name, const std::tuple<Args...>& args){
			auto tup = std::tuple_cat(std::make_tuple(str_hash(name)), args);
			auto it = entries.find(tup);
			assert(it != entries.end());
			
			TRACEF("Decreasing %s reference count.", name);
			if(--it->second.ref_count == 0){
				TRACEF("reference count == 0, removing from cache.");
				delete it->second.ptr;
				entries.erase(it);
			}
		}
		
		typedef std::tuple<uint32_t, Args...> CacheTuple;
		std::map<CacheTuple, Entry> entries;
	};
	
	template<class T, class... Args>
	Cache<T, Args...>& cache(){
		static Cache<T, Args...> cache;
		return cache;
	}

private:
	ResourceHandle import(const char* name);
	std::map<strhash_t, ResourceHandle> resources;
};

#endif
