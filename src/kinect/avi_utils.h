#ifndef _avi_utils_H
#define _avi_utils_H

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vfw.h>

// First, we'll define the WAV file format.
#include <pshpack1.h>
typedef struct
{ char id[4];         //="fmt "
unsigned long size; //=16
short wFormatTag;   //=WAVE_FORMAT_PCM=1
unsigned short wChannels;       //=1 or 2 for mono or stereo
unsigned long dwSamplesPerSec;  //=11025 or 22050 or 44100
unsigned long dwAvgBytesPerSec; //=wBlockAlign * dwSamplesPerSec
unsigned short wBlockAlign;     //=wChannels * (wBitsPerSample==8?1:2)
unsigned short wBitsPerSample;  //=8 or 16, for bits per sample
} FmtChunk;

typedef struct
{ char id[4];            //="data"
unsigned long size;    //=datsize, size of the following array
unsigned char data[1]; //=the raw data goes here
} DataChunk;

typedef struct
{ char id[4];         //="RIFF"
unsigned long size; //=datsize+8+16+4
char type[4];       //="WAVE"
FmtChunk fmt;
DataChunk dat;
} WavChunk;
#include <poppack.h>




// This is the internal structure represented by the HAVI handle:
typedef struct
{ IAVIFile *pfile;    // created by CreateAvi
WAVEFORMATEX wfx;   // as given to CreateAvi (.nChanels=0 if none was given). Used when audio stream is first created.
int period;         // specified in CreateAvi, used when the video stream is first created
IAVIStream *as;     // audio stream, initialised when audio stream is first created
IAVIStream *ps, *psCompressed;  // video stream, when first created
unsigned long nframe, nsamp;    // which frame will be added next, which sample will be added next
bool iserr;         // if true, then no function will do anything
} TAviUtil;

// AVI utilities -- for creating avi files
// (c) 2002 Lucian Wischik. No restrictions on use.


DECLARE_HANDLE(HAVI);
// An HAVI identifies an avi file that is being created


HAVI CreateAvi(const char *fn, int frameperiod, const WAVEFORMATEX *wfx);
// CreateAvi - call this to start the creation of the avi file.
// The period is the number of ms between each bitmap frame.
// The waveformat can be null if you're not going to add any audio,
// or if you're going to add audio from a file.

HRESULT AddAviFrame(HAVI avi, HBITMAP hbm);
HRESULT AddAviAudio(HAVI avi, void *dat, unsigned long numbytes);
// AddAviFrame - adds this bitmap to the avi file. hbm must point be a DIBSection.
// It is the callers responsibility to free the hbm.
// AddAviAudio - adds this junk of audio. The format of audio was as specified in the
// wfx parameter to CreateAVI. This fails if NULL was given.
// Both return S_OK if okay, otherwise one of the AVI errors.

HRESULT AddAviWav(HAVI avi, const char *wav, DWORD flags);
// AddAviWav - a convenient way to add an entire wave file to the avi.
// The wav file may be in in memory (in which case flags=SND_MEMORY)
// or a file on disk (in which case flags=SND_FILENAME).
// This function requires that either a null WAVEFORMATEX was passed to CreateAvi,
// or that the wave file now being added has the same format as was
// added earlier.

HRESULT SetAviVideoCompression(HAVI avi, HBITMAP hbm, AVICOMPRESSOPTIONS *opts, bool ShowDialog, HWND hparent);
// SetAviVideoCompression - allows compression of the video. If compression is desired,
// then this function must have been called before any bitmap frames had been added.
// The bitmap hbm must be a DIBSection (so that avi knows what format/size you're giving it),
// but won't actually be added to the movie.
// This function can display a dialog box to let the user choose compression. In this case,
// set ShowDialog to true and specify the parent window. If opts is non-NULL and its
// dwFlags property includes AVICOMPRESSF_VALID, then opts will be used to give initial
// values to the dialog. If opts is non-NULL then the chosen options will be placed in it.
// This function can also be used to choose a compression without a dialog box. In this
// case, set ShowDialog to false, and hparent is ignored, and the compression specified
// in 'opts' is used, and there's no need to call GotAviVideoCompression afterwards.

HRESULT CloseAvi(HAVI avi);
// CloseAvi - the avi must be closed with this message.



unsigned int FormatAviMessage(HRESULT code, char *buf,unsigned int len);
// FormatAviMessage - given an error code, formats it as a string.
// It returns the length of the error message. If buf/len points
// to a real buffer, then it also writes as much as possible into there.


#endif