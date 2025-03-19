#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include "StandardIncludes.h"

class SoundEffect;
class Song;

class AudioController : public Singleton<AudioController>
{
public:
	// Contructors/Destructors
	AudioController();
	virtual ~AudioController();

	// Accessors
	string GetCurrentEffect() { return m_currentEffect; }
	string GetMusicTitle() { return m_musicTitle; }
	string GetMusicLength() { return m_musicLength; }

	// Methods
	SoundEffect* LoadEffect(string _guid);
	Song* LoadSong(string _guid);
	void Play(SoundEffect* _effect);
	void Play(Song* _song);
	void StopMusic();
	void PauseMusic();
	void ResumeMusic();
	double MusicPosition();
	void Shutdown();

	static void CatchChannelDone(int _channel);

private:
	// Methods
	Mix_Chunk* GetSDLSFX(SoundEffect* _effect);
	Mix_Music* GetSDLMusic(Song* _song);

	// Members
	map<string, Mix_Chunk*> m_effects;
	map<string, Mix_Music*> m_songs;
	string m_currentEffect;
	string m_musicLength;
	string m_musicTitle;
	Mix_Music* m_currentSong;
};

#endif // AUDIOCONTROLLER_H
