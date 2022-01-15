#ifndef SOS_H_
#define SOS_H_

enum { _FALSE, _TRUE };
#define _NULL 0
#define VOID void
typedef int BOOL;
typedef unsigned WORD;
typedef long LONG;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef BYTE *PSTR;
typedef BYTE far *LPSTR;

#define _SOUND_BLASTER_8_MONO 0xe000
#define _SOUND_BLASTER_8_ST 0xe001
#define _MICROSOFT_8_ST 0xe00c
#define _MICROSOFT_16_ST 0xe00e
#define _SB16_8_ST 0xe016
#define _SB16_16_ST 0xe018
#define _GUS_8_ST 0xe024
#define _GUS_16_ST 0xe026
#define _RAP10_8_MONO 0xe021

#define _SOS_DEBUG_NORMAL 0x0000
#define _SOS_DEBUG_NO_TIMER 0x0001

#define  _MAX_VOICES 32

#pragma pack(4)
typedef struct {  
 WORD wBufferSize;
 LPSTR lpBuffer;
 BOOL wAllocateBuffer;
 WORD wSampleRate;
 WORD wParam;
 LONG dwParam;
 VOID ( far *lpFillHandler )( VOID );
 LPSTR lpDriverMemory;
 LPSTR lpDriverMemoryCS;
 LPSTR lpTimerMemory;
 LPSTR lpTimerMemoryCS;
 WORD  wTimerID;
 WORD wPhysical;
} _SOS_INIT_DRIVER;

typedef struct {
 WORD wPort;
 WORD wIRQ;
 WORD wDMA; 
 WORD wParam;
} _SOS_HARDWARE;

typedef struct {
   LPSTR lpSamplePtr;
   WORD dwSampleSize;
   WORD wLoopCount;
   WORD wChannel;
   WORD wVolume;
   WORD wSampleID;
   VOID (far cdecl *lpCallback )( WORD, WORD, WORD );
   WORD wSamplePort; 
   WORD wSampleFlags;
   WORD dwSampleByteLength;
   WORD dwSampleLoopPoint;
   WORD dwSampleLoopLength;
   WORD dwSamplePitchAdd;
   WORD wSamplePitchFraction;
   WORD wSamplePanLocation;
   WORD wSamplePanSpeed;
   WORD wSamplePanDirection;
   WORD wSamplePanStart;
   WORD wSamplePanEnd;
   WORD wSampleDelayBytes;
   WORD wSampleDelayRepeat;
   WORD dwSampleADPCMPredicted;
   WORD wSampleADPCMIndex;
   WORD wSampleRootNoteMIDI;   
   WORD dwSampleTemp1;   
   WORD dwSampleTemp2;   
   WORD dwSampleTemp3;   
} _SOS_START_SAMPLE;
#pragma pack()

enum { _LEFT_CHANNEL, _RIGHT_CHANNEL, _CENTER_CHANNEL, _INTERLEAVED };

#define _ACTIVE 0x8000
#define _LOOPING 0x4000
#define _FIRST_TIME 0x2000
#define _PENDING_RELEASE 0x1000
#define _CONTINUE_BLOCK 0x0800
#define _PITCH_SHIFT 0x0400
#define _PANNING 0x0200
#define _VOLUME 0x0100
#define _STAGE_LOOP 0x0040
#define _TRANSLATE8TO16 0x0020

VOID far sosTIMERHandler(VOID);
WORD sosTIMERRegisterEvent(WORD, VOID(far *)(VOID), WORD far *);
WORD sosTIMERRemoveEvent(WORD);
WORD sosTIMERInitSystem(WORD, WORD);
WORD sosTIMERUnInitSystem(WORD);

WORD sosDIGIInitSystem(LPSTR, WORD);
WORD sosDIGIUnInitSystem(VOID);
WORD sosDIGIInitDriver(WORD, _SOS_HARDWARE far *, _SOS_INIT_DRIVER far *, WORD far *);
WORD sosDIGIUnInitDriver(WORD, BOOL, BOOL);

WORD sosDIGIStartSample(WORD, _SOS_START_SAMPLE far *);
WORD sosDIGIStopSample(WORD, WORD);
BOOL sosDIGISampleDone(WORD, WORD);

PSTR  sosGetErrorString(WORD);

WORD sosDIGISetSampleVolume(WORD, WORD, WORD);
WORD sosDIGISetPanLocation(WORD, WORD, WORD);
WORD sosDIGIGetSampleHandle(WORD, WORD);

#define _ERR_NO_SLOTS (WORD)-1

// workaround for watcom quirk where global var order depends on number of declarations
void sfill00(void);
void sfill01(void);
void sfill02(void);
void sfill03(void);
void sfill04(void);
void sfill05(void);
void sfill06(void);
void sfill07(void);
void sfill08(void);
void sfill09(void);
void sfill0a(void);
void sfill0b(void);
void sfill0c(void);
void sfill0d(void);
void sfill0e(void);
void sfill0f(void);

#endif
