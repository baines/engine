#ifndef _AUDIO_H_
#define _AUDIO_H_
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <string>
#include "util.h"

template<class T>
class Audio {
protected:
	typedef std::map<std::string, T> AudioMap;
	Audio(const char* name, T (*ld)(const char*)){
		if(!loaded) init();
		if(loaded) audio = util::cache<T>(audioMap, name, ld, "./data/snd/");
	}
	static void init(){
		int err =  SDL_InitSubSystem(SDL_INIT_AUDIO);
		err |= Mix_OpenAudio(44100, AUDIO_S16, 2, 2048);
		loaded = err ? -1 : 1;
		if(err) fprintf(stderr, "Audio init failed: %s.\n", SDL_GetError());
	}
	static int loaded;
	static AudioMap audioMap;
	T audio;
};

template<class T> int Audio<T>::loaded = 0;
template<class T> typename Audio<T>::AudioMap Audio<T>::audioMap;

static Mix_Chunk* loadwav(const char* c){
	return Mix_LoadWAV(c);
}

struct Sound : public Audio<Mix_Chunk*> {
	Sound(const char* name) :  Audio<Mix_Chunk*>(name, loadwav){}
	void play(int loops = 0){
		if(this->audio) Mix_PlayChannel(-1, this->audio, loops);
	}
};

struct Music : public Audio<Mix_Music*> {
	Music(const char* name) :  Audio<Mix_Music*>(name, Mix_LoadMUS){}
	void play(int loops = -1){
		if(this->audio) Mix_PlayMusic(this->audio, loops);
	}
};
#endif
