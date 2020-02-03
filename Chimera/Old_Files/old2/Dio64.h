#ifndef DIO64_H
#define DIO64_H

/* DIO64.H

   Viewpoint Systems, Inc.
   800 West Metro Parkway.
   Rochester, New York    14623
   (585) 475-9555
   viewpointusa.com

   Copyright (c) 2002
   All Rights Reserved
*/

#define DLLEXPORT __declspec(dllexport) __stdcall

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

#define DIO64_CLCK_INTERNAL 0
#define DIO64_CLCK_EXTERNAL 1
#define DIO64_CLCK_TRIG_0   2
#define DIO64_CLCK_OCXO			3

#define DIO64_STRT_NONE     0
#define DIO64_STRT_EXTERNAL 1
#define DIO64_STRT_TRIG_2   2
#define DIO64_STRT_PXI_STAR 3

#define DIO64_STRTTYPE_LEVEL      0
#define DIO64_STRTTYPE_EDGETOEDGE 2
#define DIO64_STRTTYPE_EDGE       4


#define DIO64_STOP_NONE					0
#define DIO64_STOP_EXTERNAL			1
#define DIO64_STOP_TRIG_3_IN		2
#define DIO64_STOP_OUTPUT_FIFO	3

#define DIO64_STOPTYPE_EDGE     0


#define DIO64_TRIG_RISING	 0
#define DIO64_TRIG_FALLING 1

#define DIO64_AI_NONE 0



#pragma pack(push, 1)
typedef struct _DIO64STAT {
	 USHORT pktsize;
   USHORT portCount;
   USHORT writePtr;
   USHORT readPtr;
   USHORT time[2];
   ULONG	fifoSize;

   USHORT fifo0;
   ULONG  ticks;
   USHORT flags;
   USHORT clkControl;
   USHORT startControl;
   USHORT stopControl;
   ULONG	AIControl;
   USHORT AICurrent;
   USHORT startTime[2];
   USHORT stopTime[2];
	 USHORT user[4];
} DIO64STAT;
#pragma pack(pop)

#define DIO64_ATTR_INPUTMODE									0
#define DIO64_ATTR_OUTPUTMODE									1
#define DIO64_ATTR_INPUTBUFFERSIZE						2
#define DIO64_ATTR_OUTPUTBUFFERSIZE						3
#define DIO64_ATTR_MAJORCLOCKSOURCE						4
#define DIO64_ATTR_INPUTTHRESHOLD							5
#define DIO64_ATTR_OUTPUTTHRESHOLD						6
#define DIO64_ATTR_INPUTTIMEOUT								7
#define DIO64_ATTR_RTSIGLOBALENABLE						8
#define DIO64_ATTR_RTSICLKSOURCE							9
#define DIO64_ATTR_RTSICLKTRIG7ENABLE					10
#define DIO64_ATTR_EXTERNALCLKENABLE					11
#define DIO64_ATTR_PXICLKENABLE								12
#define DIO64_ATTR_RTSISCANCLKTRIG0ENABLE			13
#define DIO64_ATTR_RTSISTARTTRIG2ENABLE				14
#define DIO64_ATTR_RTSISTOPTRIG3ENABLE				15
#define DIO64_ATTR_RTSIMODSCANCLKTRIG4ENABLE	16
#define DIO64_ATTR_PXISTARENABLE							17
#define DIO64_ATTR_PORTROUTING								18
#define DIO64_ATTR_STATICOUTPUTMASK 					19
#define DIO64_ATTR_SERIALNUMBER								20
#define DIO64_ATTR_ARMREENABLE								21
#define DIO64_ATTR_SCLKENABLE									22
#define DIO64_ATTR_FPGAINFO										23


#define DIO64_ERR_ILLEGALBOARD								-8
#define DIO64_ERR_BOARDNOTOPENED							-9
#define DIO64_ERR_STATUSOVERRUNUNDERRUN				-10
#define DIO64_ERR_INVALIDPARAMETER						-12
#define DIO64_ERR_NODRIVERINTERFACE						-13
#define DIO64_ERR_OCXOOPTIONNA								-14
#define DIO64_ERR_PXIONLYSIGNALS							-15
#define DIO64_ERR_STOPTRIGSRCINVALID					-16
#define DIO64_ERR_PORTNUMBERCONFLICTS					-17
#define DIO64_ERR_MISSINGDIO64CATFILE					-18
#define DIO64_ERR_NOTENOUGHRESOURCES					-19
#define DIO64_ERR_INVALIDSIGNITUREDIO64CAT		-20
#define DIO64_ERR_REQUIREDIMAGENOTFOUND				-21
#define DIO64_ERR_ERRORPROGFPGA								-22
#define DIO64_ERR_FILENOTFOUND								-23
#define DIO64_ERR_BOARDERROR									-24
#define DIO64_ERR_FUNCTIONCALLINVALID					-25
#define DIO64_ERR_NOTENOUGHTRANS							-26


int DLLEXPORT DIO64_Open(WORD board, WORD baseio);
int DLLEXPORT DIO64_Mode(WORD board, WORD mode);
int DLLEXPORT DIO64_Load(WORD board, char *rbfFile, int intputHint, int outputHint );
int DLLEXPORT DIO64_Close(WORD board);

int DLLEXPORT DIO64_In_Start(	WORD board,
															DWORD ticks,
															WORD *mask,
															WORD maskLength,
															WORD flags,
															WORD clkControl,
															WORD startType,
															WORD startSource,
															WORD stopType,
															WORD stopSource,
															DWORD AIControl,
															double *scanRate );

int DLLEXPORT DIO64_In_Status(WORD board, DWORD *scansAvail, DIO64STAT *status);
int DLLEXPORT DIO64_In_Read(WORD board, WORD *buffer, DWORD scansToRead, DIO64STAT *status);

int DLLEXPORT DIO64_In_Stop(WORD board);

int DLLEXPORT DIO64_Out_ForceOutput( WORD board, WORD *buffer, DWORD mask );
int DLLEXPORT DIO64_Out_GetInput( WORD board, WORD *buffer );
int DLLEXPORT DIO64_Out_Config( WORD board,
                            DWORD ticks,
                            WORD *mask,
                            WORD maskLength,
                            WORD flags,
                            WORD clkControl,
                            WORD startType,
														WORD startSource,
                            WORD stopType,
														WORD stopSource,
														DWORD AIControl,
														DWORD reps,
														WORD ntrans,
														double *scanRate

													);
int DLLEXPORT DIO64_Out_Start( WORD board );

int DLLEXPORT DIO64_Out_Status(
														WORD board,
														DWORD *scansAvail,
														DIO64STAT *status);

int DLLEXPORT DIO64_Out_Write(WORD board, WORD *buffer, DWORD bufsize, DIO64STAT *status);
int DLLEXPORT DIO64_Out_Stop(WORD board);

int DLLEXPORT DIO64_SetAttr(WORD board, DWORD attrID, DWORD value);
int DLLEXPORT DIO64_GetAttr(WORD board, DWORD *attrID, DWORD *value);

#endif
