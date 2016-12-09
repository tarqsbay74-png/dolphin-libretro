// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.


#include "AudioCommon/AudioCommon.h"
#include "AudioCommon/Mixer.h"
#include "AudioCommon/NullSoundStream.h"

#include "Common/Common.h"
#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Core/ConfigManager.h"
#include "Core/Movie.h"

// This shouldn't be a global, at least not here.
std::unique_ptr<SoundStream> g_sound_stream;

static bool s_audio_dump_start = false;

namespace AudioCommon
{
	static const int AUDIO_VOLUME_MIN = 0;
	static const int AUDIO_VOLUME_MAX = 100;

   void InitSoundStream()
   {
      std::string backend = SConfig::GetInstance().sBackend;
      g_sound_stream = std::make_unique<NullSound>();

      if (!g_sound_stream && NullSound::isValid())
      {
         WARN_LOG(AUDIO, "Could not initialize backend %s, using %s instead.", backend.c_str(),
               BACKEND_NULLSOUND);
         g_sound_stream = std::make_unique<NullSound>();
      }

      UpdateSoundStream();

      if (!g_sound_stream->Start())
      {
         ERROR_LOG(AUDIO, "Could not start backend %s, using %s instead", backend.c_str(),
               BACKEND_NULLSOUND);

         g_sound_stream = std::make_unique<NullSound>();
         g_sound_stream->Start();
      }

      if (SConfig::GetInstance().m_DumpAudio && !s_audio_dump_start)
         StartAudioDump();
   }

	void ShutdownSoundStream()
	{
		INFO_LOG(DSPHLE, "Shutting down sound stream");

		if (g_sound_stream)
		{
			g_sound_stream->Stop();
			if (SConfig::GetInstance().m_DumpAudio && s_audio_dump_start)
				StopAudioDump();

         g_sound_stream.reset();
		}

		INFO_LOG(DSPHLE, "Done shutting down sound stream");
	}

	std::vector<std::string> GetSoundBackends()
	{
		std::vector<std::string> backends;

      backends.push_back(BACKEND_NULLSOUND);
		return backends;
	}

	void UpdateSoundStream()
	{
		if (g_sound_stream)
		{
			int volume = SConfig::GetInstance().m_IsMuted ? 0 : SConfig::GetInstance().m_Volume;
			g_sound_stream->SetVolume(volume);
		}
	}

	void ClearAudioBuffer(bool mute)
	{
		if (g_sound_stream)
			g_sound_stream->Clear(mute);
	}

	void SendAIBuffer(short const *samples, unsigned int num_samples)
	{
		if (!g_sound_stream)
			return;

		if (SConfig::GetInstance().m_DumpAudio && !s_audio_dump_start)
			StartAudioDump();
		else if (!SConfig::GetInstance().m_DumpAudio && s_audio_dump_start)
			StopAudioDump();

		CMixer* pMixer = g_sound_stream->GetMixer();

		if (pMixer && samples)
		{
			pMixer->PushSamples(samples, num_samples);
		}

		g_sound_stream->Update();
	}

	void StartAudioDump()
	{
		std::string audio_file_name_dtk = File::GetUserPath(D_DUMPAUDIO_IDX) + "dtkdump.wav";
		std::string audio_file_name_dsp = File::GetUserPath(D_DUMPAUDIO_IDX) + "dspdump.wav";
		File::CreateFullPath(audio_file_name_dtk);
		File::CreateFullPath(audio_file_name_dsp);
		g_sound_stream->GetMixer()->StartLogDTKAudio(audio_file_name_dtk);
		g_sound_stream->GetMixer()->StartLogDSPAudio(audio_file_name_dsp);
		s_audio_dump_start = true;
	}

	void StopAudioDump()
	{
		g_sound_stream->GetMixer()->StopLogDTKAudio();
		g_sound_stream->GetMixer()->StopLogDSPAudio();
		s_audio_dump_start = false;
	}

	void IncreaseVolume(unsigned short offset)
	{
		SConfig::GetInstance().m_IsMuted = false;
		int& currentVolume = SConfig::GetInstance().m_Volume;
		currentVolume += offset;
		if (currentVolume > AUDIO_VOLUME_MAX)
			currentVolume = AUDIO_VOLUME_MAX;
		UpdateSoundStream();
	}

	void DecreaseVolume(unsigned short offset)
	{
		SConfig::GetInstance().m_IsMuted = false;
		int& currentVolume = SConfig::GetInstance().m_Volume;
		currentVolume -= offset;
		if (currentVolume < AUDIO_VOLUME_MIN)
			currentVolume = AUDIO_VOLUME_MIN;
		UpdateSoundStream();
	}

	void ToggleMuteVolume()
	{
		bool& isMuted = SConfig::GetInstance().m_IsMuted;
		isMuted = !isMuted;
		UpdateSoundStream();
	}
}
