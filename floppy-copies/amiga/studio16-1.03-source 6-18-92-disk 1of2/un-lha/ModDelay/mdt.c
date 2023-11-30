#include <exec/types.h>
#include "exec/memory.h"
#include "intuition/intuition.h"
#include "/include/psound.h"
#include "/include/clr.h"
#include <hardware/intbits.h>
#include <hardware/cia.h>
#include "libraries/configvars.h"
#include "exec/interrupts.h"
#include "mdt.data"
/**********************************/
/**********************************/
#define Clock 10000000
/**********************************/
void NotifyRecording();
void NotifyEndRecording();
/**********************************/
long _stack = 4000;
char *_procname = "STUDIO_MODULATION_DELAY_TOY";
long _priority = 0;
long _BackGroundIO = 0;
/**********************************/
/**********************************/
struct ChanKey *AllocChan();
struct ChanKey *mon_key=NULL;
struct ChanKey *play_key=NULL;
struct ChanKey *rec_key=NULL;
/**********************************/
/**********************************/
void ColorGadList();
struct window *OpenNewWind();
struct window *CloseWind();
void RefreshWind();
void SaveGadgetState();
/**********************************/
int istc=0;
void RemKeys();
/**********************************/
short se_cmd_select_module;
short se_cmd_kill_module;
short se_cmd_set_input_gain;
short se_cmd_set_cutoff_freq;
short se_cmd_set_sampling_rate;
short se_cmd_set_delay;
short se_cmd_set_feedback;
short se_cmd_set_channel_volume;
short se_info_module_opened;
short se_info_module_closed;
short se_info_prefs_changed;
short se_cmd_set_timer_rate;
short se_cmd_set_delay2;
short se_cmd_set_mix;
short se_cmd_download_code;
/**********************************/
int GadRef();
/**********************************/
int GlobSampRate=44100;
static UBYTE	AutoFilt=0;
/**********************************/
/**********************************/
void InitModule();
void DisplayWind();
void HideWind();
void handle_studioevents();
void handle_intuievents();
static void WinGadSize();
/**********************************/
/**********************************/
/**********************************/
/**********************************/
extern struct StudioBase *StudioBase;
struct Module 		*mod,*Hmod;
struct Window 		*wind;
struct StandardModState *state;
struct AudioHandler 	*hand;
struct Handler12 	*hand_state;
struct Handler12 SavedState;
struct Window *Rwind;
/**********************************/
/**********************************/
struct Task *task,*FindTask();
/**********************************/
/**********************************/
int FindBestFilter();
/*���������������������������������������������������������������������*/
/*���������������������������������������������������������������������*/
void _main(argv)
char *argv;
{
int mask;
InitModule(argv);

while (TRUE)
	{
	mask=0;
	if (wind) mask |= (1<<wind->UserPort->mp_SigBit);
	if (mod)  mask |= (1<<mod->notifyme->mp_SigBit);
	mask=Wait(mask); 
	if (wind && (mask&(1<<wind->UserPort->mp_SigBit))) handle_intuievents();
	if (mod  && (mask & (1<<mod->notifyme->mp_SigBit))) handle_studioevents();
	}
}
/*���������������������������������������������������������������������*/
/*���������������������������������������������������������������������*/
void InitModule(name)
char *name;
{
char *FindFileName();
struct AudioHandler *FindAudioHandler();
static struct NewModule ModData ={NULL,0,sizeof(struct StandardModState)};

task=FindTask(NULL);

openlibraries();
StandardEnviroment();			/** opens studio16 customscreen **/

ModData.name=FindFileName(name);

if ((mod=(struct Module *)AddModule(&ModData))==0) {telluser("Realtime Delay:","Could Not Add Module",0);closelibraries();exit(10);}

state=mod->state;
/*state->flags|=MS_CHOOSE_HANDLER;*/

se_cmd_select_module		= GetEventID("SE_CMD_SELECT_MODULE");
se_cmd_kill_module		= GetEventID("SE_CMD_KILL_MODULE");
se_cmd_set_input_gain		= GetEventID("SE_CMD_SET_INPUT_GAIN");
se_cmd_set_cutoff_freq		= GetEventID("SE_CMD_SET_CUTOFF_FREQ");
se_cmd_set_sampling_rate	= GetEventID("SE_CMD_SET_SAMPLING_RATE");
se_info_module_opened		= GetEventID("SE_INFO_MODULE_OPENED");
se_info_module_closed		= GetEventID("SE_INFO_MODULE_CLOSED");
se_info_prefs_changed		= GetEventID("SE_INFO_PREFS_CHANGED");
se_cmd_set_delay		= GetEventID("SE_CMD_SET_DELAY");
se_cmd_set_feedback		= GetEventID("SE_CMD_SET_FEEDBACK");
se_cmd_set_timer_rate		= GetEventID("SE_CMD_SET_TIMER_RATE");
se_cmd_set_delay2		= GetEventID("SE_CMD_SET_DELAY2");
se_cmd_set_mix			= GetEventID("SE_CMD_SET_MIX");
se_cmd_download_code		= GetEventID("SE_CMD_DOWNLOAD_CODE");

NotifyMeOnEvent(se_cmd_select_module, mod);
NotifyMeOnEvent(se_cmd_kill_module, mod);
}
/*���������������������������������������������������������������������*/
/*���������������������������������������������������������������������*/
void handle_studioevents()
{
struct StudioEvent *event;
int x;

while ((event=(struct StudioEvent *)GetMsg(mod->notifyme))!=0)
	{
	/*********************************/
	/*********************************/
	/*********************************/
	if(event->type == se_cmd_select_module && ((struct Module *)event->arg1==mod))
		{
		if (wind==NULL)	DisplayWind();
		}

	/*********************************/
	if(event->type == se_cmd_kill_module)
		{
		if (((struct Module *)event->arg1 == mod) || ((struct Module *)event->arg1 == Hmod))
			{
			ReplyMsg(event);
			if (wind) HideWind();
			DeleteModule(mod);
			closelibraries();
			exit(0);
			}
		}

	/*********************************/
	if (event->type == se_info_prefs_changed)
	/*********************************/
		{
		if (mon_key) FreeChan(mon_key);

		UnlockMod(hand_state);
		hand=state->hand;
		Hmod=hand->handler_mod;
		hand_state=(struct Handler12 *)hand->handler_mod->state;

		if((mon_key=AllocChan(hand,0, CK_CHAN_MONITOR | CK_ID_ONLY))==NULL) telluser("Realtime Delay:","COULD NOT GET KEY",0);

		delete_gadget_chain(wind);
		GadRef();
		if (create_gadget_chain(wind,&MONITORbutton)) ;
		FREQknob = (struct slider *)FREQgad.gad_ptr;
		RATEknob = (struct slider *)RATEgad.gad_ptr;
		GAINknob = (struct slider *)GAINgad.gad_ptr;
		}

	/*********************************/
	if (wind && event->type == se_cmd_set_sampling_rate && event->arg1==(int)(hand))
		{
		RATEknob->top_element=660-(((Clock)/2)/event->arg2);
		move_knob(RATEknob);
		}

	/*********************************/
	if (wind && event->type == se_cmd_set_cutoff_freq && event->arg1==(int)(hand))
		{
		x=FindBestFilter(event->arg2);
		if (FREQknob->top_element!=hand_state->number_of_filter_settings-x-1)
			{
			FREQknob->top_element=hand_state->number_of_filter_settings-x-1;
			move_knob(FREQknob);
			}
		}

	/*********************************/
	if (wind && event->type == se_cmd_set_input_gain && event->arg1==(int)(hand))
		{
		GAINknob->top_element=event->arg2;
		move_knob(GAINknob);
		}

	/*********************************/

	ReplyMsg(event);
	}
}
/*���������������������������������������������������������������������*/
/*���������������������������������������������������������������������*/
void handle_intuievents()
{
struct IntuiMessage *message;
ULONG class;

while ((message=(struct IntuiMessage *)GetMsg(wind->UserPort))!=0)
	{
	HandleIntuitMess(wind,message);
	class=message->Class;
	ReplyMsg(message);

	if(class== CLOSEWINDOW)
		{
		HideWind();
		return;
		}
	}
}
struct ChanKey *key1=NULL,*key2=NULL,*key3=NULL,*key0=NULL;
/*****************************************************************/
void DisplayWind()
{
int x;
struct Task *SAVEtask;


if (key0 = (struct ChanKey *)AllocChan(NULL,0,CK_CHAN_PLAY))
	if (key1 = (struct ChanKey *)AllocChan(NULL,1,CK_CHAN_PLAY))
		if (key2 = (struct ChanKey *)AllocChan(NULL,2,CK_CHAN_PLAY))
			key3 = (struct ChanKey *)AllocChan(NULL,3,CK_CHAN_PLAY);

if (key0 && key1 && key2 && key3)
if (!GadRef())
if (wind = OpenNewWind(&Record_NewWindow,mod->state,PREFERENCES,NULL,NULL))
	{
	SavedState=*hand_state;

	BroadcastEventParms(se_cmd_download_code,0,"Studio16:RunTime/dsp/Delay.DSP", 0,0,0,0);

	BroadcastEventParms(se_info_module_opened, mod, 0,0,0,0,0);
	ColorGadList(&MONITORbutton);
/***/
	if (create_gadget_chain(wind,&MONITORbutton)) ;
	FREQknob = (struct slider *)FREQgad.gad_ptr;
	RATEknob = (struct slider *)RATEgad.gad_ptr;
	GAINknob = (struct slider *)GAINgad.gad_ptr;

	if((mon_key=AllocChan(hand,0, CK_CHAN_MONITOR | CK_ID_ONLY))==NULL) telluser("Realtime Delay:","COULD NOT GET KEY",0);

	NotifyMeOnEvent(se_cmd_set_input_gain, mod);
	NotifyMeOnEvent(se_cmd_set_cutoff_freq, mod);
	NotifyMeOnEvent(se_cmd_set_sampling_rate, mod);
	NotifyMeOnEvent(se_cmd_set_channel_volume, mod);

	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.bottomright_3d);
	Move(wind->RPort,1		,66);
	Draw(wind->RPort,wind->Width-2	,66);
	Move(wind->RPort,1		,125);
	Draw(wind->RPort,wind->Width-2	,125);
	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.topleft_3d);
	Move(wind->RPort,1		,67);
	Draw(wind->RPort,wind->Width-2	,67);
	Move(wind->RPort,1		,126);
	Draw(wind->RPort,wind->Width-2	,126);

	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.topleft_3d);
	SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	SetDrMd(wind->RPort,JAM2);
	Move(wind->RPort,(wind->Width>>1)-4*7,69 );
	Text(wind->RPort," Delay ", 7);
	Move(wind->RPort,(wind->Width>>1)-4*12,128 );
	Text(wind->RPort," Modulation ", 12);

	telluser("PRELIMINARY DEMO! This takes over Studio 16","Other modules will not run with this module running.",NOTICE_RETURN_NOW);
	x=0;/* <--- Default prefs */
	SAVEtask=task;task=(struct Task *)-1;
	select_list_entry_by_userdata(((struct droplist *)(PREFselect.gad_ptr))->l,x);
	((struct slider *)(RATEgad.gad_ptr))->top_element=Pref[x][0];move_knob(RATEgad.gad_ptr);
	((struct slider *)(DELAYgad.gad_ptr))->top_element=Pref[x][1];move_knob(DELAYgad.gad_ptr);
	((struct slider *)(FEEDBACKgad.gad_ptr))->top_element=Pref[x][2];move_knob(FEEDBACKgad.gad_ptr);
	((struct slider *)(MIXgad.gad_ptr))->top_element=Pref[x][3];move_knob(MIXgad.gad_ptr);
	((struct slider *)(DELAY2gad.gad_ptr))->top_element=Pref[x][4];move_knob(DELAY2gad.gad_ptr);
	((struct slider *)(TRATEgad.gad_ptr))->top_element=Pref[x][5];move_knob(TRATEgad.gad_ptr);
	task=SAVEtask;
	}
else telluser("Realtime Delay:","Could not Open Window",0);
else telluser("Realtime Delay:","Could not Open Window",0);
else {telluser("Realtime Delay:","CHANNELS PLAYING, Could not Open Window",0); RemKeys();}
}
/*****************************************************************/
void RemKeys()
{
if (key0) FreeChan(key0); key0=NULL;
if (key1) FreeChan(key1); key1=NULL;
if (key2) FreeChan(key2); key2=NULL;
if (key3) FreeChan(key3); key3=NULL;
}
/*****************************************************************/
void HideWind()
{
struct Window *w;
int x;
if (wind && (!rec_key))
	{
/*	SaveGadgetState(&MONITORbutton);*/
	delete_gadget_chain(wind);
	w=wind;
	wind=NULL;
	CloseWind(w);

	IgnoreEvent(se_cmd_set_input_gain, mod);
	IgnoreEvent(se_cmd_set_cutoff_freq, mod);
	IgnoreEvent(se_cmd_set_sampling_rate, mod);
	IgnoreEvent(se_cmd_set_channel_volume, mod);

	if (mon_key) FreeChan(mon_key);
	BroadcastEventParms(se_info_module_closed, mod, 0,0,0,0,0);

for(x=0;x<32;x++)
	{
	hand_state->sampling_rate[x]	=SavedState.sampling_rate[x];
	hand_state->filter[x]		=SavedState.filter[x];
	hand_state->input_gain[x]	=SavedState.input_gain[x];
	hand_state->vol_play[x]		=SavedState.vol_play[x];
	hand_state->vol_out[x]		=SavedState.vol_out[x];
	hand_state->vol_in[x]		=SavedState.vol_in[x];
	hand_state->Extra[x]		=SavedState.Extra[x];
	}

	UnlockMod(hand_state);
	BroadcastEventParms(se_cmd_download_code,0,0, 0,0,0,0);

	RemKeys();
	}
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
int GadRef()
{
int x;

/***/
	if (state->hand==NULL)
		{
		if (hand=FindAudioHandler(NULL,-1,0,NULL,-1))
			{
			state->hand=hand;
			state->handler_name=hand->name;
			state->handler_class_id =hand->class_id;
			}
		else {telluser("Realtime Delay:","Could not Find handler, Closing Window",NULL);return(-1);}
		}
	else	hand=state->hand;

	Hmod=hand->handler_mod;
	hand_state=(struct Handler12 *)hand->handler_mod->state;
	LockMod(hand_state);
/***/
	x=FindBestFilter(hand_state->filter[0]);

	FREQgad.list_size=((struct Handler12 *)hand->handler_mod->state)->number_of_filter_settings;
	FREQgad.top_element=((struct Handler12 *)hand->handler_mod->state)->number_of_filter_settings-x-1;

	RATEgad.top_element=660-(((Clock)/2)/hand_state->sampling_rate[0]);

	GAINgad.top_element=hand_state->input_gain[0];


return(NULL);
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
static void GAINfunc(k)
struct slider *k;
{
char buf[20];
int x;

sprintf(buf,"Gain %2d",k->top_element);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 7);

x=k->top_element;

if (FindTask(NULL)!=task) BroadcastEventParmsLessOne(se_cmd_set_input_gain,mod,hand,x, 0, 0, 0, 0);
}

/*****************************************************************/
static void FREQfunc(k)
struct slider *k;
{
char buf[20];
int	x;

if (FindTask(NULL)!=task)
	{
	x=(hand_state->FreqValFunc)(hand_state->number_of_filter_settings-k->top_element-1);
	BroadcastEventParmsLessOne(se_cmd_set_cutoff_freq,mod,hand,x, 0, 0, 0,0);
	}
else
	{
	x=FindBestFilter(hand_state->filter[0]);
	k->list_size=hand_state->number_of_filter_settings;
	k->top_element=hand_state->number_of_filter_settings-x-1;
	x=(hand_state->FreqValFunc)(x);
	}

sprintf(buf,"Filter %5d",x);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 12);
}
/*****************************************************************/
static void RATEfunc(k)
struct slider *k;
{
char buf[20];
int	x,y;
int	c;

x=660-(k->top_element);
y=(Clock/2)/x;
c=y;
GlobSampRate=c;

sprintf(buf,"Rate %5d",c);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 10);

DELAYfunc(DELAYgad.gad_ptr);

if (FindTask(NULL)!=task) 
	{
	BroadcastEventParmsLessOne(se_cmd_set_sampling_rate,mod,hand,c,0, 0, 0,0);
	if (AutoFilt)	BroadcastEventParms(se_cmd_set_cutoff_freq,hand,c>>1,0, 0, 0,0);
	}
}
/*****************************************************************/
/*****************************************************************/
static void DELAYfunc(k)
struct slider *k;
{
char buf[40];
int x=k->top_element;
int t,d,f;

t=0x2000-x;

t=(t*100000)/(GlobSampRate);
f=t%100;
d=t/100;

sprintf(buf,"Delay %4d.%02dmS",d,f);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 15);

((struct slider *)(DELAY2gad.gad_ptr))->top_element=k->top_element; move_knob(DELAY2gad.gad_ptr);

if (FindTask(NULL)!=task) 
	{
	BroadcastEventParmsLessOne(se_cmd_set_delay,mod,hand,x,0, 0, 0,0);
	}
}
/*****************************************************************/
static void FEEDBACKfunc(k)
struct slider *k;
{
char buf[40];
int x=k->top_element;
int d,f;
d=((x*100)/0x1000);
f=(((x*100)-(d*0x1000))*100)/0x1000;
sprintf(buf,"FeedBack %2d.%02d%%",d,f);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 15);

if (FindTask(NULL)!=task) 
	{
	BroadcastEventParmsLessOne(se_cmd_set_feedback,mod,hand,x * 0x10,0, 0, 0,0);
	}
}


/*****************************************************************/
void MIXfunc(k)
struct slider *k;
{
char buf[40];
int x=k->top_element;
int d,f;
d=((x*100)/0x1000);
f=(((x*100)-(d*0x1000))*100)/0x1000;
sprintf(buf,"D Level  %2d.%02d%%",d,f);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 15);

if (FindTask(NULL)!=task) 
	{
	BroadcastEventParmsLessOne(se_cmd_set_mix,mod,hand,x * 0x10,0, 0, 0,0);
	}
}

/*****************************************************************/
void DELAY2func(k)
struct slider *k;
{
char buf[40];
int x=k->top_element;
int t,d,f;
static int flag = 0;

if (DELAYgad.gad_ptr)
	if (((struct slider *)(DELAYgad.gad_ptr))->top_element < k->top_element)
		{
		flag=1;
		k->top_element=((struct slider *)(DELAYgad.gad_ptr))->top_element;
		x=k->top_element;
		}
if ((!(k->flags & BUTN_HIT)) && flag) {flag=0;move_knob(k);}


t=0x2000-x;

t=(t*100000)/(GlobSampRate);
f=t%100;
d=t/100;

sprintf(buf,"Delay %4d.%02dmS",d,f);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 15);

if (FindTask(NULL)!=task) 
	{
	BroadcastEventParmsLessOne(se_cmd_set_delay2,mod,hand,x,0, 0, 0,0);
	}
}
/*****************************************************************/
void TRATEfunc(k)
struct slider *k;
{
char buf[40];
int x=k->top_element;

sprintf(buf,"Modu Rate %5d",x);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 15);

if (FindTask(NULL)!=task) 
	{
	BroadcastEventParmsLessOne(se_cmd_set_timer_rate,mod,hand,x+20,0, 0, 0,0);
	}
}

/*****************************************************************/
static void AUTOFILTfunc(b)
struct button *b;
{
/*char buf[200];
char buf2[200];
sprintf(buf,"RATE %d  Delay %d  Feed %d",((struct slider *)(RATEgad.gad_ptr))->top_element,((struct slider *)(DELAYgad.gad_ptr))->top_element,((struct slider *)(FEEDBACKgad.gad_ptr))->top_element);
sprintf(buf2,"Level %d  Delay2 %d  ModR %d",((struct slider *)(MIXgad.gad_ptr))->top_element,((struct slider *)(DELAY2gad.gad_ptr))->top_element,((struct slider *)(TRATEgad.gad_ptr))->top_element);
telluser(buf,buf2,NOTICE_RETURN_NOW);*/

if (b->flags&BUTN_HIT)
	{
	BroadcastEventParms(se_cmd_set_cutoff_freq,hand,GlobSampRate>>1, 0, 0, 0,0);
	AutoFilt=1;
	}
else	AutoFilt=0;
}
/*****************************************************************/
/*****************************************************************/
int FindBestFilter(target)
int target;
{
int x;
for(x=0;x < hand_state->number_of_filter_settings && target<(hand_state->FreqValFunc)(x);x++) ;
if (x==hand_state->number_of_filter_settings) x=hand_state->number_of_filter_settings-1;
return(x);
}
/*****************************************************************/
/*****************************************************************/
void NewPrefFunc(k)
struct slider *k;
{
int x;
x=(((struct droplist *)(PREFselect.gad_ptr))->l->current);

((struct slider *)(RATEgad.gad_ptr))->top_element=Pref[x][0];move_knob(RATEgad.gad_ptr);
((struct slider *)(DELAYgad.gad_ptr))->top_element=Pref[x][1];move_knob(DELAYgad.gad_ptr);
((struct slider *)(FEEDBACKgad.gad_ptr))->top_element=Pref[x][2];move_knob(FEEDBACKgad.gad_ptr);
((struct slider *)(MIXgad.gad_ptr))->top_element=Pref[x][3];move_knob(MIXgad.gad_ptr);
((struct slider *)(DELAY2gad.gad_ptr))->top_element=Pref[x][4];move_knob(DELAY2gad.gad_ptr);
((struct slider *)(TRATEgad.gad_ptr))->top_element=Pref[x][5];move_knob(TRATEgad.gad_ptr);

}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/