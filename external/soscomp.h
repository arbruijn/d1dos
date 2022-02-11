#ifndef _SOSCODEC_DEFINED
#define _SOSCODEC_DEFINED

typedef struct _SOS_COMPRESS_INFO {
	char *lpSource;
	char *lpDest;

	unsigned long dwCompSize;
	unsigned long dwUnCompSize;

	unsigned long dwSampleIndex;
	long dwPredicted;
	long dwDifference;

	short wCodeBuf;
	short wCode;
	short wStep;
	short wIndex;

	short wBitSize;
} _SOS_COMPRESS_INFO;

void sosCODECInitStream(_SOS_COMPRESS_INFO *sSOSInfo);
unsigned long sosCODECDecompressData(_SOS_COMPRESS_INFO *sSOSInfo, unsigned long wBytes);
unsigned long sosCODECCompressData(_SOS_COMPRESS_INFO *sSOSInfo, unsigned long wBytes);

#endif
