#include "resource_system.h"
#include <physfs.h>

using namespace std;

extern "C" {

	asm (
		".section .rodata\n"
		".align 4\n"
		".global _internal_zip_start\n"
		"_internal_zip_start:\n"
		".incbin \"internal.zip\"\n"
		".global _internal_zip_end\n"
		"_internal_zip_end:"
	);

	extern char _internal_zip_start[], _internal_zip_end[];

}

ResourceSystem::ResourceSystem(const char* argv0){
	if(!PHYSFS_init(argv0)){
		log(logging::error, "PhysFS init Error: %s", PHYSFS_getLastError());
	}
	
	const size_t sz = _internal_zip_end - _internal_zip_start;

	if(!PHYSFS_mountMemory(_internal_zip_start, sz, NULL, "internal.zip", NULL, 0)){
		log(logging::error, "PhysFS mount Error: %s", PHYSFS_getLastError());
	}

	char path[PATH_MAX] = {};
	snprintf(path, sizeof(path), "%s%sdata", PHYSFS_getBaseDir(), PHYSFS_getDirSeparator());
	if(!PHYSFS_mount(path, NULL, 0)){
		log(logging::error, "PhysFS mount Error: %s", PHYSFS_getLastError());
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

