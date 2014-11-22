#ifndef CONFIG_H_
#define CONFIG_H_
#include "common.h"
#include "cvar.h"
#include "trie.h"
#include <vector>
#include <list>
#include <cstring>

struct Config {

	template<class Var>
	Var* addVar(const char* name, Var&& var){
		if(CVar* v = cvar_trie.find(name)){
			return v->template get<Var>();
		} else {
			cvars.emplace_back(name, std::forward<Var>(var));
			cvar_trie.add(name, &cvars.back());
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
	
	bool extendPrefix(std::string& prefix){
		return cvar_trie.prefixExtend(prefix);
	}
	
	void getVarsWithPrefix(const char* prefix, std::vector<CVar*>& output){
		cvar_trie.prefixSearch(prefix, output);
	}
	
	void load(int argc, char** argv);
private:
	std::list<CVar> cvars;
	Trie<CVar*> cvar_trie;
};

#endif
