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
		fprintf(stderr, "PhysFS init Error: %s\n", PHYSFS_getLastError());
	}
	
	const size_t sz = _internal_zip_end - _internal_zip_start;

	if(!PHYSFS_mountMemory(_internal_zip_start, sz, NULL, "internal.zip", NULL, 0)){
		fprintf(stderr, "PhysFS mount Error: %s\n", PHYSFS_getLastError());
	}
}

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
	uint8_t* file_data = nullptr;
	size_t file_size = 0;

	if(PHYSFS_File* f = PHYSFS_openRead(name)){
		file_size = PHYSFS_fileLength(f);
		if(file_size > 0){
			file_data = new uint8_t[file_size];
			size_t r = PHYSFS_readBytes(f, file_data, file_size);
		}
		PHYSFS_close(f);
	}

	return ResourceHandle(file_data, file_size);
}

