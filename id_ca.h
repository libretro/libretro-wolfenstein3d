#ifndef __ID_CA_H_
#define __ID_CA_H_

//===========================================================================

#define NUMMAPS         60
#define MAPPLANES       3
#include "wl_def.h"
#ifdef VIEASM

#else
#define UNCACHEAUDIOCHUNK(chunk) {if(audiosegs[chunk]) {free(audiosegs[chunk]); audiosegs[chunk]=NULL;}}
#endif
//===========================================================================

typedef struct
{
    int32_t planestart[MAPPLANES];
    word    planelength[MAPPLANES];
    word    width,height;
    char    name[16];
} maptype;

//===========================================================================

extern  word    *mapsegs[MAPPLANES];
extern  maptype *mapheaderseg[NUMMAPS];
#ifdef VIEASM

#else
extern  byte    *audiosegs[NUMSNDCHUNKS];
#endif
extern  byte    *grsegs[NUMCHUNKS];

extern  char  extension[5];
extern  char  graphext[5];
extern  char  audioext[5];

#ifdef VIEASM

#else
extern  char  audioext[5];
#endif
//===========================================================================

boolean CA_LoadFile (const char *filename, void **ptr);
boolean CA_WriteFile (const char *filename, void *ptr, int32_t length);

int32_t CA_RLEWCompress (word *source, int32_t length, word *dest, word rlewtag);

void CA_RLEWexpand (word *source, word *dest, int32_t length, word rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

int32_t CA_CacheAudioChunk (int chunk);
void CA_LoadAllSounds (void);

void CA_CacheGrChunks (void);
void CA_CacheMap (int mapnum);

void CA_CannotOpen(const char *name);

#endif
