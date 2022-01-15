#ifndef SOSM_H_
#define SOSM_H_
#include "sos.h"
#define _MIDI_MAP_TRACK 0xff

#pragma pack(4)
typedef struct { WORD wTrackDevice[32]; } _SOS_MIDI_TRACK_DEVICE;
typedef struct {  
 WORD wDIGIDriverID;
 VOID far * lpDriverMemory;
 VOID far * lpDriverMemoryCS;
 void far * sDIGIInitInfo;
 WORD  wParam;
 DWORD dwParam;
} _SOS_MIDI_INIT_DRIVER;

typedef struct {
 WORD  wPort;
 WORD  wIRQ;
 WORD  wParam;
} _SOS_MIDI_HARDWARE;

#define _MIDI_FM 0xa002
#define _MIDI_GUS 0xa00a
#define _MIDI_OPL3 0xa009

typedef struct {  
 BYTE _huge *lpSongData;
 VOID (far *lpSongCallback)(WORD);
} _SOS_MIDI_INIT_SONG;

#define _SOS_MIDI_MAX_TRACKS 32

typedef struct {
   BYTE  bMidiData[256];
} _NDMF_MIDI_EVENT;
#pragma pack()

extern _NDMF_MIDI_EVENT _huge *_lpSOSMIDITrack[][_SOS_MIDI_MAX_TRACKS];

WORD  sosMIDIInitSystem(LPSTR, WORD);
WORD  sosMIDIUnInitSystem(VOID);

WORD  sosMIDIInitDriver(WORD, _SOS_MIDI_HARDWARE far *, _SOS_MIDI_INIT_DRIVER far *, WORD far *);
WORD  sosMIDIUnInitDriver(WORD, BOOL);

WORD  sosMIDIInitSong(_SOS_MIDI_INIT_SONG far *, _SOS_MIDI_TRACK_DEVICE far *, WORD far *);
WORD  sosMIDIUnInitSong(WORD);
WORD  sosMIDIStartSong(WORD);
WORD  sosMIDIStopSong(WORD);
WORD  sosMIDISetInsData(WORD, LPSTR, WORD);
WORD  sosMIDIResetDriver(WORD);

WORD sosMIDIEnableChannelStealing(WORD);
WORD sosMIDISetMasterVolume(BYTE);


#endif
