// ID_CA.C

// this has been customized for WOLF

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include <sys/types.h>
#if defined _WIN32
    #include <io.h>
#elif defined _arch_dreamcast
    #include <unistd.h>
#elif defined(SWITCH)
	#include <sys/_iovec.h>
#elif defined(N3DS) 
struct iovec 
{
     void *iov_base;     
     size_t iov_len;    
};
#else
    //#include <sys/uio.h>
    #include <unistd.h>
#endif

#include "wl_def.h"

#define THREEBYTEGRSTARTS

/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
    word bit0,bit1;       // 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
    word RLEWtag;
#if MAPPLANES >= 4
    word numplanes;       // unused, but WDC needs 2 bytes here for internal usage
#endif
    int32_t headeroffsets[NUMMAPS];
} mapfiletype;


/*
=============================================================================

                             GLOBAL VARIABLES

=============================================================================
*/

word *mapsegs[MAPPLANES];
maptype *mapheaderseg[NUMMAPS];
#ifdef VIEASM

#else
byte *audiosegs[NUMSNDCHUNKS];
#endif
byte *grsegs[NUMCHUNKS];

mapfiletype *tinf;

/*
=============================================================================

                             LOCAL VARIABLES

=============================================================================
*/

char extension[5]; // Need a string, not constant to change cache files
char graphext[5];


char audioext[5];

#if defined(SWITCH) //|| defined(N3DS)
static const char gheadname[] = DATADIR "vgahead.";
static const char gfilename[] = DATADIR "vgagraph.";
static const char gdictname[] = DATADIR "vgadict.";
static const char mheadname[] = DATADIR "maphead.";
static const char mfilename[] = DATADIR "maptemp.";
#ifdef CARMACIZED
static const char mfilecama[] = DATADIR "gamemaps.";
#endif
#ifdef VIEASM
#else
static const char aheadname[] = DATADIR "audiohed.";
#endif
static const char afilename[] = DATADIR "audiot.";
#else
static const char gheadname[] = "vgahead.";
static const char gfilename[] = "vgagraph.";
static const char gdictname[] = "vgadict.";
static const char mheadname[] = "maphead.";
#ifdef CARMACIZED
static const char mfilecama[] = "gamemaps.";
#endif
static const char mfilename[] = "maptemp.";
#ifdef VIEASM
#else
static const char aheadname[] = "audiohed.";
#endif
static const char afilename[] = "audiot.";
#endif
void CA_CannotOpen(const char *string);

static int32_t  grstarts[NUMCHUNKS + 1];
static int32_t* audiostarts; // array of offsets in audio / audiot

#ifdef VIEASM

#else
static s32* audiostarts; // array of offsets in audio / audiot
#endif

#ifdef GRHEADERLINKED
huffnode *grhuffman;
#else
huffnode grhuffman[255];
#endif

int    grhandle = -1;               // handle to EGAGRAPH
int    maphandle = -1;              // handle to MAPTEMP / GAMEMAPS

#ifdef VIEASM

#else
int    audiohandle = -1;            // handle to AUDIOT / AUDIO
#endif
s32   chunkcomplen,chunkexplen;

#ifdef VIEASM
#else
SDMode oldsoundmode;
#endif


static int32_t GRFILEPOS(const size_t idx)
{
	assert(idx < lengthof(grstarts));
	return grstarts[idx];
}

/*
=============================================================================

                            LOW LEVEL ROUTINES

=============================================================================
*/

/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

void CAL_GetGrChunkLength (int chunk)
{
    lseek(grhandle,GRFILEPOS(chunk),SEEK_SET);
    read(grhandle,&chunkexplen,sizeof(chunkexplen));
    chunkcomplen = GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
}


/*
==========================
=
= CA_WriteFile
=
= Writes a file from a memory buffer
=
==========================
*/

boolean CA_WriteFile (const char *filename, void *ptr, int32_t length)
{
    const int handle = open(filename, O_CREAT | O_WRONLY | O_BINARY, 0644);
    if (handle == -1)
        return false;

    if (!write (handle,ptr,length))
    {
        close (handle);
        return false;
    }
    close (handle);
    return true;
}



/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

boolean CA_LoadFile (const char *filename, void **ptr)
{
    int32_t size;

    const int handle = open(filename, O_RDONLY | O_BINARY);
    if (handle == -1)
        return false;

    size = lseek(handle, 0, SEEK_END);
    lseek(handle, 0, SEEK_SET);
    *ptr = SafeMalloc(size);

    if (!read (handle,*ptr,size))
    {
        close (handle);
        return false;
    }
    close (handle);
    return true;
}

/*
============================================================================

                COMPRESSION routines, see JHUFF.C for more

============================================================================
*/

static void CAL_HuffExpand(byte *source, byte *dest, int32_t length, huffnode *hufftable)
{
    byte *end;
    huffnode *headptr, *huffptr;

    if(!length || !dest)
    {
        Quit("length or dest is null!");
        return;
    }

    headptr = hufftable+254;        // head node is always node 254

    int written = 0;

    end=dest+length;

    byte val = *source++;
    byte mask = 1;
    word nodeval;
    huffptr = headptr;
    while(1)
    {
        if(!(val & mask))
            nodeval = huffptr->bit0;
        else
            nodeval = huffptr->bit1;
        if(mask==0x80)
        {
            val = *source++;
            mask = 1;
        }
        else mask <<= 1;

        if(nodeval<256)
        {
            *dest++ = (byte) nodeval;
            written++;
            huffptr = headptr;
            if(dest>=end) break;
        }
        else
        {
            huffptr = hufftable + (nodeval - 256);
        }
    }
}

/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/


void CAL_CarmackExpand (byte *source, word *dest, int length)
{
    word ch,chhigh,count,offset;
    byte *inptr;
    word *copyptr, *outptr;

    #define NEARTAG 0xa7
    #define FARTAG  0xa8

    length/=2;

    inptr = (byte *) source;
    outptr = dest;

    while (length>0)
    {
        ch = READWORD(inptr);
        inptr += 2;
        chhigh = ch>>8;
        if (chhigh == NEARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr++ = ch;
                length--;
            }
            else
            {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;
                if(length<0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else if (chhigh == FARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr++ = ch;
                length --;
            }
            else
            {
                offset = READWORD(inptr);
                inptr += 2;
                copyptr = dest + offset;
                length -= count;
                if(length<0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else
        {
            *outptr++ = ch;
            length --;
        }
    }
}

/*
======================
=
= CA_RLEWcompress
=
======================
*/

int32_t CA_RLEWCompress (word *source, int32_t length, word *dest, word rlewtag)
{
    word value,count;
    unsigned i;
    word *start,*end;

    start = dest;

    end = source + (length+1)/2;

    //
    // compress it
    //
    do
    {
        count = 1;
        value = *source++;
        while (*source == value && source<end)
        {
            count++;
            source++;
        }
        if (count>3 || value == rlewtag)
        {
            //
            // send a tag / count / value string
            //
            *dest++ = rlewtag;
            *dest++ = count;
            *dest++ = value;
        }
        else
        {
            //
            // send word without compressing
            //
            for (i=1;i<=count;i++)
                *dest++ = value;
        }

    } while (source<end);

    return (int32_t)(2*(dest-start));
}


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (word *source, word *dest, int32_t length, word rlewtag)
{
    word value,count,i;
    word *end=dest+length/2;

//
// expand it
//
    do
    {
        value = *source++;
        if (value != rlewtag)
            //
            // uncompressed
            //
            *dest++=value;
        else
        {
            //
            // compressed string
            //
            count = *source++;
            value = *source++;
            for (i=1;i<=count;i++)
                *dest++ = value;
        }
    } while (dest<end);
}



/*
=============================================================================

                                         CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
#if defined(SWITCH) || defined (N3DS)
    char fname[13 + sizeof(DATADIR)];
#else    
    char fname[13];
#endif   
    int handle;
    byte *compseg;

//
// load ???dict.ext (huffman dictionary for graphics files)
//

    strcpy(fname,gdictname);
    strcat(fname,graphext);

    handle = open(fname, O_RDONLY | O_BINARY);
    if (handle == -1)
        CA_CannotOpen(fname);

    read(handle, grhuffman, sizeof(grhuffman));
    close(handle);

    // load the data offsets from ???head.ext
    strcpy(fname,gheadname);
    strcat(fname,graphext);

    handle = open(fname, O_RDONLY | O_BINARY);
    if (handle == -1)
        CA_CannotOpen(fname);

    long headersize = lseek(handle, 0, SEEK_END);
    lseek(handle, 0, SEEK_SET);

	int expectedsize = lengthof(grstarts);

    if(!param_ignorenumchunks && headersize / 3 != (long) expectedsize)
        Quit("Wolf4SDL was not compiled for these data files:\n"
            "%s contains a wrong number of offsets (%i instead of %i)!\n\n"
            "Please check whether you are using the right executable!\n"
            "(For mod developers: perhaps you forgot to update NUMCHUNKS?)",
            fname, headersize / 3, expectedsize);

    byte data[lengthof(grstarts) * 3];
    read(handle, data, sizeof(data));
    close(handle);

    const byte* d = data;
    int32_t* i;
    for (i = grstarts; i != endof(grstarts); ++i)
    {
        const int32_t val = d[0] | d[1] << 8 | d[2] << 16;
        *i = (val == 0x00FFFFFF ? -1 : val);
        d += 3;
    }

//
// Open the graphics file
//
    strcpy(fname,gfilename);
    strcat(fname,graphext);

    grhandle = open(fname, O_RDONLY | O_BINARY);
    if (grhandle == -1)
        CA_CannotOpen(fname);


//
// load the pic and sprite headers into the arrays in the data segment
//
    pictable = SafeMalloc(NUMPICS * sizeof(*pictable));
    CAL_GetGrChunkLength(STRUCTPIC);                // position file pointer
    compseg = SafeMalloc(chunkcomplen);
    read (grhandle,compseg,chunkcomplen);
    CAL_HuffExpand(compseg, (byte*)pictable, NUMPICS * sizeof(*pictable), grhuffman);
    free(compseg);

    CA_CacheGrChunks ();

    close (grhandle);
}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
#if defined SWITCH && defined N3DS  
    printf("CA_SetupMapFile_Start\n");
#endif   
    int     i;
    int handle;
    s32 pos;
#if defined SWITCH && defined N3DS
    char fname[13 + sizeof(DATADIR)];
#else
    char fname[13];
#endif
//
// load maphead.ext (offsets and tileinfo for map file)
//
    strcpy(fname,mheadname);
    strcat(fname,extension);

    handle = open(fname, O_RDONLY | O_BINARY);
    if (handle == -1)
        CA_CannotOpen(fname);

    tinf = SafeMalloc(sizeof(*tinf));

    read(handle, tinf, sizeof(*tinf));
    close(handle);

//
// open the data file
//
#ifdef CARMACIZED
    strcpy(fname, mfilecama);    
    strcat(fname, extension);

    maphandle = open(fname, O_RDONLY | O_BINARY);
    if (maphandle == -1)
        CA_CannotOpen(fname);
#else
    strcpy(fname,mfilename);
    strcat(fname,extension);

    maphandle = open(fname, O_RDONLY | O_BINARY);
    if (maphandle == -1)
        CA_CannotOpen(fname);
#endif

//
// load all map header
//
    for (i=0;i<NUMMAPS;i++)
    {
        pos = tinf->headeroffsets[i];
        if (pos<0)                          // $FFFFFFFF start is a sparse map
            continue;

        mapheaderseg[i] = SafeMalloc(sizeof(*mapheaderseg[i]));

        lseek(maphandle,pos,SEEK_SET);
        read (maphandle,mapheaderseg[i],sizeof(*mapheaderseg[i]));
    }

//
// allocate space for 3 64*64 planes
//
    for (i = 0; i < MAPPLANES; i++)
    {
        mapsegs[i] = SafeMalloc(MAPAREA * sizeof(*mapsegs[i]));
    }
}


//==========================================================================

#ifdef VIEASM

#else
/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
#if defined SWITCH && defined N3DS
    char fname[13 + sizeof(DATADIR)];
#else    
    char fname[13];
#endif
//
// load audiohed.ext (offsets for audio file)
//
    strcpy(fname,aheadname);
    strcat(fname,audioext);

    void* ptr;
    if (!CA_LoadFile(fname, &ptr))
        CA_CannotOpen(fname);
    audiostarts = (int32_t*)ptr;

//
// open the data file
//
    strcpy(fname,afilename);
    strcat(fname,audioext);

    audiohandle = open(fname, O_RDONLY | O_BINARY);
    if (audiohandle == -1)
        CA_CannotOpen(fname);
}
#endif
//==========================================================================


/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
#ifdef PROFILE
    unlink ("PROFILE.TXT");
    profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif

#if defined SWITCH && defined N3DS
    printf("CA_INIT\n");
#endif   
    CAL_SetupMapFile ();
#if defined SWITCH && defined N3DS   
    printf("CAL_SetupMapFile ();\n");
#endif    
    CAL_SetupGrFile ();
#if defined SWITCH && defined N3DS    
    printf("CAL_SetupGrFile ();\n");
#endif    
#ifdef VIEASM

#else
    CAL_SetupAudioFile ();
#endif
#if defined SWITCH && defined N3DS
    printf("CAL_SetupAudioFile ();\n");
#endif
}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
    int i,start;

    if (maphandle != -1)
        close(maphandle);
    
#ifdef VIEASM
    
#else
    if (audiohandle != -1)
        close(audiohandle);
#endif
    for (i=0; i<NUMCHUNKS; i++)
    {
        free (grsegs[i]);
        grsegs[i] = NULL;
    }

    free (pictable);
    pictable = NULL;

    for (i = 0; i < NUMMAPS; i++)
    {
        free (mapheaderseg[i]);
        mapheaderseg[i] = NULL;
    }

    for (i = 0; i < MAPPLANES; i++)
    {
        free (mapsegs[i]);
        mapsegs[i] = NULL;
    }

    free (tinf);
    tinf = NULL;
#ifdef VIEASM

#else
    switch(oldsoundmode)
    {
        case sdm_Off:
            return;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    for(i=0; i<NUMSOUNDS; i++,start++)
        UNCACHEAUDIOCHUNK(start);
#endif
}

//===========================================================================

#ifdef VIEASM

#else
/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

int32_t CA_CacheAudioChunk (int chunk)
{
    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;

    if (audiosegs[chunk])
        return size;                        // already in memory

    audiosegs[chunk] = SafeMalloc(size);

    lseek(audiohandle,pos,SEEK_SET);
    read(audiohandle,audiosegs[chunk],size);

    return size;
}

void CA_CacheAdlibSoundChunk (int chunk)
{
    byte    *bufferseg;
    byte    *ptr;
    int32_t pos = audiostarts[chunk];
    int32_t size = audiostarts[chunk+1]-pos;

    if (audiosegs[chunk])
        return;                        // already in memory

    lseek(audiohandle, pos, SEEK_SET);

    bufferseg = SafeMalloc(ORIG_ADLIBSOUND_SIZE - 1);
    ptr = bufferseg;

    read(audiohandle, ptr, ORIG_ADLIBSOUND_SIZE - 1);   // without data[1]

    AdLibSound *sound = SafeMalloc(size + sizeof(*sound) - ORIG_ADLIBSOUND_SIZE);

    sound->common.length = READLONGWORD(ptr);
    ptr += 4;

    sound->common.priority = READWORD(ptr);
    ptr += 2;

    sound->inst.mChar = *ptr++;
    sound->inst.cChar = *ptr++;
    sound->inst.mScale = *ptr++;
    sound->inst.cScale = *ptr++;
    sound->inst.mAttack = *ptr++;
    sound->inst.cAttack = *ptr++;
    sound->inst.mSus = *ptr++;
    sound->inst.cSus = *ptr++;
    sound->inst.mWave = *ptr++;
    sound->inst.cWave = *ptr++;
    sound->inst.nConn = *ptr++;
    sound->inst.voice = *ptr++;
    sound->inst.mode = *ptr++;
    sound->inst.unused[0] = *ptr++;
    sound->inst.unused[1] = *ptr++;
    sound->inst.unused[2] = *ptr++;
    sound->block = *ptr++;

    read(audiohandle, sound->data, size - ORIG_ADLIBSOUND_SIZE + 1);  // + 1 because of byte data[1]

    audiosegs[chunk]=(byte *) sound;

    free (bufferseg);
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds (void)
{
    unsigned start,i;

    switch (oldsoundmode)
    {
        case sdm_Off:
            goto cachein;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    for (i=0;i<NUMSOUNDS;i++,start++)
        UNCACHEAUDIOCHUNK(start);

cachein:

    oldsoundmode = SoundMode;

    switch (SoundMode)
    {
        case sdm_Off:
            start = STARTADLIBSOUNDS;   // needed for priorities...
            break;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    if(start == STARTADLIBSOUNDS)
    {
        for (i=0;i<NUMSOUNDS;i++,start++)
            CA_CacheAdlibSoundChunk(start);
    }
    else
    {
        for (i=0;i<NUMSOUNDS;i++,start++)
            CA_CacheAudioChunk(start);
    }
}
#endif
//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (int chunk, int32_t *source)
{
    int32_t    expanded;

    if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
    {
        //
        // expanded sizes of tile8/16/32 are implicit
        //

#define BLOCK           64
#define MASKBLOCK       128

        if (chunk<STARTTILE8M)          // tile 8s are all in one chunk!
            expanded = BLOCK*NUMTILE8;
        else if (chunk<STARTTILE16)
            expanded = MASKBLOCK*NUMTILE8M;
        else if (chunk<STARTTILE16M)    // all other tiles are one/chunk
            expanded = BLOCK*4;
        else if (chunk<STARTTILE32)
            expanded = MASKBLOCK*4;
        else if (chunk<STARTTILE32M)
            expanded = BLOCK*16;
        else
            expanded = MASKBLOCK*16;
    }
    else
    {
        //
        // everything else has an explicit size longword
        //
        expanded = *source++;
    }

    //
    // allocate final space and decompress it
    //
    grsegs[chunk] = SafeMalloc(expanded);

    CAL_HuffExpand((byte *) source, grsegs[chunk], expanded, grhuffman);
}


/*
======================
=
= CAL_DeplaneGrChunk
=
======================
*/

void CAL_DeplaneGrChunk (int chunk)
{
    int     i;
    int16_t width,height;

    if (chunk == STARTTILE8)
    {
        width = height = 8;

        for (i = 0; i < NUMTILE8; i++)
            VL_DePlaneVGA (grsegs[chunk] + (i * (width * height)),width,height);
    }
    else
    {
        width = pictable[chunk - STARTPICS].width;
        height = pictable[chunk - STARTPICS].height;

        VL_DePlaneVGA (grsegs[chunk],width,height);
    }
}


/*
======================
=
= CA_CacheGrChunks
=
= Load all graphics chunks into memory
=
======================
*/

void CA_CacheGrChunks (void)
{
    int32_t pos,compressed;
    int32_t *bufferseg;
    int32_t *source;
    int     chunk,next;

    for (chunk = STRUCTPIC + 1; chunk < NUMCHUNKS; chunk++)
    {
        if (grsegs[chunk])
            continue;                             // already in memory

        //
        // load the chunk into a buffer
        //
        pos = GRFILEPOS(chunk);

        if (pos<0)                              // $FFFFFFFF start is a sparse tile
            continue;

        next = chunk + 1;

        while (GRFILEPOS(next) == -1)           // skip past any sparse tiles
            next++;

        compressed = GRFILEPOS(next)-pos;

        lseek(grhandle,pos,SEEK_SET);

        bufferseg = SafeMalloc(compressed);
        source = bufferseg;

        read(grhandle,source,compressed);

        CAL_ExpandGrChunk (chunk,source);

        if (chunk >= STARTPICS && chunk < STARTEXTERNS)
            CAL_DeplaneGrChunk (chunk);

        free(bufferseg);
    }
}



//==========================================================================


/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/

void CA_CacheMap (int mapnum)
{
    int32_t  pos,compressed;
    int      plane;
    word     *dest;
    unsigned size;
    word     *bufferseg;
    word     *source;
#ifdef CARMACIZED
    word     *buffer2seg;
    int32_t  expanded;
#endif

    if (mapheaderseg[mapnum]->width != MAPSIZE || mapheaderseg[mapnum]->height != MAPSIZE)
        Quit ("CA_CacheMap: Map not %u*%u!",MAPSIZE,MAPSIZE);

//
// load the planes into the allready allocated buffers
//
    size = MAPAREA * sizeof(*dest);

    for (plane = 0; plane<MAPPLANES; plane++)
    {
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];

        dest = mapsegs[plane];

        lseek(maphandle,pos,SEEK_SET);

        bufferseg = SafeMalloc(compressed);
        source = bufferseg;

        read(maphandle,source,compressed);
#ifdef CARMACIZED
        //
        // unhuffman, then unRLEW
        // The huffman'd chunk has a two byte expanded length first
        // The resulting RLEW chunk also does, even though it's not really
        // needed
        //
        expanded = *source;
        source++;
        buffer2seg = SafeMalloc(expanded);
        CAL_CarmackExpand((byte *) source, buffer2seg,expanded);
        CA_RLEWexpand(buffer2seg+1,dest,size,tinf->RLEWtag);
        free(buffer2seg);

#else
        //
        // unRLEW, skipping expanded length
        //
        CA_RLEWexpand (source+1,dest,size,tinf->RLEWtag);
#endif
        free(bufferseg);
    }
}

//===========================================================================

void CA_CannotOpen(const char *string)
{
    char str[30];

    strcpy(str,"Can't open ");
    strcat(str,string);
    strcat(str,"!\n");
    Quit (str);
}
