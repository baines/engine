#ifndef CONFIG_H_
#define CONFIG_H_
#include "common.h"
#include "resource_system.h"
#include "cvar.h"
#include "trie.h"
#include <vector>
#include <list>
#include <cstring>
#include <map>
#include <algorithm>

struct Config {

	Config(Engine& e, int argc, char** argv);

	template<class Var>
	Var* addVar(const str_const& name, Var&& var){
		if(CVar* v = cvar_trie.find(name.str)){
			return v->template get<Var>();
		} else {
			cvars.emplace_back(name, std::forward<Var>(var));
			cvar_trie.add(name.str, &cvars.back());
			
			auto it_pair = cvar_hooks.equal_range(name.hash);
			for(auto& i = it_pair.first, &j = it_pair.second; i != j; ++i){
				cvars.back().eval(i->second.str, i->second.len);
			}
			return cvars.back().get<Var>();
		}
	}

	template<class Var>
	Var* getVar(const char* name){
		if(CVar* v = cvar_trie.find(name)){
			return v->template get<Var>();
		} else {
			return nullptr;
		}
	}
	
	// sets value to val or calls function with val for CVarFuncs
	//TODO: template this instead or use std::string??
	void hookVar(uint32_t hash, const char* val, size_t len = 0){
		auto it = std::find_if(cvars.begin(), cvars.end(), [&](const CVar& cv){
			return cv.name.hash == hash;
		});
		if(it != cvars.end()){
			it->eval(val, len);
		} else {
			cvar_hooks.emplace(hash, hook_str{ val, len });
		}
	}
	
	void hookVar(const str_const& str, const char* val, size_t len = 0){
		hookVar(str.hash, val, len);
	}
	
	bool extendPrefix(std::string& prefix){
		return cvar_trie.prefixExtend(prefix);
	}
	
	void getVarsWithPrefix(const char* prefix, std::vector<CVar*>& output){
		cvar_trie.prefixSearch(prefix, output);
	}
private:
	std::list<CVar> cvars;
	Trie<CVar*> cvar_trie;
	
	ResourceHandle cfg_file;
	struct hook_str { const char* str; size_t len; };
	std::multimap<uint32_t, hook_str> cvar_hooks;
};

#endif
