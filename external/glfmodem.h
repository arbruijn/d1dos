#include "commlib.h"
int HMSendString(PORT*,char*);
int HMSendStringNoWait(PORT*,char*,int);
void HMWaitForOK(long,char*);
long HMInputLine(PORT*,long,char*,int);
int HMReset(PORT *);
int HMAnswer(PORT *);
int HMDial(PORT*,char*);
