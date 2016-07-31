#ifndef CONFIG_H_
#define CONFIG_H_
#include "common.h"
#include "cvar.h"
#include "resource_system.h"
#include "trie.h"
#include <vector>
#include <unordered_map>

struct Config {

	Config(Engine& e, int argc, char** argv);

	template<class Var, class... Args>
	Var* addVar(const str_const& name, Args&&... args){
		if(CVar* v = cvar_trie.find(name.str)){
			return v->template get<Var>();
		} else {
			return addVar(new Var(name, args...));
		}
	}

	template<class Var>	Var* addVar(Var* v);

	template<class Var>	Var* getVar(strhash_t hash);
	template<class Var> Var* getVar(const StrRef& name);

	// sets value to val or calls function with val for CVarFuncs
	bool evalVar(uint32_t hash, const StrRef& args, bool hook = false){
		CVar* cvar = nullptr;
		bool ret_val = false;
		
		for(auto& up : cvars){
			if(up->name.hash == hash){
				cvar = up.get();
			}
		}

		if(cvar){
			cvar->eval(args);
			ret_val = true; //XXX: return if setting the var to str succeeded or not instead?
		} else if(hook){
			cvar_hooks.emplace(hash, args);
		}

		return ret_val;
	}

	bool evalVar(const str_const& key, const StrRef& value, bool hook = false){
		return evalVar(key.hash, value, hook);
	}
	
	bool extendPrefix(StrMut& prefix, size_t offset = 0){
		return cvar_trie.prefixExtend(prefix, offset);
	}
	
	void getVarsWithPrefix(const char* prefix, std::vector<CVar*>& output){
		cvar_trie.prefixSearch(prefix, output);
	}
private:
	std::vector<UniquePtr<CVar>> cvars;
	Trie<CVar*> cvar_trie;
	
	ResourceHandle cfg_file;
	std::unordered_multimap<strhash_t, StrRef> cvar_hooks;
};

#endif
