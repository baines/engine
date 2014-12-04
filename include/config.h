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

struct Config {

	Config(Engine& e, int argc, char** argv);

	template<class Var>
	Var* addVar(const char* name, Var&& var){
		if(CVar* v = cvar_trie.find(name)){
			return v->template get<Var>();
		} else {
			cvars.emplace_back(name, std::forward<Var>(var));
			cvar_trie.add(name, &cvars.back());
			
			/* TODO: look through cfg_file_cmds for name w/ multimap::equal_range
					 if CvarFunc, call it with value from the map
					 else, convert map value to type for this CVar + set it.
					 	//XXX: the conversion should probably be done inside the cvar's Set member func.
			*/
			
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
	
	template<class T>
	void overrideVar(const char* name, const T& val){
		/*TODO: similar to cfg_file_cmds, store val to be set when Var is created.
				could accept a string instead of templated type and add to the
				cfg_file_cmds for simplicity, would be slightly less performant though...
		*/
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
	std::multimap<uint32_t, std::string> cfg_file_cmds;
};

#endif
