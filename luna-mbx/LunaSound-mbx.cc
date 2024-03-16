#include "Luna-mbx.h"

#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_mixer.h>
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#undef USE_SPEEXDSP_RESAMPLER
#endif
#ifdef USE_SPEEXDSP_RESAMPLER
#include <speex/speex_resampler.h>
#endif


//==========================================================================
// STRUCT
//==========================================================================

// ヘッダ
struct RWFFILEHEADER
{
    char guid[4];
};

#pragma pack(2)
typedef struct PACKED twaveformatex {
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
} waveformatex,*pwaveforamtex,*lpwaveformatex;
#pragma pack()

typedef struct SoundData {
    Mix_Chunk chunk;
    int channel;
} SoundData;


class LunaSound_private : public LunaSound
{
public:
    SoundData *GetData( void ) { return (SoundData *) lpSoundBuffer; }
    void SetStopped( void ) { ((SoundData *) lpSoundBuffer)->channel = -1; bPlay = FALSE; bLoop = FALSE; }
};



static CList List;
static int AudioRate, AudioChannels;
static Uint16 AudioFormat;


void LogOut(const char *format, ...);


static void ChannelFinished(int channel)
{
    LunaSound_private *sound = (LunaSound_private *) List.Top();
    for (; sound != NULL; sound = (LunaSound_private *) sound->Next)
    {
        SoundData *data = sound->GetData();

        if (data != NULL)
        {
            if (data->channel == channel)
            {
                sound->SetStopped();
                break;
            }
        }
    }
}

LunaSound::LunaSound()
{
    List.InsertTop(this);
    lpSoundBuffer = NULL;
    bPause = FALSE;
    bLoop = FALSE;
    bPlay = FALSE;
    bLoad = FALSE;
    LogOut("○ CreateSound   / ID [0x%08X] / ", (long)this);
}

LunaSound::~LunaSound()
{
    Stop();

    if (lpSoundBuffer != NULL)
    {
        SoundData *data = (SoundData *) lpSoundBuffer;
        free(data->chunk.abuf);
        free(data);
    }

    LogOut("● ReleaseSound / ID [0x%08X]\n", (long)this);
    List.Erase(this);
}

void LunaSound::LoadWave( const char *pack, const char *data )
{
    struct RWFFILEHEADER rwh;
    uint32_t DataSize;
    FILE *f;
    const char *LogMsg;
    char DataName[32];
    waveformatex wf;
    Uint8 *DataBuffer;
    int frequency, channels;
    Uint16 format;
    SoundData *sdata;


    rwh.guid[0] = 0;
    rwh.guid[1] = 0;
    rwh.guid[2] = 0;
    rwh.guid[3] = 0;

    DataSize = 0;

    LogOut("%s in %s / ", data, pack);

    f = fopen(pack, "rb");
    if (f == NULL)
    {
        LogOut("  ERROR Can't find PackFile\n");
        return;
    }

    rwh.guid[0] = fgetc(f);
    rwh.guid[1] = fgetc(f);
    rwh.guid[2] = fgetc(f);
    rwh.guid[3] = fgetc(f);
    if ( strcmp(rwh.guid, "LSD") )
    {
        LogMsg = "  ERROR This file isn't LSD File\n";
        goto logout_and_close;
    }

    while ( 1 )
    {
        fread(DataName, 1, 32, f);
        if ( feof(f) ) break;

        if ( !strcmp(DataName, data) )
        {
            fread(&DataSize, sizeof(uint32_t), 1, f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            DataSize = SDL_Swap32(DataSize);
#endif
            break;
        }

        uint32_t dsize;
        fread(&dsize, sizeof(uint32_t), 1, f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        dsize = SDL_Swap32(dsize);
#endif
        fseek(f, dsize + sizeof(waveformatex), 1);
    }

    if ( DataSize == 0 )
    {
        LogMsg = "  ERROR Can't Find WaveData\n";
        goto logout_and_close;
    }

    fread(&wf, sizeof(waveformatex), 1, f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    wf.wFormatTag      = SDL_Swap16(wf.wFormatTag     );
    wf.nChannels       = SDL_Swap16(wf.nChannels      );
    wf.nSamplesPerSec  = SDL_Swap32(wf.nSamplesPerSec );
    wf.nAvgBytesPerSec = SDL_Swap32(wf.nAvgBytesPerSec);
    wf.nBlockAlign     = SDL_Swap16(wf.nBlockAlign    );
    wf.wBitsPerSample  = SDL_Swap16(wf.wBitsPerSample );
    wf.cbSize          = SDL_Swap16(wf.cbSize         );
#endif

    frequency = wf.nSamplesPerSec;
    format = (wf.wBitsPerSample == 16)?AUDIO_S16LSB:AUDIO_U8;
    channels = wf.nChannels;

    if ( ( AudioRate != frequency ) ||
         ( AudioFormat != format ) ||
         ( AudioChannels != channels )
       )
    {
        SDL_AudioCVT cvt;
        Uint8 *ConvertBuffer;

#ifdef USE_SPEEXDSP_RESAMPLER
        DataBuffer = NULL;

        if ((AudioRate != frequency) && ((channels == 1) || (channels == 2)))
        {
            int index, src_samples, dst_samples, err;
            SpeexResamplerState *resampler;
            spx_uint32_t in_len, out_len;

            if ( ( AudioFormat != AUDIO_S16LSB ) ||
                 ( AudioChannels != channels )
               )
            {
                // after resampling, use SDL to convert format and/or number of channels
                if ( SDL_BuildAudioCVT(&cvt, AUDIO_S16LSB, channels, AudioRate, AudioFormat, AudioChannels, AudioRate) < 0 )
                {
                    LogMsg = "  ERROR Failed SDL_BuildAudioCVT\n";
                    goto logout_and_close;
                }
            }
            else
            {
                // after resampling, no additional conversion is needed
                cvt.len_mult = 0;
            }

            resampler = speex_resampler_init(channels, frequency, AudioRate, SPEEX_RESAMPLER_QUALITY_DESKTOP, &err);
            if (err != RESAMPLER_ERR_SUCCESS)
            {
                LogMsg = "  ERROR Failed speex_resampler_init\n";
                goto logout_and_close;
            }

            ConvertBuffer = (Uint8 *) malloc(DataSize * (format == AUDIO_U8 ? 2 : 1));

            fread(ConvertBuffer, 1, DataSize, f);
            fclose(f);
            f = NULL;

            if (format == AUDIO_U8)
            {
                // first convert data from U8 to S16
                for (index = DataSize - 1; index >= 0; index--)
                {
                    ((Sint16 *)ConvertBuffer)[index] = (ConvertBuffer[index] ^ 0x80) << 8;
                }

                format = AUDIO_S16LSB;
                DataSize *= 2;
            }

            src_samples = DataSize >> channels;
            dst_samples = (src_samples * (int64_t)AudioRate) / frequency;

            DataBuffer = (Uint8 *) malloc((dst_samples << channels) * (cvt.len_mult ? cvt.len_mult : 1));

            in_len = src_samples;
            out_len = dst_samples;

            if (channels == 1)
            {
                err = speex_resampler_process_int(resampler, 0, (const spx_int16_t *)ConvertBuffer, &in_len, (spx_int16_t *)DataBuffer, &out_len);
            }
            else
            {
                err = speex_resampler_process_interleaved_int(resampler, (const spx_int16_t *)ConvertBuffer, &in_len, (spx_int16_t *)DataBuffer, &out_len);
            }

            free(ConvertBuffer);
            speex_resampler_destroy(resampler);

            if ((err != RESAMPLER_ERR_SUCCESS) || ((in_len != (spx_uint32_t)src_samples) && (out_len != (spx_uint32_t)dst_samples)))
            {
                LogMsg = "  ERROR Failed speex_resampler_process\n";
                goto logout_and_close;
            }

            frequency = AudioRate;
            DataSize = out_len << channels;

            if (cvt.len_mult != 0)
            {
                ConvertBuffer = DataBuffer;
                DataBuffer = NULL;
            }
        }
        else
#endif
        {
            if ( SDL_BuildAudioCVT(&cvt, format, channels, frequency, AudioFormat, AudioChannels, AudioRate) < 0 )
            {
                LogMsg = "  ERROR Failed SDL_BuildAudioCVT\n";
                goto logout_and_close;
            }

            ConvertBuffer = (Uint8 *) malloc(DataSize * cvt.len_mult);

            fread(ConvertBuffer, 1, DataSize, f);
            fclose(f);
            f = NULL;
        }

#ifdef USE_SPEEXDSP_RESAMPLER
        if (cvt.len_mult != 0)
#endif
        {
            cvt.buf = ConvertBuffer;
            cvt.len = DataSize;

            if ( SDL_ConvertAudio(&cvt) )
            {
                free(ConvertBuffer);
                LogMsg = "  ERROR Failed SDL_ConvertAudio\n";
                goto logout_and_close;
            }

            DataSize = cvt.len * cvt.len_ratio;
            DataBuffer = (Uint8 *) realloc(cvt.buf, DataSize);
        }
    }
    else
    {
        DataBuffer = (Uint8 *) malloc(DataSize);

        fread(DataBuffer, 1, DataSize, f);
        fclose(f);
        f = NULL;
    }

    sdata = (SoundData *) malloc(sizeof(SoundData));
    sdata->chunk.allocated = 0;
    sdata->chunk.volume = 128;

    sdata->chunk.abuf = DataBuffer;
    sdata->chunk.alen = DataSize;
    sdata->channel = -1;

    lpSoundBuffer = sdata;
    bLoad = TRUE;
    bPlay = FALSE;
    bLoop = FALSE;
    bPause = FALSE;

    LogMsg = "Complete\n";

logout_and_close:
    LogOut(LogMsg);
    if (f != NULL) fclose(f);
}

BOOL LunaSound::Init( void )
{
    LogOut("\n============================================== Initialize DirectSound\n");

    if ( !SDL_InitSubSystem(SDL_INIT_AUDIO) )
    {
        int frequency, channels, buffersize, result;
        Uint16 format;

        frequency = 44100;
        format = AUDIO_S16LSB;
        channels = 2;
        buffersize = 4096;

#if SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >= SDL_VERSIONNUM(2,0,2)
        const SDL_version *link_version = Mix_Linked_Version();
        if (SDL_VERSIONNUM(link_version->major, link_version->minor, link_version->patch) >= SDL_VERSIONNUM(2,0,2))
        {
            result = Mix_OpenAudioDevice(frequency, format, channels, buffersize, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
        }
        else
#endif
        {
            result = Mix_OpenAudio(frequency, format, channels, buffersize);
        }
        if (result == 0)
        {
            Mix_QuerySpec(&frequency, &format, &channels);
            AudioRate = frequency;
            AudioFormat = format;
            AudioChannels = channels;

            Mix_AllocateChannels(32);

            // set function to call when any channel finishes playback
            Mix_ChannelFinished(ChannelFinished);
        }
        else
        {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
    }

    List.Init();
    return TRUE;
}

void LunaSound::UnInit( void )
{
    LogOut("\n============================================== Destroy DirectSound\n");

    Mix_HaltChannel(-1);

    {
        LunaSound_private *sound = (LunaSound_private *) List.Top();
        for (; sound != NULL; sound = (LunaSound_private *) sound->Next)
        {
            SoundData *data = sound->GetData();

            if (data != NULL)
            {
                if (data->channel >= 0)
                {
                    sound->SetStopped();
                }
            }
        }
    }

    List.Release();

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

LSOUND LunaSound::Load( const char *pack, const char *data )
{
    LSOUND result = new LunaSound();

    result->LoadWave(pack, data);

    return result;
}

void LunaSound::Play( BOOL loop )
{
    if ( bLoad )
    {
        Stop();

        SoundData *data = (SoundData *) lpSoundBuffer;

        data->channel = Mix_PlayChannelTimed(-1, &(data->chunk), (loop)?-1:0, -1);

        bLoop = loop;
        bPlay = TRUE;
    }
}

void LunaSound::Stop( void )
{
    if ( bLoad && bPlay )
    {
        int channel = -1;

        if (lpSoundBuffer != NULL)
        {
            channel = ((SoundData *) lpSoundBuffer)->channel;
        }

        if (channel >= 0)
        {
            Mix_HaltChannel(channel);

            ((SoundData *) lpSoundBuffer)->channel = -1;
        }

        bPlay = FALSE;
        bLoop = FALSE;
    }
}

