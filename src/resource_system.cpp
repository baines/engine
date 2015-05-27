#include "resource_system.h"
#include <physfs.h>

using namespace std;

#ifndef __EMSCRIPTEN__
	#ifdef _WIN32
		#define INTERNAL_ZIP(x) binary_internal_zip_ ## x
	#else
		#define INTERNAL_ZIP(x) _binary_internal_zip_ ## x
	#endif
	
	extern "C" char INTERNAL_ZIP(start)[], INTERNAL_ZIP(end)[];
#endif

ResourceSystem::ResourceSystem(const char* argv0){
	if(!PHYSFS_init(argv0)){
		log(logging::error, "PhysFS init Error: %s", PHYSFS_getLastError());
	}
#ifdef __EMSCRIPTEN__
	if(!PHYSFS_mount("internal.zip", NULL, 0)){
#else
	const size_t sz = INTERNAL_ZIP(end) - INTERNAL_ZIP(start);
	if(!PHYSFS_mountMemory(INTERNAL_ZIP(start), sz, NULL, ".zip", NULL, 0)){
#endif
		log(logging::error, "Error mounting internal zip: %s", PHYSFS_getLastError());
	}

	char path[PATH_MAX] = {};
	snprintf(path, sizeof(path), "%s%sdata/", PHYSFS_getBaseDir(), PHYSFS_getDirSeparator());
	if(!PHYSFS_mount(path, NULL, 0)){
		log(logging::info, "Couldn't mount ./data/: %s", PHYSFS_getLastError());
	}
}

ResourceHandle ResourceSystem::load(const char* name){
	auto it = resources.find(str_hash(name));
	if(it != resources.end()){
		DEBUGF("Resource '%s' loaded from map.", name);
		return it->second;
	} else {
		DEBUGF("Resource '%s' not in map, loading via PHYSFS.", name);
		ResourceHandle h = import(name);
		resources.emplace(str_hash(name), h);
		return h;
	}
}

size_t ResourceSystem::getUseCount(const char* name){
	auto it = resources.find(str_hash(name));
	
	if(it != resources.end()){
		return it->second->use_count();
	} else {
		return 0;
	}
}

ResourceHandle ResourceSystem::import(const char* name){
	uint8_t* file_data = nullptr;
	size_t file_size = 0;

	if(PHYSFS_File* f = PHYSFS_openRead(name)){
		file_size = PHYSFS_fileLength(f);
		if(file_size > 0){
			// add an extra null byte in case c string functions are used.
			file_data = new uint8_t[file_size+1]();
			PHYSFS_readBytes(f, file_data, file_size);
		}
		PHYSFS_close(f);
	}

	return ResourceHandle(file_data, file_size);
}

