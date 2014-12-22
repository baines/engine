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
				cvars.back().eval(i->second);
			}
			cvar_hooks.erase(it_pair.first, it_pair.second);
			
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
	bool evalVar(uint32_t hash, const string_view& str, bool hook = false){
		auto it = std::find_if(cvars.begin(), cvars.end(), [&](const CVar& cv){
			return cv.name.hash == hash;
		});
		
		bool ret_val = false;
		if(it != cvars.end()){
			it->eval(str);
			ret_val = true; //XXX: return if setting the var to str succeeded or not instead?
		} else if(hook){
			cvar_hooks.emplace(hash, str);
		}

		return ret_val;
	}

	bool evalVar(const str_const& key, const string_view& value, bool hook = false){
		return evalVar(key.hash, value, hook);
	}
	
	bool extendPrefix(std::string& prefix, size_t offset = 0){
		return cvar_trie.prefixExtend(prefix, offset);
	}
	
	void getVarsWithPrefix(const char* prefix, std::vector<CVar*>& output){
		cvar_trie.prefixSearch(prefix, output);
	}
private:
	std::list<CVar> cvars;
	Trie<CVar*> cvar_trie;
	
	ResourceHandle cfg_file;
	std::multimap<uint32_t, string_view> cvar_hooks;
};

#endif
