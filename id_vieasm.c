/*
VODKA-INDUCED ENTERTAINMENT ADVANCED SOUND MANAGER v0.9.1
BY GERARD 'ALUMIUN' WATSON
MODIFICATION By: Wolf3s


Provides high quality sound and music using SDL_MIXER AND SDL2_MIXER 2.6.1
Built on SDL_MIXER 1.2.5 and SDL2_MIXER 2.6.1
Supports all formats supported by SDL_mixer.
For more details, please see the SDL_mixer manual.

Changes
-------
v0.9.2  - Mix Everything In One Header File and Source File 
		- Port to SDL2 Mixer

v0.9.1  - Fixed the reverse stereo channel bug
        - Made ASM_ChangeVolume a lot faster and clipped the volume levels
        - Added VERBOSE defines

v0.9    - Initial release

PLEASE ALSO NOTE THAT THIS IS STILL A BETA. IT IS LIKELY TO HAVE LOTS OF BUGS.
IF YOU FIND ANY, PLEASE TELL ME SO I CAN FIX THEM!
*/

// Initialization stuff
#include "id_vieasm.h"
#ifdef VIEASM
#include "asmcref.h"// Duh.

const char* ASM_Verstring = "v0.9.1 Beta";  // Version string

Uint8 sndvol, musvol;                       // Volumes for sound
int origchannels, maxchannels, lastchan;    // Channel variables
boolean chanused[ASM_ABSMAXCHANNELS];          // Is channel used?
static Mix_Music* music = 0, * switchto = 0; // Music references
int fadetime;                               // Milliseconds to fade in\out
boolean deviceopen = false;                    // Is device open?
boolean switching = false;                     // Is switching music tracks?
boolean reversemode = false;                   // Reverse stereomode
boolean nosound;

SDMode      	SoundMode;
SMMode          MusicMode;
SDSMode         DigiMode;
static  int     LeftPosition;
static  int     RightPosition;
static  boolean    ambience;

sample ASM_Audiosegs[NUMSOUNDS];
int ambientsnds[NUMAMBIENTS];
boolean         AdLibPresent, SoundBlasterPresent, SBProPresent, SoundPositioned;
globalsoundpos channelSoundPos[ASM_ABSMAXCHANNELS];

// ASM_IsOpen
// Returns true if the device is open, false otherwise
boolean ASM_IsOpen(void)
{
    return (deviceopen) ? true : false;
}

void ASM_AdjustChannels(int reqchan)
{
    if (reqchan <= maxchannels)
    {
        int i = (reqchan > origchannels) ? reqchan : origchannels;
        Mix_AllocateChannels(i + 1);
        lastchan = i;
    }
}

// ASM_GetFreeChannel
// Returns the first free channel, calls ASM_AdjustChannels if another is needed

int ASM_GetFreeChannel(void)
{
    for (int i = 0; i <= maxchannels - 1; i++)
        if (chanused[i] == false)
        {
            chanused[i] = true;
            if (i >= lastchan)
                ASM_AdjustChannels(i);
            return i;
        }
    return -1;
}

// ASM_ChannelDone
// Callback for Mix_ChannelFinished
// Turns off channel and adjust if necessary

void ASM_ChannelDone(int channel)
{
    chanused[channel] = false;

    if (channel == (lastchan - 1) && (lastchan - 1) > origchannels - 1)
        ASM_AdjustChannels(lastchan);
}

// ASM_Open
// Opens audio device at given specs, clears used

boolean ASM_Open(int frequency, int channels, int maxchan, int buffersize, Uint8 sndvolume, Uint8 musvolume, boolean reverse)
{
    if (ASM_IsOpen())       // Device is already open!
        return false;
#ifdef VERBOSE
    Uint16 null;
    SDL_version compile_version;

    printf("----------\n"
        "Vodka-Induced Entertainment Advanced Sound Manager %s\n"
        "Developed for WolfSDL v1.6\n"
        "By Gerard 'AlumiuN' Watson\n"
        "\n"
        , ASM_Verstring);

    MIX_VERSION(&compile_version);
    printf("Compiled with SDL_mixer version: %d.%d.%d\n",
        compile_version.major, compile_version.minor, compile_version.patch);

    const SDL_version* link_version = Mix_Linked_Version();
    printf("Running with SDL_mixer version: %d.%d.%d\n"
        "\n"
        , link_version->major, link_version->minor, link_version->patch);

    printf("Opened with:\n"
        "      frequency - %d Hz\n"
        "       channels - %d\n"
        "   max channels - %d\n"
        "     buffersize - %d bytes\n"
        "   sound volume - %d\n"
        "   music volume - %d\n"
        " reverse stereo - %s\n"
        "----------\n"
        , frequency, channels, maxchan, buffersize, sndvolume, musvolume, (reverse) ? "true" : "false");
#endif
    if (Mix_OpenAudio(frequency, AUDIO_S8, 2, buffersize) == -1)
    {
#ifdef VERBOSE
        printf("ASM_Open: %s\n", Mix_GetError());
#endif
        return false;
    }

    if ((maxchan > ASM_ABSMAXCHANNELS) || (channels > maxchan))
    {
#ifdef VERBOSE
        printf("ASM_Open: Invalid maxchan value!\n");
#endif
        return false;
    }

    Mix_AllocateChannels(channels);

    memset(chanused, 0, sizeof(chanused));

    lastchan = origchannels = channels;
    maxchannels = maxchan;

    if (reverse)
        reversemode = true;

    deviceopen = true;

    ASM_ChangeVolume(sndvolume, musvolume);
    Mix_ChannelFinished(ASM_ChannelDone);
    return true;
}

// ASM_Close
// Closes the audio device

void ASM_Close(void)
{
#ifdef VERBOSE
    printf("ASM_Close: Audio device closed.\n");
#endif
    Mix_CloseAudio();
    deviceopen = false;
}

// ASM_HaltSound
// Ends all playing sounds, clears all channels, frees all chunks, etc.

void ASM_HaltSound(void)
{
    ASM_AbortIfClosed;

    Mix_HaltChannel(-1);
    memset(chanused, 0, sizeof(chanused));
    ASM_AdjustChannels(0);
}

// ASM_Pause
// Pauses all sound

void ASM_Pause(void)
{
    ASM_AbortIfClosed;

    ASM_PauseSound();
    ASM_PauseMusic();
}

// ASM_Resume
// Resumes all sound

void ASM_Resume(void)
{
    ASM_AbortIfClosed;

    ASM_ResumeSound();
    ASM_ResumeMusic();
}

// ASM_Halt
// Stops all sound, frees all chunks, etc.

void ASM_Halt(void)
{
    ASM_AbortIfClosed;

    ASM_HaltSound();
    ASM_HaltMusic();
}

// ASM_PlayMusic
// Plays a music sample from a file

boolean ASM_PlayMusic(char* musfile)
{
    ASM_AbortIfClosed false;

    if (Mix_PlayingMusic())
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
    }

    music = Mix_LoadMUS(musfile);

    if (!music)
    {
#ifdef VERBOSE
        printf("ASM_PlayMusic: %s\n", Mix_GetError());
#endif
        return false;
    }

    Mix_VolumeMusic(musvol);

    if (Mix_PlayMusic(music, -1) == -1)
    {
#ifdef VERBOSE
        printf("ASM_PlayMusic: %s\n", Mix_GetError());
#endif
        return false;
    }

    return true;
}

// ASM_ChangeVolume
// Changes the volume of:
//      Sound - iterates through each channel and sets volume accordingly
//      Music - just calls Mix_VolumeMusic

void ASM_ChangeVolume(Uint8 sndvolume, Uint8 musvolume)
{
    ASM_AbortIfClosed;

    sndvol = (sndvolume > 128) ? 128 : sndvolume;
    musvol = (musvolume > 128) ? 128 : musvolume;
    Mix_Volume(-1, sndvol);
    Mix_VolumeMusic(musvol);
}

// ASM_ReturnVolume
// Takes two byte pointers and returns the internal sound and music volumes using them

void ASM_ReturnVolume(Uint8* retsnd, Uint8* retmus)
{
    *retsnd = 0;
    *retmus = 0;

    ASM_AbortIfClosed;

    *retsnd = sndvol;
    *retmus = musvol;
}

// ASM_SwitchMus
// Changes music to another track, either directly or via a fade-in/out

boolean ASM_SwitchMus(char* loadmus, int fadems, boolean fade)
{
    ASM_AbortIfClosed false;

    switchto = Mix_LoadMUS(loadmus);

    if (switchto == NULL)
    {
#ifdef VERBOSE
        printf("ASM_SwitchMus: %s\n", Mix_GetError());
#endif
        return false;
    }

    if (fade != true)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = switchto;
        Mix_VolumeMusic(musvol);
        if (Mix_PlayMusic(music, -1) == -1)
        {
#ifdef VERBOSE
            printf("ASM_SwitchMus: %s\n", Mix_GetError());
#endif
            return false;
        }
        return true;
    }

    Mix_FadeOutMusic(fadems);
    fadetime = fadems;
    switching = true;

    return true;
}

// ASM_FadeInMus
// Fades in a piece of music, stopping whatever was playing before it

boolean ASM_FadeInMus(char* loadmus, int fadems)
{
    ASM_AbortIfClosed false;

    if (Mix_PlayingMusic())
        Mix_HaltMusic();

    if (music)
        Mix_FreeMusic(music);

    music = Mix_LoadMUS(loadmus);

    if (music == NULL)
    {
#ifdef VERBOSE
        printf("ASM_FadeInMus: %s\n", Mix_GetError());
#endif
        return false;
    }

    Mix_FadeInMusic(music, -1, fadems);

    return true;
}

// ASM_FadeOutMus
// Fades out and stops the music currently playing

void ASM_FadeOutMus(int fadems)
{
    ASM_AbortIfClosed;

    if (!Mix_PlayingMusic())
        return;

    Mix_FadeOutMusic(fadems);
}

// ASM_Cache
// Loads a sound into memory and returns it as a sample

sample ASM_Cache(char* sndfile, const char* name)
{
    sample sound;
    sound.chunk = NULL;
    sound.name = NULL;

    ASM_AbortIfClosed sound;

    Mix_Chunk* chunk = Mix_LoadWAV(sndfile);

    if (chunk == NULL)
    {
#ifdef VERBOSE
        printf("ASM_Cache: %s on sound %s\n", Mix_GetError(), sndfile);
#endif
        return sound;
    }

    sound.chunk = chunk;
    sound.name = (char*)malloc((size_t)strlen(name) + 1);
    strcpy(sound.name, name);

    return sound;
}

// ASM_CacheFromMem
// Loads a sound from a pointer into a sample, returning that sample
// Also frees the original data

sample ASM_CacheFromMem(void* ptr, int size, const char* name)
{
    sample sound;
    sound.chunk = NULL;
    sound.name = NULL;

    ASM_AbortIfClosed sound;

    Mix_Chunk* chunk = Mix_LoadWAV_RW(SDL_RWFromMem(ptr, size), 1);
    if (chunk == NULL)
    {
#ifdef VERBOSE
        printf("ASM_CacheFromMem: %s on sound %s\n", Mix_GetError(), name);
#endif
        return sound;
    }

    sound.chunk = chunk;
    sound.name = (char*)malloc((size_t)strlen(name) + 1);
    strcpy(sound.name, name);

    return sound;
}


// ASM_Uncache
// Removes a sound from memory

void ASM_Uncache(sample sound)
{
    ASM_AbortIfClosed;

    if (sound.chunk == NULL)
        return;

    Mix_FreeChunk(sound.chunk);
    sound.chunk = NULL;
    free(sound.name);
    sound.name = NULL;
}

// ASM_PlaySound
// Play a sound in memory loaded with ASM_Cache or Mix_LoadWAV
// Returns -1 on errors or channel number

int ASM_PlaySound(sample sound, int angle, Uint8 distance, boolean ambient)
{
    ASM_AbortIfClosed - 1;

    if (sound.chunk == NULL)
    {
#ifdef VERBOSE
        printf("ASM_PlaySound: \"%s\" not cached!\n", sound.name);
#endif
        return -1;
    }

    int chanon = ASM_GetFreeChannel();

    if (chanon == -1)
    {
#ifdef VERBOSE
        printf("ASM_PlaySound: No free channels!\n");
#endif
        return -1;
    }

    Mix_Volume(chanon, sndvol);

    if (Mix_PlayChannel(chanon, sound.chunk, (ambient) ? -1 : 0) == -1)
    {
#ifdef VERBOSE
        printf("ASM_PlaySound: %s\n", Mix_GetError());
#endif
        return -1;
    }

    Mix_SetPosition(chanon, angle, distance);

    if (reversemode)
        Mix_SetReverseStereo(chanon, 1);
    else
        Mix_SetReverseStereo(chanon, 0);

    return chanon;
}

// ASM_StopChannel
// Stops sound on a given channel

void ASM_StopChannel(int channel)
{
    ASM_AbortIfClosed;

    Mix_HaltChannel(channel);
}

// ASM_SwitchStep
// Used each program step for switching music tracks

void ASM_SwitchStep(void)
{
    ASM_AbortIfClosed;

    if (Mix_PlayingMusic())
        return;

    if (switching)       // If music is stopped and we want to switch
    {
        Mix_FreeMusic(music);
        music = switchto;
        switchto = 0;
        Mix_FadeInMusic(music, -1, fadetime);
        switching = false;
    }
}

// ASM_ReverseStereo
// Sets reverse stereo mode after initialization

void ASM_ReverseStereo(boolean reverse)
{
    ASM_AbortIfClosed;

    Mix_SetReverseStereo(MIX_CHANNEL_POST, (reverse) ? 1 : 0);
}

// ASM_CurChannels
// Returns the number of channels allocated

int ASM_CurChannels(void)
{
    return lastchan;
}

// SD_MusIsOn
// Returns true if music is on, false otherwise
boolean SD_MusIsOn(void)
{
    return (MusicMode == smm_AdLib) ? true : false;
}

boolean SD_SndIsOn(void)
{
    return (SoundMode == sdm_AdLib) ? true : false;
}

void SD_ChannelDone(int channel)
{
    ASM_ChannelDone(channel);
    channelSoundPos[channel].valid = 0;
}

void SD_Startup(void)
{
    if (nosound)
        return;

    if (!ASM_Open(param_samplerate, MIX_CHANNELS, ASM_ABSMAXCHANNELS / 2, param_audiobuffer,
        128, 128, false))
    {
        printf("SD_Startup: Unable to open audio device. Trying defaults.\n");
        if (!ASM_Open(22050, 16, 128, 2048, 128, 128, false))
        {
            printf("SD_Startup: Unable to open audio device with defaults.\n"
                "            Try closing any other applications that might\n"
                "            be using the audio driver and try again, or\n"
                "            start the program with the '--nosound' switch.\n");
            exit(1);
        }
    }

    Mix_ChannelFinished(SD_ChannelDone);
    memset(ASM_Audiosegs, 0, sizeof(ASM_Audiosegs));
}

void SD_Shutdown(void)
{
    for (int i = 0; i < NUMSOUNDS; i++)
        if (ASM_Audiosegs[i].chunk != NULL)
            UNCACHEAUDIOCHUNK(i);

    ASM_Close();
}

boolean SD_MusicPlaying(void)
{
    ASM_AbortIfClosed false;
    SD_AbortIfMusOff false;

    return Mix_PlayingMusic();
}

// This has changed slightly - it returns 0 on an error, 1 on a successful load

int32_t CA_CacheAudioChunk(int chunk)
{
    if (chunk >= NUMSOUNDS)
    {
#ifdef VERBOSE
        printf("CA_CacheAudioChunk: Invalid sample number %d!", chunk);
#endif
        return 0;
    }

    char* file = (char*)malloc((size_t)(strlen(sounddir) + strlen(ASM_Soundnames[chunk])) + 1);
    strcpy(file, sounddir);
    strcat(file, ASM_Soundnames[chunk]);

    ASM_Audiosegs[chunk] = ASM_Cache(file, ASM_Soundnames[chunk]);

    free(file);
    return 1;
}

void CA_LoadAllSounds(void)
{
    for (int i = 0; i < NUMSOUNDS; i++)
    {
        if (ASM_Audiosegs[i].chunk != NULL)
            continue;
        CA_CacheAudioChunk(i);
    }
}

void SD_MusicOn(void)
{
    ASM_AbortIfClosed;

    SD_AbortIfMusOff;

    ASM_ResumeMusic();
}

int SD_MusicOff(void)
{
    ASM_AbortIfClosed 0;

    ASM_PauseMusic();

    return 0;
}

void SD_StartMusic(int chunk)
{
    ASM_AbortIfClosed;

    SD_AbortIfMusOff;

    if (chunk >= NUMSOUNDS)
    {
#ifdef VERBOSE
        printf("SD_StartMusic: Invalid chunk number %d!", chunk);
#endif
        return;
    }

    char* file = (char*)malloc((size_t)(strlen(musicdir) + strlen(ASM_Musicnames[chunk])) + 1);
    strcpy(file, musicdir);
    strcat(file, ASM_Musicnames[chunk]);

    ASM_PlayMusic(file);
}

boolean SD_SetMusicMode(SMMode mode)
{
    if (mode == MusicMode)
        return true;

    if (mode == smm_Off)
    {
        MusicMode = smm_Off;
        SD_MusicOff();
    }

    else
    {
        MusicMode = smm_AdLib;
        SD_MusicOn();
    }

    return true;
}

boolean SD_SetSoundMode(SDMode mode)
{
    if (mode == SoundMode)
        return true;

    ASM_HaltSound();

    if (mode == sdm_Off)
        SoundMode = sdm_Off;

    else if (mode == sdm_PC)
        SoundMode = sdm_PC;

    else
        SoundMode = sdm_AdLib;
    return true;
}

void SD_StopSound(void)
{
    ASM_AbortIfClosed;

    ASM_HaltSound();
}

void SD_PositionSound(int leftvol, int rightvol)
{
    LeftPosition = leftvol;
    RightPosition = rightvol;
}

void SD_SetPosition(int channel, int leftpos, int rightpos)
{
    if ((leftpos < 0) || (leftpos > 15) || (rightpos < 0) || (rightpos > 15) || ((leftpos == 15) && (rightpos == 15)))
    {
#ifdef VERBOSE
        printf("SD_SetPosition: Illegal position: leftpos - %d, rightpos - %d", leftpos, rightpos);
#endif
        return;
    }
    Mix_SetPanning(channel, ((15 - leftpos) << 4) + 15, ((15 - rightpos) << 4) + 15);
}

int SD_GetAmbIndex(void)
{
    for (int i = 0; i < NUMAMBIENTS; i++)
        if (ambientsnds[i] != 0)
            return i;
    return -1;
}

byte SD_PlaySound(soundnames sound)
{
    SD_AbortIfSndOff 0;

    int lp = LeftPosition;
    int rp = RightPosition;
    boolean amb = ambience;

    LeftPosition = RightPosition = ambience = 0;

    if (sound >= NUMSOUNDS)
    {
#ifdef VERBOSE
        printf("SD_PlaySound: Invalid sound number %d!\n", sound);
#endif
        return 0;
    }

    if (ASM_Audiosegs[sound].chunk == NULL)
    {
#ifdef VERBOSE
        printf("SD_PlaySound: Sound %d not cached!\n", sound);
#endif
        return 0;
    }


    int channel = SD_PlayDigitized(sound, lp, rp, amb);
    return channel;
}

int SD_PlayDigitized(word which, int leftpos, int rightpos, boolean ambient)
{
    SD_AbortIfSndOff 0;

    int channel = ASM_PlayDirect(ASM_Audiosegs[which], ambient);
    if (ambient)
        ambientsnds[SD_GetAmbIndex()] = channel;
    ambient = false;
    SD_SetPosition(channel, leftpos, rightpos);
    return channel;
}

int SD_AmbientSound(void)
{
    int ambindex = SD_GetAmbIndex();
    if (ambindex > -1)
        ambience = true;
    return ambindex;
}

void SD_StopAmbient(int ambindex)
{
    if (ambindex >= NUMAMBIENTS)
        return;

    ASM_StopChannel(ambientsnds[ambindex]);
}

void SD_SetDigiDevice(SDSMode mode)
{
    boolean devicenotpresent;

    if (mode == DigiMode)
        return;

    SD_StopDigitized();

    devicenotpresent = false;
    switch (mode)
    {
    case sds_SoundBlaster:
        if (!SoundBlasterPresent)
            devicenotpresent = true;
        break;
    case sds_PC:
        if (!SoundPositioned)
            devicenotpresent = true;
        break;
    case sds_Off:
        for (int i; i < DigiMode; i++)
        {
            SD_Shutdown();
        }
    }

    if (!devicenotpresent)
    {
        DigiMode = mode;
    }
}
#endif