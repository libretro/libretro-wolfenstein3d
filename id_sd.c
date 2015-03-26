//
//      ID Engine
//      ID_SD.c - Sound Manager for Wolfenstein 3D
//      v1.2
//      By Jason Blochowiak
//

//
//      This module handles dealing with generating sound on the appropriate
//              hardware
//
//      Depends on: User Mgr (for parm checking)
//
//      Globals:
//              For User Mgr:
//                      SoundBlasterPresent - SoundBlaster card present?
//                      AdLibPresent - AdLib card present?
//                      SoundMode - What device is used for sound effects
//                              (Use SM_SetSoundMode() to set)
//                      MusicMode - What device is used for music
//                              (Use SM_SetMusicMode() to set)
//                      DigiMode - What device is used for digitized sound effects
//                              (Use SM_SetDigiDevice() to set)
//
//              For Cache Mgr:
//                      NeedsDigitized - load digitized sounds?
//                      NeedsMusic - load music?
//

#include "wl_def.h"
#include "SDL_mixer/SDL_mixer.h"
#include "fmopl.h"

#define ORIGSAMPLERATE 7042

typedef struct
{
    char RIFF[4];
    longword filelenminus8;
    char WAVE[4];
    char fmt_[4];
    longword formatlen;
    word val0x0001;
    word channels;
    longword samplerate;
    longword bytespersec;
    word bytespersample;
    word bitspersample;
} headchunk;

typedef struct
{
    char chunkid[4];
    longword chunklength;
} wavechunk;

typedef struct
{
    uint32_t startpage;
    uint32_t length;
} digiinfo;

static Mix_Chunk *SoundChunks[ STARTMUSIC - STARTDIGISOUNDS];
static byte      *SoundBuffers[STARTMUSIC - STARTDIGISOUNDS];

globalsoundpos channelSoundPos[MIX_CHANNELS];

/* Global variables */
boolean         AdLibPresent,
                SoundBlasterPresent,SBProPresent,
                SoundPositioned;
SDMode          SoundMode;
SMMode          MusicMode;
SDSMode         DigiMode;

static  byte          **SoundTable;
        int             DigiMap[LASTSOUND];
        int             DigiChannel[STARTMUSIC - STARTDIGISOUNDS];

/*      Internal variables */
static  boolean                 SD_Started;
static  boolean                 nextsoundpos;
static  soundnames              SoundNumber;
static  soundnames              DigiNumber;
static  word                    SoundPriority;
static  word                    DigiPriority;
static  int                     LeftPosition;
static  int                     RightPosition;

        word                    NumDigi;
static  digiinfo               *DigiList;
static  boolean                 DigiPlaying;

//      PC Sound variables

static  byte * volatile         pcSound;


//      AdLib variables
static  byte * volatile         alSound;
static  byte                    alBlock;
static  longword                alLengthLeft;
static  longword                alTimeCount;
static  Instrument              alZeroInst;

//      Sequencer variables
static  volatile boolean        sqActive;
static  word                   *sqHack;
static  word                   *sqHackPtr;
static  int                     sqHackLen;
static  int                     sqHackSeqLen;
static  longword                sqHackTime;


static void SDL_SoundFinished(void)
{
   SoundNumber   = (soundnames)0;
   SoundPriority = 0;
}

void SD_StopDigitized(void)
{
   DigiPlaying = false;
   DigiNumber = (soundnames) 0;
   DigiPriority = 0;
   SoundPositioned = false;
   if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
      SDL_SoundFinished();

   switch (DigiMode)
   {
      case sds_PC:
         break;
      case sds_SoundBlaster:
         Mix_HaltChannel(-1);
         break;
   }
}

int SD_GetChannelForDigi(int which)
{
   int channel;
   if(DigiChannel[which] != -1)
      return DigiChannel[which];

   channel = Mix_GroupAvailable(1);
   if(channel == -1)
      channel = Mix_GroupOldest(1);

   if(channel == -1)           /* All sounds stopped in the meantime? */
      return Mix_GroupAvailable(1);
   return channel;
}

void SD_SetPosition(int channel, int leftpos, int rightpos)
{
   if((leftpos < 0) || (leftpos > 15) || (rightpos < 0) || (rightpos > 15)
         || ((leftpos == 15) && (rightpos == 15)))
      Quit("SD_SetPosition: Illegal position");

   switch (DigiMode)
   {
      case sds_SoundBlaster:
         Mix_SetPanning(channel, ((15 - leftpos) << 4) + 15,
               ((15 - rightpos) << 4) + 15);
         break;
   }
}

static Sint16 GetSample(float csample, byte *samples, int size)
{
   float val;
   int32_t intval;
   float s0=0, s1=0, s2=0;
   int cursample = (int) csample;
   float sf = csample - (float) cursample;

   if(cursample-1 >= 0)
      s0 = (float) (samples[cursample-1] - 128);

   s1 = (float) (samples[cursample] - 128);

   if(cursample+1 < size)
      s2 = (float) (samples[cursample+1] - 128);

   val    = s0*sf*(sf-1)/2 - s1*(sf*sf-1) + s2*(sf+1)*sf/2;
   intval = (int32_t) (val * 256);

   if(intval < -32768)
      intval = -32768;
   else if(intval > 32767)
      intval = 32767;
   return (Sint16) intval;
}

void SD_PrepareSound(int which)
{
   unsigned i;
   if(DigiList == NULL)
      Quit("SD_PrepareSound(%i): DigiList not initialized!\n", which);

   int page = DigiList[which].startpage;
   int size = DigiList[which].length;

   byte *origsamples = PM_GetSound(page);
   if(origsamples + size >= PM_GetEnd())
      Quit("SD_PrepareSound(%i): Sound reaches out of page file!\n", which);

   int destsamples = (int) ((float) size * (float) param_samplerate
         / (float) ORIGSAMPLERATE);

   byte *wavebuffer = (byte *) malloc(sizeof(headchunk) + sizeof(wavechunk)
         + destsamples * 2);     // dest are 16-bit samples
   if(wavebuffer == NULL)
      Quit("Unable to allocate wave buffer for sound %i!\n", which);

   headchunk head = {{'R','I','F','F'}, 0, {'W','A','V','E'},
      {'f','m','t',' '}, 0x10, 0x0001, 1, param_samplerate, param_samplerate*2, 2, 16};
   wavechunk dhead = {{'d', 'a', 't', 'a'}, destsamples*2};
   head.filelenminus8 = sizeof(head) + destsamples*2;  // (sizeof(dhead)-8 = 0)
   memcpy(wavebuffer, &head, sizeof(head));
   memcpy(wavebuffer+sizeof(head), &dhead, sizeof(dhead));

   // alignment is correct, as wavebuffer comes from malloc
   // and sizeof(headchunk) % 4 == 0 and sizeof(wavechunk) % 4 == 0
   Sint16 *newsamples = (Sint16 *)(void *) (wavebuffer + sizeof(headchunk)
         + sizeof(wavechunk));
   float cursample = 0.F;
   float samplestep = (float) ORIGSAMPLERATE / (float) param_samplerate;
   for(i=0; i<destsamples; i++, cursample+=samplestep)
   {
      newsamples[i] = GetSample((float)size * (float)i / (float)destsamples,
            origsamples, size);
   }
   SoundBuffers[which] = wavebuffer;

   SoundChunks[which] = Mix_LoadWAV_RW(SDL_RWFromMem(wavebuffer,
            sizeof(headchunk) + sizeof(wavechunk) + destsamples * 2), 1);
}

int SD_PlayDigitized(word which,int leftpos,int rightpos)
{
   if (!DigiMode)
      return 0;

   if (which >= NumDigi)
      Quit("SD_PlayDigitized: bad sound number %i", which);

   int channel = SD_GetChannelForDigi(which);
   SD_SetPosition(channel, leftpos,rightpos);

   DigiPlaying = true;

   Mix_Chunk *sample = SoundChunks[which];
   if(sample == NULL)
   {
      printf("SoundChunks[%i] is NULL!\n", which);
      return 0;
   }

   if(Mix_PlayChannel(channel, sample, 0) == -1)
   {
      printf("Unable to play sound: %s\n", Mix_GetError());
      return 0;
   }

   return channel;
}

void SD_ChannelFinished(int channel)
{
   channelSoundPos[channel].valid = 0;
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
   }

   if (!devicenotpresent)
      DigiMode = mode;
}

void SDL_SetupDigi(void)
{
   /* Correct padding enforced by PM_Startup() */
   int i;
   word *soundInfoPage = (word *) (void *) PM_GetPage(ChunksInFile-1);
   NumDigi = (word) PM_GetPageSize(ChunksInFile - 1) / 4;

   DigiList = (digiinfo *) malloc(NumDigi * sizeof(digiinfo));

   for(i = 0; i < NumDigi; i++)
   {
      unsigned page;
      // Calculate the size of the digi from the sizes of the pages between
      // the start page and the start page of the next sound

      DigiList[i].startpage = soundInfoPage[i * 2];
      if((int) DigiList[i].startpage >= ChunksInFile - 1)
      {
         NumDigi = i;
         break;
      }

      int lastPage;
      if(i < NumDigi - 1)
      {
         lastPage = soundInfoPage[i * 2 + 2];
         if(lastPage == 0 || lastPage + PMSoundStart > ChunksInFile - 1) lastPage = ChunksInFile - 1;
         else lastPage += PMSoundStart;
      }
      else lastPage = ChunksInFile - 1;

      int size = 0;
      for(page = PMSoundStart + DigiList[i].startpage; page < lastPage; page++)
         size += PM_GetPageSize(page);

      /* Don't include padding of sound info page, if padding was added */
      if(lastPage == ChunksInFile - 1 && PMSoundInfoPagePadded)
         size--;

      // Patch lower 16-bit of size with size from sound info page.
      // The original VSWAP contains padding which is included in the page size,
      // but not included in the 16-bit size. So we use the more precise value.
      if((size & 0xffff0000) != 0 && (size & 0xffff) < soundInfoPage[i * 2 + 1])
         size -= 0x10000;
      size = (size & 0xffff0000) | soundInfoPage[i * 2 + 1];

      DigiList[i].length = size;
   }

   for(i = 0; i < LASTSOUND; i++)
   {
      DigiMap[i] = -1;
      DigiChannel[i] = -1;
   }
}

//      AdLib Code

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ALStopSound() - Turns off any sound effects playing through the
//              AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void SDL_ALStopSound(void)
{
   alSound = 0;
   YM3812Write(0, alFreqH + 0, 0);
}

static void SDL_AlSetFXInst(Instrument *inst)
{
   byte m = 0;      // modulator cell for channel 0
   byte c = 3;      // carrier cell for channel 0
   YM3812Write(0, m + alChar,inst->mChar);
   YM3812Write(0, m + alScale,inst->mScale);
   YM3812Write(0, m + alAttack,inst->mAttack);
   YM3812Write(0, m + alSus,inst->mSus);
   YM3812Write(0, m + alWave,inst->mWave);
   YM3812Write(0, c + alChar,inst->cChar);
   YM3812Write(0, c + alScale,inst->cScale);
   YM3812Write(0, c + alAttack,inst->cAttack);
   YM3812Write(0, c + alSus,inst->cSus);
   YM3812Write(0, c + alWave,inst->cWave);

   YM3812Write(0, alFeedCon,0);
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ALPlaySound() - Plays the specified sound on the AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void SDL_ALPlaySound(AdLibSound *sound)
{
   Instrument      *inst;
   byte            *data;

   SDL_ALStopSound();

   alLengthLeft = sound->common.length;
   data         = sound->data;
   alBlock      = ((sound->block & 7) << 2) | 0x20;
   inst         = &sound->inst;

   if (!(inst->mSus | inst->cSus))
   {
      Quit("SDL_ALPlaySound() - Bad instrument");
   }

   SDL_AlSetFXInst(inst);
   alSound = (byte *)data;
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ShutAL() - Shuts down the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void SDL_ShutAL(void)
{
   alSound = 0;
   YM3812Write(0, alEffects,0);
   YM3812Write(0, alFreqH + 0,0);
   SDL_AlSetFXInst(&alZeroInst);
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_CleanAL() - Totally shuts down the AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void SDL_CleanAL(void)
{
   int     i;

   YM3812Write(0, alEffects,0);
   for (i = 1; i < 0xf5; i++)
      YM3812Write(0, i, 0);
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_StartAL() - Starts up the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void SDL_StartAL(void)
{
   YM3812Write(0, alEffects, 0);
   SDL_AlSetFXInst(&alZeroInst);
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_DetectAdLib() - Determines if there's an AdLib (or SoundBlaster
//              emulating an AdLib) present
//
///////////////////////////////////////////////////////////////////////////
static boolean SDL_DetectAdLib(void)
{
   unsigned i;
   for (i = 1; i <= 0xf5; i++)       // Zero all the registers
      YM3812Write(0, i, 0);

   YM3812Write(0, 1, 0x20);             // Set WSE=1

   return true;
}

////////////////////////////////////////////////////////////////////////////
//
//      SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////
static void SDL_ShutDevice(void)
{
   switch (SoundMode)
   {
      case sdm_PC:
#if 0
         SDL_ShutPC();
#endif
         break;
      case sdm_AdLib:
         SDL_ShutAL();
         break;
   }
   SoundMode = sdm_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_CleanDevice() - totally shuts down all sound devices
//
///////////////////////////////////////////////////////////////////////////
static void SDL_CleanDevice(void)
{
   if ((SoundMode == sdm_AdLib) || (MusicMode == smm_AdLib))
      SDL_CleanAL();
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////
static void SDL_StartDevice(void)
{
   switch (SoundMode)
   {
      case sdm_AdLib:
         SDL_StartAL();
         break;
   }
   SoundNumber = (soundnames) 0;
   SoundPriority = 0;
}

//      Public routines

///////////////////////////////////////////////////////////////////////////
//
//      SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetSoundMode(SDMode mode)
{
   boolean result = false;
   word    tableoffset;

   SD_StopSound();

   if ((mode == sdm_AdLib) && !AdLibPresent)
      mode = sdm_PC;

   switch (mode)
   {
      case sdm_Off:
         tableoffset = STARTADLIBSOUNDS;
         result = true;
         break;
      case sdm_PC:
         tableoffset = STARTPCSOUNDS;
         result = true;
         break;
      case sdm_AdLib:
         tableoffset = STARTADLIBSOUNDS;
         if (AdLibPresent)
            result = true;
         break;
      default:
         Quit("SD_SetSoundMode: Invalid sound mode %i", mode);
         return false;
   }
   SoundTable = &audiosegs[tableoffset];

   if (result && (mode != SoundMode))
   {
      SDL_ShutDevice();
      SoundMode = mode;
      SDL_StartDevice();
   }

   return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetMusicMode(SMMode mode)
{
   boolean result = false;

   SD_FadeOutMusic();
   while (SD_MusicPlaying())
      rarch_sleep(5);

   switch (mode)
   {
      case smm_Off:
         result = true;
         break;
      case smm_AdLib:
         if (AdLibPresent)
            result = true;
         break;
   }

   if (result)
      MusicMode = mode;

   return(result);
}

int numreadysamples = 0;
byte *curAlSound = 0;
byte *curAlSoundPtr = 0;
longword curAlLengthLeft = 0;
int soundTimeCounter = 5;
int samplesPerMusicTick;

void SDL_IMFMusicPlayer(void *udata, Uint8 *stream, int len)
{
   int stereolen = len>>1;
   int sampleslen = stereolen>>1;
   INT16 *stream16 = (INT16 *) (void *) stream;    // expect correct alignment

   while(1)
   {
      if(numreadysamples)
      {
         if(numreadysamples < sampleslen)
         {
            YM3812UpdateOne(0, stream16, numreadysamples);
            stream16 += numreadysamples*2;
            sampleslen -= numreadysamples;
         }
         else
         {
            YM3812UpdateOne(0, stream16, sampleslen);
            numreadysamples -= sampleslen;
            return;
         }
      }
      soundTimeCounter--;
      if(!soundTimeCounter)
      {
         soundTimeCounter = 5;
         if(curAlSound != alSound)
         {
            curAlSound = curAlSoundPtr = alSound;
            curAlLengthLeft = alLengthLeft;
         }
         if(curAlSound)
         {
            if(*curAlSoundPtr)
            {
               YM3812Write(0, alFreqL, *curAlSoundPtr);
               YM3812Write(0, alFreqH, alBlock);
            }
            else YM3812Write(0, alFreqH, 0);
            curAlSoundPtr++;
            curAlLengthLeft--;
            if(!curAlLengthLeft)
            {
               curAlSound = alSound = 0;
               SoundNumber = (soundnames) 0;
               SoundPriority = 0;
               YM3812Write(0, alFreqH, 0);
            }
         }
      }
      if(sqActive)
      {
         do
         {
            if(sqHackTime > alTimeCount) break;
            sqHackTime = alTimeCount + *(sqHackPtr+1);
            YM3812Write(0, *(byte *) sqHackPtr, *(((byte *) sqHackPtr)+1));
            sqHackPtr += 2;
            sqHackLen -= 4;
         }
         while(sqHackLen>0);
         alTimeCount++;
         if(!sqHackLen)
         {
            sqHackPtr = sqHack;
            sqHackLen = sqHackSeqLen;
            sqHackTime = 0;
            alTimeCount = 0;
         }
      }
      numreadysamples = samplesPerMusicTick;
   }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Startup() - starts up the Sound Mgr
//              Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////
void SD_Startup(void)
{
   int     i;

   if (SD_Started)
      return;

   if(Mix_OpenAudio(param_samplerate, AUDIO_S16, 2, param_audiobuffer))
   {
      printf("Unable to open audio: %s\n", Mix_GetError());
      return;
   }

   Mix_ReserveChannels(2);  // reserve player and boss weapon channels
   Mix_GroupChannels(2, MIX_CHANNELS-1, 1); // group remaining channels

   // Init music

   samplesPerMusicTick = param_samplerate / 700;    // SDL_t0FastAsmService played at 700Hz

   if(YM3812Init(1,3579545,param_samplerate))
      printf("Unable to create virtual OPL!!\n");

   for(i=1;i<0xf6;i++)
      YM3812Write(0,i,0);

   YM3812Write(0,1,0x20); // Set WSE=1

   Mix_HookMusic(SDL_IMFMusicPlayer, 0);
   Mix_ChannelFinished(SD_ChannelFinished);
   AdLibPresent = true;
   SoundBlasterPresent = true;

   alTimeCount = 0;

   SD_SetSoundMode(sdm_Off);
   SD_SetMusicMode(smm_Off);

   SDL_SetupDigi();

   SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Shutdown() - shuts down the Sound Mgr
//              Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////
void SD_Shutdown(void)
{
   unsigned i;
   if (!SD_Started)
      return;

   SD_MusicOff();
   SD_StopSound();

   for(i = 0; i < STARTMUSIC - STARTDIGISOUNDS; i++)
   {
      if(SoundChunks[i])
         Mix_FreeChunk(SoundChunks[i]);
      if(SoundBuffers[i])
         free(SoundBuffers[i]);
   }

   free(DigiList);

   SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_PositionSound() - Sets up a stereo imaging location for the next
//              sound to be played. Each channel ranges from 0 to 15.
//
///////////////////////////////////////////////////////////////////////////
void SD_PositionSound(int leftvol,int rightvol)
{
   LeftPosition = leftvol;
   RightPosition = rightvol;
   nextsoundpos = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
boolean SD_PlaySound(soundnames sound)
{
   SoundCommon     *s;
   int lp = LeftPosition;
   int rp = RightPosition;
   LeftPosition = 0;
   RightPosition = 0;
   boolean ispos = nextsoundpos;
   nextsoundpos = false;

   if (sound == -1 || (DigiMode == sds_Off && SoundMode == sdm_Off))
      return 0;

   s = (SoundCommon *) SoundTable[sound];

   if ((SoundMode != sdm_Off) && !s)
      Quit("SD_PlaySound() - Uncached sound");

   if ((DigiMode != sds_Off) && (DigiMap[sound] != -1))
   {
      if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
      {
         return 0;
      }
      else
      {

         int channel = SD_PlayDigitized(DigiMap[sound], lp, rp);
         SoundPositioned = ispos;
         DigiNumber = sound;
         DigiPriority = s->priority;
         return channel + 1;
      }

      return(true);
   }

   if (SoundMode == sdm_Off)
      return 0;

   if (!s->length)
      Quit("SD_PlaySound() - Zero length sound");
   if (s->priority < SoundPriority)
      return 0;

   switch (SoundMode)
   {
      case sdm_PC:
         //            SDL_PCPlaySound((PCSound *)s);
         break;
      case sdm_AdLib:
         SDL_ALPlaySound((AdLibSound *)s);
         break;
   }

   SoundNumber = sound;
   SoundPriority = s->priority;

   return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//              no sound is playing
//
///////////////////////////////////////////////////////////////////////////
word SD_SoundPlaying(void)
{
   boolean result = false;

   switch (SoundMode)
   {
      case sdm_PC:
         result = pcSound? true : false;
         break;
      case sdm_AdLib:
         result = alSound? true : false;
         break;
   }

   if (result)
      return SoundNumber;
   return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void
SD_StopSound(void)
{
   if (DigiPlaying)
      SD_StopDigitized();

   switch (SoundMode)
   {
      case sdm_PC:
#if 0
         SDL_PCStopSound();
#endif
         break;
      case sdm_AdLib:
         SDL_ALStopSound();
         break;
   }

   SoundPositioned = false;

   SDL_SoundFinished();
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void
SD_WaitSoundDone(void)
{
   while (SD_SoundPlaying())
      rarch_sleep(5);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOn(void)
{
    sqActive = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicOff() - turns off the sequencer and any playing notes
//      returns the last music offset for music continue
//
///////////////////////////////////////////////////////////////////////////
int SD_MusicOff(void)
{
   word    i;

   sqActive = false;
   switch (MusicMode)
   {
      case smm_AdLib:
         YM3812Write(0, alEffects, 0);
         for (i = 0;i < sqMaxTracks;i++)
            YM3812Write(0, alFreqH + i + 1, 0);
         break;
   }

   return (int) (sqHackPtr-sqHack);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void SD_StartMusic(int chunk)
{
   SD_MusicOff();

   if (MusicMode == smm_AdLib)
   {
      int32_t chunkLen = CA_CacheAudioChunk(chunk);
      sqHack = (word *)(void *) audiosegs[chunk];     // alignment is correct
      if(*sqHack == 0) sqHackLen = sqHackSeqLen = chunkLen;
      else sqHackLen = sqHackSeqLen = *sqHack++;
      sqHackPtr = sqHack;
      sqHackTime = 0;
      alTimeCount = 0;
      SD_MusicOn();
   }
}

void SD_ContinueMusic(int chunk, int startoffs)
{
   SD_MusicOff();

   if (MusicMode == smm_AdLib)
   {
      unsigned i;
      int32_t chunkLen = CA_CacheAudioChunk(chunk);
      sqHack = (word *)(void *) audiosegs[chunk];     // alignment is correct
      if(*sqHack == 0) sqHackLen = sqHackSeqLen = chunkLen;
      else sqHackLen = sqHackSeqLen = *sqHack++;
      sqHackPtr = sqHack;

      if(startoffs >= sqHackLen)
      {
         Quit("SD_StartMusic: Illegal startoffs provided!");
      }

      // fast forward to correct position
      // (needed to reconstruct the instruments)

      for(i = 0; i < startoffs; i += 2)
      {
         byte reg = *(byte *)sqHackPtr;
         byte val = *(((byte *)sqHackPtr) + 1);
         if(reg >= 0xb1 && reg <= 0xb8) val &= 0xdf;           // disable play note flag
         else if(reg == 0xbd) val &= 0xe0;                     // disable drum flags

         YM3812Write(0, reg,val);
         sqHackPtr += 2;
         sqHackLen -= 4;
      }
      sqHackTime = 0;
      alTimeCount = 0;

      SD_MusicOn();
   }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//              to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////
void SD_FadeOutMusic(void)
{
   switch (MusicMode)
   {
      case smm_AdLib:
         // DEBUG - quick hack to turn the music off
         SD_MusicOff();
         break;
   }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicPlaying() - returns true if music is currently playing, false if
//              not
//
///////////////////////////////////////////////////////////////////////////
boolean SD_MusicPlaying(void)
{
   boolean result;

   switch (MusicMode)
   {
      case smm_AdLib:
         result = sqActive;
         break;
      default:
         result = false;
         break;
   }

   return(result);
}
