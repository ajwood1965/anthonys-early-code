#include "exec/types.h"
#include "exec/exec.h"
#include <libraries/dosextens.h>
#include "intuition/intuition.h"
#include "devices/audio.h"
#include "prog:include/smpte.h"
#include <hardware/intbits.h>
#include "graphics/gfxbase.h"
#include "/include/psound.h"
/*�������������������������������������������������������������������*/
#define PERIOD 0x2c0
#define GOFF  1234562
#define GON   433214
#define GQUIT 6311314
/*�������������������������������������������������������������������*/
extern struct SmpteSource *SMPTEsrc;
extern struct MenuItem Items[];
/*�������������������������������������������������������������������*/
void FigurePeriod();
void RemInt();
/*�������������������������������������������������������������������*/
void __asm FillTimeCodeBuffer(register __a0 BYTE *,register __d0 ULONG);
/*�������������������������������������������������������������������*/
struct IOAudio sound1,sound2;
BYTE           *buff1=NULL,*buff2=NULL;
struct MsgPort *port1=NULL,*port2=NULL;
UBYTE whichannel[]={2,8};
int clock;
/*�������������������������������������������������������������������*/
extern struct StudioBase *StudioBase;
/*�������������������������������������������������������������������*/
int OUTFLAG=0xFFFF;
/*�������������������������������������������������������������������*/
extern int GENERATING;
ULONG tc_t;
extern struct Task *task;
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
void OutputTimeCode(tc)
ULONG tc;
{
void OutputTimeCodeTask();
tc_t=tc;
if (GENERATING==GON) {return;}
while(GENERATING!=GOFF) WaitTOF();
GENERATING=GON;
CreateTask("STUDIO_SMPTE_OUTPUT_TASK",75,OutputTimeCodeTask,4000);
}
/*�������������������������������������������������������������������*/
extern struct NewButton StopBut;
extern struct NewButton PlayBut;
extern struct NewButton RewBut;
extern struct NewButton FFBut;
/*�������������������������������������������������������������������*/
int curtc,tcadd;
int zzz;
/*�������������������������������������������������������������������*/
void OutputTimeCodeTask()
{
ULONG tc=tc_t,ltc=tc;
int OpenAudDevice();
void CloseAudDevice();
UBYTE *pa;
void SmpteData();
extern ULONG draw_sigmask;
int lzzz=zzz,tzzz;
/*int vbp;
int xxx=0;*/

if (OpenAudDevice())
	{
	pa=(UBYTE *)0xbfe001;
	*pa=*pa|2;

	if (tcadd!=1)
		{
		sound1.ioa_Volume=0;
		sound2.ioa_Volume=0;
		}

	FillTimeCodeBuffer(buff1,tc);
	curtc=ltc;ltc=tc;tc=AddTimeCode(tc,tcadd,StudioBase->defaults.smpte_mode);
	FillTimeCodeBuffer(buff2,tc);

	FigurePeriod();

	WaitTOF();
	BeginIO(&sound1);
	Signal(task,draw_sigmask);
	BeginIO(&sound2);
	if (OUTFLAG&2) InjectTimeCode(SMPTEsrc,curtc);
	while (GENERATING==GON)
		{
		curtc=ltc;ltc=tc;tc=AddTimeCode(tc,tcadd,StudioBase->defaults.smpte_mode);
		Wait(1<<port1->mp_SigBit);
		Signal(task,draw_sigmask);
		while(GetMsg(port1));
		FillTimeCodeBuffer(buff1,tc);
		BeginIO(&sound1);
		if (OUTFLAG&2) InjectTimeCode(SMPTEsrc,curtc);

		curtc=ltc;ltc=tc;tc=AddTimeCode(tc,tcadd,StudioBase->defaults.smpte_mode);
		Wait(1<<port2->mp_SigBit);
/*		vbp=VBeamPos();*/

		if (Items[11].Flags&CHECKED)
			{
			tzzz=zzz;
			if ( tzzz != (lzzz+4) )
				{
				if ( tzzz > (lzzz+4) ) {sound1.ioa_Period--;sound2.ioa_Period--;}
				else {sound1.ioa_Period++;sound2.ioa_Period++;}
				}
			lzzz=tzzz;
			}
/*		xxx++;
		if ((xxx&7)==0) kprintf("%d%d\n",vbp);*/
		Signal(task,draw_sigmask);
		while(GetMsg(port2));
		FillTimeCodeBuffer(buff2,tc);
		BeginIO(&sound2);
		if (OUTFLAG&2) InjectTimeCode(SMPTEsrc,curtc);
		}
	*pa=*pa&~2;
	}
CloseAudDevice();
GENERATING=GOFF;
}
/*�������������������������������������������������������������������*/
int OpenAudDevice()
{
ULONG device;
void FigurePeriod();

setmem(&sound1,sizeof(struct IOAudio),0);
setmem(&sound2,sizeof(struct IOAudio),0);

port1=(struct MsgPort *)CreatePort(0,0);
port2=(struct MsgPort *)CreatePort(0,0);

if (!port1 || !port2) {telluser("Smpte Output:","Could not create MsgPorts",NULL);return(0);}

buff1=(BYTE *)AllocMem(80*2+1,MEMF_PUBLIC|MEMF_CHIP|MEMF_CLEAR);
buff2=(BYTE *)AllocMem(80*2+1,MEMF_PUBLIC|MEMF_CHIP|MEMF_CLEAR);
if (!buff1 || !buff2) {telluser("Smpte Output:","Could not alloc buffer mem",NULL);return(0);}

sound1.ioa_Request.io_Message.mn_ReplyPort=port1;
sound1.ioa_Request.io_Message.mn_Node.ln_Pri=127;
sound1.ioa_Request.io_Command=ADCMD_ALLOCATE;
sound1.ioa_Request.io_Flags=/*ADIOF_NOWAIT*/NULL;
sound1.ioa_AllocKey=0;
sound1.ioa_Data=whichannel;
sound1.ioa_Length=sizeof(whichannel);

device=OpenDevice(AUDIONAME,0L,&sound1,0L);
if (0!=device) {telluser("Smpte Output:","Could not open device",NULL);return(0);}

sound1.ioa_Cycles=1;
sound1.ioa_Length=80*2;
sound1.ioa_Period=PERIOD;
sound1.ioa_Volume=63;
sound1.ioa_Request.io_Command=CMD_WRITE;
sound1.ioa_Request.io_Flags=ADIOF_PERVOL/*|IOF_QUICK*/;

sound2=sound1;

sound2.ioa_Request.io_Message.mn_ReplyPort=port1;
sound2.ioa_Request.io_Message.mn_ReplyPort=port2;
sound2.ioa_Data=buff2;
sound1.ioa_Data=buff1;

if (AddMyInt()==0) {telluser("SMPTE Output:","Add Interupt Failed",NULL);}

FigurePeriod();

return(1);
}
/*�������������������������������������������������������������������*/
void CloseAudDevice()
{
Forbid();

sound1.ioa_Request.io_Command=ADCMD_FINISH;
sound1.ioa_Request.io_Flags=IOF_QUICK;
BeginIO(&sound1);
WaitIO(&sound1);
if(port1!=NULL) {DeletePort(port1);port1=NULL;}

sound2.ioa_Request.io_Command=ADCMD_FINISH;
sound2.ioa_Request.io_Flags=IOF_QUICK;
BeginIO(&sound2);
WaitIO(&sound2);
if(port2!=NULL) {DeletePort(port2);port2=NULL;}
CloseDevice(&sound1);

if(buff1) {FreeMem(buff1,80*2+1);buff1=NULL;}
if(buff2) {FreeMem(buff2,80*2+1);buff2=NULL;}

RemInt();

Permit();
}
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
struct Interrupt *smpte_int;			/* VertBlank Smpte timer */
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
int SmpteTimer()
{
zzz=zzz+1;
return(0);
}
/*�������������������������������������������������������������������*/
AddMyInt()
{
int SmpteTimer();
extern struct Interrupt *MakeInt();

zzz=0;

smpte_int = MakeInt("SmpteGenerator",127,SmpteTimer,NULL);
if (smpte_int==0) return(0);
AddIntServer(INTB_VERTB, smpte_int); 
return(1);
}
/*�������������������������������������������������������������������*/
void RemInt()
{
void FreeInt();

if (smpte_int) 
	{
	RemIntServer(INTB_VERTB,smpte_int);
	FreeInt(smpte_int);
	}
}
/*�������������������������������������������������������������������*/
struct Interupt *MakeInt(name,pri,code,data)
char *name;
int pri;
void (*code)();
APTR data;
{
struct Interrupt *intr;

intr=(struct Interrupt *)AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR);
if (intr==0) return(0);

intr->is_Node.ln_Pri=pri;
intr->is_Node.ln_Type=NT_INTERRUPT;
intr->is_Node.ln_Name=name;
intr->is_Data = data;
intr->is_Code = code;

return(intr);
}
/*�������������������������������������������������������������������*/
void FreeInt(intr)
struct Interrupt *intr;
{
if (intr==0) return;
if (intr->is_Node.ln_Type != NT_INTERRUPT) return;
intr->is_Node.ln_Type=0;
FreeMem(intr,sizeof(struct Interrupt));
}
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
#include <devices/timer.h>
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
void FigurePeriod()
{
extern struct GfxBase *GfxBase;
struct timerequest tr;
struct MsgPort	*tport;
int x,szzz,ezzz;
int Smic,Ssec,Emic,Esec,mic;

if (OpenDevice(TIMERNAME,UNIT_MICROHZ,(struct IORequest *)&tr,0)) exit(10);
if (!(tport=(struct MsgPort *)CreatePort(0,0))) exit(11);
tr.tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
tr.tr_node.io_Message.mn_Node.ln_Pri = 0;
tr.tr_node.io_Message.mn_Node.ln_Name = NULL;
tr.tr_node.io_Message.mn_ReplyPort = tport;
tr.tr_node.io_Command = TR_GETSYSTIME;

WaitTOF();
szzz=zzz;
DoIO((struct IORequest *) &tr);
Ssec=tr.tr_time.tv_secs;
Smic=tr.tr_time.tv_micro;

for(x=0;x<4;x++) WaitTOF();

WaitTOF();
ezzz=zzz;
DoIO((struct IORequest *) &tr);
Esec=tr.tr_time.tv_secs;
Emic=tr.tr_time.tv_micro;

DeletePort(tport);
CloseDevice((struct IORequest *) &tr);

if (GfxBase->DisplayFlags & PAL) clock=3546895;
else clock=3579545;

if (Emic>=Smic) mic=Emic-Smic;
else mic=Emic+1000000-Smic;

sound1.ioa_Period=clock/((((ezzz-szzz)*80*1000000)/mic));
sound2.ioa_Period=sound1.ioa_Period;
}
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/
/*�������������������������������������������������������������������*/