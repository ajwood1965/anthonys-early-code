#include "exec/types.h"
#include "exec/memory.h"
#include "exec/lists.h"
#include "intuition/intuition.h"
#include "/include/psound.h"
#include <devices/timer.h>
#include "trans.data"
#include "clib/macros.h"
/**********************************/
/* Declarations for CBACK */
long _stack = 10000;			/* Amount of stack space our task needs   */
char *_procname = "STUDIO_TRANSPORT";	/* The name of the task to create         */
long _priority = 0;			/* The priority to run us at              */
long _BackGroundIO = 0;			/* Flag to tell it we want to do I/O      */
/**********************************/
struct Task *task,*FindTask();
int TempSize=0;
/**********************************/
void HidePrefWind();
struct window *DisplayPrefWind();
/**********************************/
struct ChanKey *key[5],*AllocChan();
int GetHandler();
void ForgetHandler();
/**********************************/
short se_cmd_select_module;
short se_cmd_kill_module;
short se_info_module_opened;
short se_info_module_closed;
short se_cmd_hide_window;
short se_cmd_pause_playback;
short se_info_playback_done;
short se_info_record_done;
short se_info_sample_created;
short se_info_sample_deleted;
short se_cmd_stop_all_playback;
short se_cmd_stop_recording;
short se_cmd_set_sampling_rate;
short se_cmd_set_cutoff_freq;
short se_cmd_set_channel_volume;
short se_cmd_set_input_gain;
short se_info_block_recorded;
/**********************************/
/**********************************/
struct window *OpenWind();
struct window *CloseWind();
void RefreshWind();
char *FindFileName();
/**********************************/
/**********************************/
void InitModule();
void DisplayWind();
void HideWind();
void Quit();
void handle_studioevents();
void handle_intuievents();
void handle_gadlibevents();
void WindGadSize();
/**********************************/
/**********************************/
extern struct StudioBase *StudioBase;
struct Module 		*mod,*Hmod;
struct Window 		*wind;
struct StandardModState *state;
struct AudioHandler 	*hand;
struct Handler12 	*hand_state;
struct MsgPort *gadlibport;
/**********************************/
/**********************************/
short Playing=0;
short RECORD=FALSE;
struct Region region[6];
struct ActiveSample *as[5] = {0,0,0,0,0};
struct ChanKey *playkey=NULL;
struct ChanKey *reckey=NULL;
struct ChanKey *KEYQ=NULL;
int DiskSpaceAvailable;
/**************************************************************/
/**************************************************************/
void _main(cmd)
char *cmd;
{
int mask;
void ClipOff();

InitModule(cmd);			/** initalize module **/

while (TRUE)
	{
	mask=NULL;
	if (gadlibport) mask|=(1<<gadlibport->mp_SigBit);
	if (wind) mask |= (1<<wind->UserPort->mp_SigBit);
	if (mod)  mask |= (1<<mod->notifyme->mp_SigBit);
	mask=Wait(mask);
	if (mod)  if(mask & (1<<mod->notifyme->mp_SigBit)) handle_studioevents();
	if (wind) if(mask&(1<<wind->UserPort->mp_SigBit))  handle_intuievents();
	if (gadlibport) if (mask & (1<<gadlibport->mp_SigBit)) handle_gadlibevents();
	}
}
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
void InitModule(name)
char *name;
{
static struct NewModule ModData ={NULL,NULL,sizeof(struct StandardModState)};

task=FindTask(NULL);
openlibraries();
StandardEnviroment();			/** opens studio16 customscreen **/

ModData.name=FindFileName(name);
if ((mod=(struct Module *)AddModule(&ModData))==0) {telluser("Transport:","Could Not Add Module",0);Quit();}

state=mod->state;

se_cmd_select_module 		= GetEventID("SE_CMD_SELECT_MODULE");
se_cmd_kill_module 		= GetEventID("SE_CMD_KILL_MODULE");
se_info_module_opened		= GetEventID("SE_INFO_MODULE_OPENED");
se_info_module_closed		= GetEventID("SE_INFO_MODULE_CLOSED");
se_cmd_hide_window		= GetEventID("SE_CMD_HIDE_WINDOW");
se_info_playback_done		= GetEventID("SE_INFO_PLAYBACK_DONE");
se_info_record_done		= GetEventID("SE_INFO_RECORD_DONE");
se_info_sample_created		= GetEventID("SE_INFO_SAMPLE_CREATED");
se_info_sample_deleted		= GetEventID("SE_INFO_SAMPLE_DELETED");
se_cmd_pause_playback		= GetEventID("SE_CMD_PAUSE_PLAYBACK");
se_cmd_stop_all_playback	= GetEventID("SE_CMD_STOP_ALL_PLAYBACK");
se_cmd_stop_recording		= GetEventID("SE_CMD_STOP_RECORDING");
se_cmd_set_sampling_rate	= GetEventID("SE_CMD_SET_SAMPLING_RATE");
se_cmd_set_cutoff_freq		= GetEventID("SE_CMD_SET_CUTOFF_FREQ");
se_cmd_set_channel_volume	= GetEventID("SE_CMD_SET_CHANNEL_VOLUME");
se_cmd_set_input_gain		= GetEventID("SE_CMD_SET_INPUT_GAIN");
se_info_block_recorded		= GetEventID("SE_INFO_BLOCK_RECORDED");

NotifyMeOnEvent(se_cmd_hide_window, mod);
NotifyMeOnEvent(se_cmd_select_module, mod);
NotifyMeOnEvent(se_cmd_kill_module, mod);

}
/**************************************************************/
/**************************************************************/
void Quit()
{
if (wind) HideWind();
if (mod) DeleteModule(mod);
closelibraries();
exit(0);
}
/**************************************************************/
void RecOff()
{
RecBut.gad_ptr->flags&=~BUTN_HIT;
refresh_gadget(RecBut.gad_ptr);
RECORD=FALSE;
as[4]=NULL;
SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
RectFill(wind->RPort,PlayList[3].box_x1,PlayList[3].box_y1,PlayList[3].box_x2,PlayList[3].box_y1+12);
show_gadget(PlayList[3].gad_ptr);
refresh_gadget(PlayList[3].gad_ptr);
}
/**************************************************************/
void handle_studioevents()
{
void InformStartPlayback();
void InformEndPlayback();
struct StudioEvent *event;
struct Disk_Samp *samp;
char buff[40];
int x;
struct ActiveSample *ps;
void RecordStats();

while ((event=(struct StudioEvent *)GetMsg(mod->notifyme))!=0)
	{
	/*********************************/
/*	if (event->type == se_cmd_set_sampling_rate && event->arg1==(int)(hand))
		{
		((struct slider *)(RATEgad.gad_ptr))->top_element=660-(((Clock/2)/event->arg2)-1);
		if(!(RATEgad.gad_ptr->flags&HIDE_GADGET)) move_knob(RATEgad.gad_ptr);
		}*/
	/*********************************/
	if(event->type == se_info_record_done)
		{
		if (event->arg1 == (int)mod)
			if(as[4]) 
				{
				RecBut.gad_ptr->flags&=~BUTN_HIT;
				refresh_gadget(RecBut.gad_ptr);

				if(!Playing)
					{
					PlayBut.gad_ptr->flags&=~BUTN_HIT;
					StopBut.gad_ptr->flags|=BUTN_HIT;
					refresh_gadget(StopBut.gad_ptr);
					refresh_gadget(PlayBut.gad_ptr);
					}

				SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
				SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
				RectFill(wind->RPort,PlayList[3].box_x1,PlayList[3].box_y1,PlayList[3].box_x2,PlayList[3].box_y1+12);
				show_gadget(PlayList[3].gad_ptr);
				refresh_gadget(PlayList[3].gad_ptr);

				if (reckey) FreeChan(reckey);
				if (playkey) FreeChan(playkey);
				as[4]=NULL;
				reckey=NULL;
				playkey=NULL;
				RECORD=FALSE;
				}
		}
	/*********************************/
	if(event->type == se_info_playback_done)
		{
		if (event->arg1 == (int)mod) Playing--;
		for(x=0;x<5;x++)
			if (key[x]==(struct ChanKey *)event->arg2) {key[x]=NULL;break;}
		if (Playing==0)
			{
			hide_gadget(PauseBut.gad_ptr);
			if(!RECORD)
				{
				show_gadget(RecBut.gad_ptr);

				PlayBut.gad_ptr->flags&=~BUTN_HIT;
				StopBut.gad_ptr->flags|=BUTN_HIT;
				refresh_gadget(StopBut.gad_ptr);
				refresh_gadget(PlayBut.gad_ptr);
				}
			if(RECORD)
				{
				if(as[4] && ((struct droplist *)(AutoBut.gad_ptr))->l->current==0) 
					{
					PlayBut.gad_ptr->flags&=~BUTN_HIT;
					StopBut.gad_ptr->flags|=BUTN_HIT;
					refresh_gadget(StopBut.gad_ptr);
					refresh_gadget(PlayBut.gad_ptr);
					WaitTOF();
					ASStop(as[4]);
					RECORD=FALSE;
					}
				}
			}
		}
	/*********************************/
	if(event->type == se_info_sample_deleted)
		{
		delete_list_entry_by_userdata(((struct droplist *)(PlayList[0].gad_ptr))->l, event->arg1);
		delete_list_entry_by_userdata(((struct droplist *)(PlayList[1].gad_ptr))->l, event->arg1);
		delete_list_entry_by_userdata(((struct droplist *)(PlayList[2].gad_ptr))->l, event->arg1);
		delete_list_entry_by_userdata(((struct droplist *)(PlayList[3].gad_ptr))->l, event->arg1);
		}
	/*********************************/
	if(event->type == se_info_sample_created)
		{
		samp=(struct Disk_Samp *)event->arg1;
		sprintf(buff,"%-12s",samp->name);
		add_list_entry(((struct droplist *)(PlayList[0].gad_ptr))->l,buff, samp);
		add_list_entry(((struct droplist *)(PlayList[1].gad_ptr))->l,buff, samp);
		add_list_entry(((struct droplist *)(PlayList[2].gad_ptr))->l,buff, samp);
		add_list_entry(((struct droplist *)(PlayList[3].gad_ptr))->l,buff, samp);
		}
	/*********************************/
	if (event->type == se_info_block_recorded)
		{
		ps=(struct ActiveSample *)event->arg1;
		TempSize=ASPos(ps);
		if (RECORD)
		if(TempSize>DiskSpaceAvailable)
			{
			ASStop(as[4]);
			RECORD=FALSE;
			telluser("Record:","Disk Full!",NULL);
			}
		else RecordStats();
		}
	/*********************************/
	if(event->type == se_cmd_select_module && (struct Module *)event->arg1==mod)
			if (wind==NULL) DisplayWind();

	/*********************************/
	if(event->type == se_cmd_kill_module && (struct Module *)event->arg1 == mod)
		{
		ReplyMsg(event);
		Quit();
		}
	/*********************************/
	if (wind && event->type == se_cmd_set_sampling_rate && event->arg1==(int)(hand))
		{
		((struct slider *)(RATEgad.gad_ptr))->top_element=660-(((Clock/2)/event->arg2)-1);
		if (!(((struct slider *)(RATEgad.gad_ptr))->flags&HIDE_GADGET)) move_knob(RATEgad.gad_ptr);
		}

	/*********************************/
	if (wind && event->type == se_cmd_set_cutoff_freq && event->arg1==(int)(hand))
		{
		x=FindBestFilter(event->arg2);
		if (((struct slider *)(FILTERgad.gad_ptr))->top_element!=hand_state->number_of_filter_settings-x-1)
			{
			((struct slider *)(FILTERgad.gad_ptr))->top_element=hand_state->number_of_filter_settings-x-1;
			if (!(((struct slider *)(FILTERgad.gad_ptr))->flags&HIDE_GADGET)) move_knob(FILTERgad.gad_ptr);
			}
		}

	/*********************************/
	if (wind && event->type == se_cmd_set_input_gain && event->arg1==(int)(hand))
		{
		((struct slider *)(GAINgad.gad_ptr))->top_element=event->arg2;
		if (!(((struct slider *)(GAINgad.gad_ptr))->flags&HIDE_GADGET)) move_knob(GAINgad.gad_ptr);
		}

	ReplyMsg(event);
	}
}
/**************************************************************/
/**************************************************************/
void handle_intuievents()
{
struct IntuiMessage *message;
ULONG class;

while ((message=(struct IntuiMessage *)GetMsg(wind->UserPort))!=0)
	{
	HandleIntuitMess(wind,message);
	class=message->Class;
	ReplyMsg(message);

	if (class == NEWSIZE)
		WindGadSize();

	if (class == CLOSEWINDOW)
		{
		HideWind();
		return;
		}
	}
}
/**************************************************************/
/**************************************************************/
void handle_gadlibevents()
{
struct GadMsg *msg;
struct GadgetHeader *g;
ULONG flags;
void PlaySamples();

while ((msg=(struct GadMsg *)GetMsg(gadlibport))!=0) 
	{
	g = msg->g;
	flags=msg->flags;
	ReplyMsg(msg);

	if (g==PlayBut.gad_ptr) 
		{
		if (flags & BUTN_HIT) PlaySamples();
		}
	}
}
/**************************************************************/
/**************************************************************/
void DisplayWind()
{
char buff[200];
struct MinNode *node;
struct Disk_Samp *samp;
struct Window *OpenNewWind();
if (GetHandler()!=-1)
{
GlobSampRate=hand_state->sampling_rate[0];
if ((wind = OpenNewWind(&NewWind,mod->state,NULL,NULL))!=0)  
	{
	BroadcastEventParms(se_info_module_opened, mod, 0,0,0,0,0);
	ColorGadList(Gadgets);
	WindGadSize();

	gadlibport = (struct MsgPort *)CreatePort(0,0);
	PlayBut.notifyme=gadlibport;
	if (create_gadget_chain(wind,Gadgets)==0)  {telluser("Transport:","Could not create gadgets!",0); exit(10);}
	hide_gadget(PauseBut.gad_ptr);

	NotifyMeOnEvent(se_info_record_done, mod);
	NotifyMeOnEvent(se_info_playback_done,mod);
	NotifyMeOnEvent(se_info_block_recorded,mod);
	ObtainSharedSemaphore(&StudioBase->lockstudio);

	for (node=StudioBase->samps.mlh_Head; node->mln_Succ; node = node->mln_Succ)
		{
		samp=(struct Disk_Samp *)node;
		sprintf(buff,"%-12s",samp->name);
		add_list_entry(((struct droplist *)(PlayList[0].gad_ptr))->l,buff, samp);
		add_list_entry(((struct droplist *)(PlayList[1].gad_ptr))->l,buff, samp);
		add_list_entry(((struct droplist *)(PlayList[2].gad_ptr))->l,buff, samp);
		add_list_entry(((struct droplist *)(PlayList[3].gad_ptr))->l,buff, samp);
		}

	ReleaseSharedSemaphore(&StudioBase->lockstudio);

	NotifyMeOnEvent(se_cmd_set_sampling_rate,mod);
	NotifyMeOnEvent(se_cmd_set_cutoff_freq,mod);
	NotifyMeOnEvent(se_cmd_set_input_gain,mod);
	NotifyMeOnEvent(se_info_sample_created,mod);
	NotifyMeOnEvent(se_info_sample_deleted,mod);
	}
else telluser("Transport:","Can not open window!",0);
}
}
/**************************************************************/
/**************************************************************/
void HideWind()
{
struct Window *w;

if (wind)
	{
	IgnoreEvent(se_cmd_set_sampling_rate,mod);
	IgnoreEvent(se_cmd_set_cutoff_freq,mod);
	IgnoreEvent(se_cmd_set_input_gain,mod);
	IgnoreEvent(se_info_record_done,mod);
	IgnoreEvent(se_info_playback_done,mod);
	IgnoreEvent(se_info_sample_created,mod);
	IgnoreEvent(se_info_sample_deleted,mod);
	IgnoreEvent(se_info_block_recorded, mod);
	DeletePort(gadlibport);	gadlibport=0;

	ForgetHandler();

	delete_gadget_chain(wind);
	w=wind;
	wind=NULL;
	CloseWind(w);
	BroadcastEventParms(se_info_module_closed, mod, 0,0,0,0,0);
	}
}
/**************************************************************/
/**************************************************************/
void WindGadSize()
{
void InformEndPlayback();
if (wind)
	{
	RefreshWind(wind);
	draw_indent(wind->RPort,  8,15,wind->Width-9,wind->Height-5,StudioBase->defaults.colors.gadget_window.topleft_3d,StudioBase->defaults.colors.gadget_window.bottomright_3d);
	draw_indent(wind->RPort,10,16,px2-1,wind->Height-6,StudioBase->defaults.colors.gadget_window.bottomright_3d,StudioBase->defaults.colors.gadget_window.topleft_3d);

	draw_indent(wind->RPort,px2,pyb-1,px2p+pbl*4+2,pyb+15,StudioBase->defaults.colors.gadget_window.bottomright_3d,StudioBase->defaults.colors.gadget_window.topleft_3d);
	draw_indent(wind->RPort,px2,16,px2p+pbl*4+2,pyb-2,StudioBase->defaults.colors.gadget_window.bottomright_3d,StudioBase->defaults.colors.gadget_window.topleft_3d);

	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.bottomright_3d);
	SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	Move(wind->RPort,px1-14,py1+8);Text(wind->RPort,"#1",2);
	Move(wind->RPort,px1-14,py2+8);Text(wind->RPort,"#2",2);
	Move(wind->RPort,px1-14,py3+8);Text(wind->RPort,"#3",2);
	Move(wind->RPort,px1-14,py4+8);Text(wind->RPort,"#4",2);

	}
}
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/*��������������������������������������������������������������������*/
/*��������������������������������������������������������������������*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/*/
/* \  /								 \  / */
/*  XX				Playback			  XX  */
/* /  \								 /  \ */
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/*/
/*��������������������������������������������������������������������*/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
void STOPfunc(b)
struct button *b;
{
if (b->flags&BUTN_HIT)
	{
	PlayBut.gad_ptr->flags&=~BUTN_HIT;

	if (key[0]) {ASStop(as[0]);key[0]=NULL;}
	if (key[1]) {ASStop(as[1]);key[1]=NULL;}
	if (key[2]) {ASStop(as[2]);key[2]=NULL;}
	if (key[3]) {ASStop(as[3]);key[3]=NULL;}
	if(RECORD && as[4]) {ASStop(as[4]);RECORD=FALSE;}
	}
}
/*****************************************************************/
void PAUSEfunc(b)
struct button *b;
{
if (b->flags&BUTN_HIT) BroadcastEventParms(se_cmd_pause_playback, hand, TRUE,0,0,0,0);

else BroadcastEventParms(se_cmd_pause_playback, hand, FALSE,0,0,0,0);
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
void PlaySamples()
{
struct Disk_Samp *samp;
struct ActiveSample *PlaySam();
struct ActiveSample *RecSam();

if (!Playing && !as[4])
	{
	as[0]=NULL;
	as[1]=NULL;
	as[2]=NULL;
	as[3]=NULL;
	as[4]=NULL;

	samp=(struct Disk_Samp *)((struct droplist *)(PlayList[0].gad_ptr))->l->active_entry->user_data;
	if (samp) as[0]=PlaySam(samp,0);
	samp=(struct Disk_Samp *)((struct droplist *)(PlayList[1].gad_ptr))->l->active_entry->user_data;
	if (samp) as[1]=PlaySam(samp,1);
	samp=(struct Disk_Samp *)((struct droplist *)(PlayList[2].gad_ptr))->l->active_entry->user_data;
	if (samp) as[2]=PlaySam(samp,2);

	samp=NULL;
	if (!RECORD) samp=(struct Disk_Samp *)((struct droplist *)(PlayList[3].gad_ptr))->l->active_entry->user_data;
	if (samp) as[3]=PlaySam(samp,3);

	if (RECORD)
		{
		as[4]=RecSam(RecFileName);
		if (as[4]==NULL)
			{
			RecBut.gad_ptr->flags&=~BUTN_HIT;
			refresh_gadget(RecBut.gad_ptr);
			RECORD=FALSE;
			show_gadget(PlayList[3].gad_ptr);
			refresh_gadget(PlayList[3].gad_ptr);

			if ((!as[0])&&(!as[1])&&(!as[2])&&(!as[3]))
				{
				PlayBut.gad_ptr->flags&=~BUTN_HIT;
				StopBut.gad_ptr->flags|=BUTN_HIT;
				refresh_gadget(StopBut.gad_ptr);
				refresh_gadget(PlayBut.gad_ptr);
				return;
				}
			}
		}


	if (as[0]) Playing++;
	if (as[1]) Playing++;
	if (as[2]) Playing++;
	if (as[3]) Playing++;

	if (Playing || as[4])
		{
		BroadcastEventParms(se_cmd_pause_playback, hand, TRUE,0,0,0,0);
		if (as[0]) ASTrigger(as[0]);
		if (as[1]) ASTrigger(as[1]);
		if (as[2]) ASTrigger(as[2]);
		if (as[3]) ASTrigger(as[3]);
		if (as[4]) ASTrigger(as[4]);
		WaitTOF();
		BroadcastEventParms(se_cmd_pause_playback, hand,FALSE,0,0,0,0);
		}
	if (!Playing && !RECORD)
		{
		PlayBut.gad_ptr->flags&=~BUTN_HIT;
		StopBut.gad_ptr->flags|=BUTN_HIT;
		refresh_gadget(StopBut.gad_ptr);
		refresh_gadget(PlayBut.gad_ptr);
		}
	else	{
		if (Playing) show_gadget(PauseBut.gad_ptr);
		if (!RECORD) hide_gadget(RecBut.gad_ptr);
		}
	}
}
/*****************************************************************/
/*****************************************************************/
struct ActiveSample *PlaySam(samp,x)
struct Disk_Samp *samp;
int x;
{
int ASFLAG=0;
if (samp)
	{
	region[x].samp=samp;
	region[x].start=0;
	region[x].end=samp->length-1;
	region[x].owner=mod;
	region[x].user_data=NULL;
	region[x].parms=samp->parms;
	if(((struct droplist *)(AutoVolBut.gad_ptr))->l->current==1) ASFLAG=AS_IGNORE_DEFAULT_VOLUME;
	if ((as[x]=(struct ActiveSample *)ASOpen(&region[x], AS_PLAY|AS_AUTO_DATA|AS_AUTO_CLOSE|AS_AUTO_FREECHAN|ASFLAG))!=0)
		{
		as[x]->key=(struct ChanKey *)AllocChan(hand,x,CK_CHAN_PLAY);
		key[x]=as[x]->key;
		if (as[x]->key==0)
			{
			ASClose(as[x]);
			telluser("Transport:","Can't Allocate a play channel!",NULL);
			return(NULL);
			}
		}
	else {telluser("Transport:","Can not open disk sample",NULL);return(NULL);}
	}
return(as[x]);
}
/*****************************************************************/
char sn[200];
/*****************************************************************/
struct ActiveSample *RecSam(name)
char *name;
{
struct Disk_Samp *samp;
int y=0;

int DiskSpaceAvail();

DiskSpaceAvailable=DiskSpaceAvail();
if (DiskSpaceAvailable<1) 
	{
	telluser("Transport:","Not enough disk space available to record");
/*	but[2].gad_ptr->flags&=~BUTN_HIT;
	refresh_gadget(but[2].gad_ptr);*/
	return(NULL);
	}

sprintf(sn,"%s",name);
while(FindSample(sn,-1))
	{
	y++;
	sprintf(sn,"%s_#%d",name,y);
	}

if (CreateSample(sn, 0, &samp)==NO_ERROR)
	{
	region[4].samp=samp;
	region[4].start=0;
	region[4].end=0;
	region[4].owner=mod;
	region[4].user_data=NULL;
	if (((struct droplist *)(IRateBut.gad_ptr))->l->current==0) region[4].parms.rate=hand_state->sampling_rate[0];
	else region[4].parms.rate=GlobSampRate;
	region[4].parms.filter3db=hand_state->filter[0];
	region[4].parms.volume=3200;
	if ((as[4]=(struct ActiveSample *)ASOpen(&region[4], AS_RECORD|AS_AUTO_CLOSE|AS_AUTO_DATA))!=0)
		{
		if (((struct droplist *)(TypeBut.gad_ptr))->l->current==0)
			as[4]->key=(struct ChanKey *)AllocChan(hand,0,CK_CHAN_RECORD|CK_TAP_INPUT);
		else 	as[4]->key=(struct ChanKey *)AllocChan(hand,0,CK_CHAN_RECORD|CK_TAP_OUTPUT);
		playkey=(struct ChanKey *)AllocChan(hand,3,CK_CHAN_PLAY);
		reckey=as[4]->key;
		if (reckey==0 || playkey==0)
			{
			if (reckey) {FreeChan(reckey);reckey=NULL;}
			if (playkey) {FreeChan(playkey);playkey=NULL;}
			ASClose(as[4]);
			CloseSample(samp);
			telluser("Transport:","Can't Allocate a record channel!",NOTICE_RETURN_NOW);
			RecOff();
			return(NULL);
			}
		}
	else {telluser("Transport:","Can not open record sample",NOTICE_RETURN_NOW);CloseSample(samp);RecOff();return(NULL);}
	}
else {telluser("Transport:","Can't create record sample!",0);RecOff();return(NULL);}
return(as[4]);
}
/*****************************************************************/
/*****************************************************************/
void RECfunc(b)
struct button *b;
{
void RecordStats();

if (b->flags&BUTN_HIT) 
	{
	RECORD=TRUE;
	hide_gadget(PlayList[3].gad_ptr);

	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	RectFill(wind->RPort,PlayList[3].box_x1,PlayList[3].box_y1,PlayList[3].box_x2,PlayList[3].box_y1+12);
	TempSize=0;
	RecordStats();
	}
else
	{
	if (as[4] && RECORD==TRUE) {ASStop(as[4]);}

	RECORD=FALSE;

	show_gadget(PlayList[3].gad_ptr);

	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	RectFill(wind->RPort,PlayList[3].box_x1,PlayList[3].box_y1,PlayList[3].box_x2,PlayList[3].box_y1+12);
	refresh_gadget(PlayList[3].gad_ptr);
	}
}
/*****************************************************************/
/*****************************************************************/

/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
/**************************************************************/
void ForgetHandler()
{
if (KEYQ) FreeChan(KEYQ);

UnlockMod(hand_state);
hand=NULL;hand_state=NULL;
Hmod=NULL;
}
/**************************************************************/
int GetHandler()
{
struct AudioHandler *FindAudioHandler();

if (state->hand==NULL) 
	{
	hand=FindAudioHandler(NULL,-1,0,NULL,-1);
	if (hand)
		{
		state->hand=hand;
		state->handler_name=hand->name;
		state->handler_class_id =hand->class_id;
		}
	else {telluser("Transport:","Could not find a Handler.",NULL);return(-1);}
	}
else	hand=state->hand;
hand_state=(struct Handler12 *)hand->handler_mod->state;
LockMod(hand_state);
if((KEYQ=AllocChan(hand,0, CK_CHAN_MONITOR | CK_ID_ONLY))==NULL) telluser("Transport:","COULD NOT GET KEY",0);


Hmod=hand->handler_mod;
return(NULL);
}
/**************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
void ParmsChange(old,new)
int old,new;
{
struct NewGadgetHeader *g;

g=&AutoBut;
while(g)
	{
	if (g->user_data==(APTR)old) hide_gadget(g->gad_ptr);
	g=g->next;
	}
SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
RectFill(wind->RPort,px2+2,17,px2p+pbl*4,pyb-3);
g=&AutoBut;

while(g)
	{
	if (g->user_data==(APTR)new) {show_gadget(g->gad_ptr);refresh_gadget(g->gad_ptr);}
	g=g->next;
	}
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
void RecordParms()
{
ParmsChange(9,1);

draw_indent(wind->RPort,mx1,my1,mx1+FileNameGad.Width,my1+10,StudioBase->defaults.colors.gadget_window.bottomright_3d,StudioBase->defaults.colors.gadget_window.topleft_3d);
RefreshGList(&FileNameGad,wind,NULL,1);
AddGList(wind,&FileNameGad,NULL,1,NULL);
RefreshGList(&FileNameGad,wind,NULL,1);
SetAPen(wind->RPort,0);
SetBPen(wind->RPort,0);
Move(wind->RPort,mx1+2,my1+1);
Draw(wind->RPort,mx1+FileNameGad.Width-2,my1+1);
}
/*****************************************************************/
void RecordParmsX()
{
ParmsChange(1,9);
RemoveGList(wind,&FileNameGad,1);
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
void SampleParms()
{
int FindBestFilter();

((struct slider *)(RATEgad.gad_ptr))->top_element=660-(((Clock/2)/hand_state->sampling_rate[0])-1);
((struct slider *)(GAINgad.gad_ptr))->top_element=hand_state->input_gain[0];
((struct slider *)(FILTERgad.gad_ptr))->list_size=((struct Handler12 *)hand_state)->number_of_filter_settings;
((struct slider *)(FILTERgad.gad_ptr))->top_element=((struct Handler12 *)hand_state)->number_of_filter_settings-FindBestFilter(hand_state->filter[0])-1;

ParmsChange(9,2);

move_knob(RATEgad.gad_ptr);
move_knob(GAINgad.gad_ptr);
move_knob(FILTERgad.gad_ptr);
}
/*****************************************************************/
void SampleParmsX()
{
ParmsChange(2,9);
}
/*****************************************************************/
static void RATEfunc(k)
struct slider *k;
{
char buf[20];
int	x,y;
int	c;

x=660-(k->top_element);
x++;
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

if (FindTask(NULL)!=task) BroadcastEventParmsLessOne(se_cmd_set_sampling_rate,mod,hand,c,0, 0, 0,0);
}
/*****************************************************************/
int FindBestFilter(target)
int target;
{
int x;
for(x=0;x < hand_state->number_of_filter_settings && target<(hand_state->FreqValFunc)(x);x++) ;
if (x==hand_state->number_of_filter_settings) x=hand_state->number_of_filter_settings-1;
return(x);
}
/**************************************************************/
void FILTERfunc(k)
struct slider *k;
{
char buf[20];
int	x;

x=(hand_state->FreqValFunc)(hand_state->number_of_filter_settings-k->top_element-1);
if (FindTask(NULL)!=task) BroadcastEventParmsLessOne(se_cmd_set_cutoff_freq,mod,hand,x, 0, 0, 0,0);

sprintf(buf,"Filter %5d",x);
GlobFilterCut=x;
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 12);
}
/*****************************************************************/
void GAINfunc(k)
struct slider *k;
{
char buf[20];
int x;
static int KFLAG=0;
static int VOL=0;

sprintf(buf,"Gain %2d",k->top_element);
SetAPen(k->kw->RPort, k->c->slider_knob.draw_selected);
if (k->flags & BUTN_HIT) SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_selected);
else SetBPen(k->kw->RPort,  k->c->slider_knob.hitbox_unselected);
SetDrMd(k->kw->RPort, JAM2);
Move(k->kw->RPort, k->knob_x+4, k->knob_y+8);
Text(k->kw->RPort, buf, 7);

x=k->top_element;

if (KFLAG!=(k->flags&BUTN_HIT)) 
	{
	if (!KFLAG) 
		{
		VOL=hand_state->vol_in[0];
		BroadcastEventParms(se_cmd_set_channel_volume,KEYQ,3200,0,0,0);
		}
	else	BroadcastEventParms(se_cmd_set_channel_volume,KEYQ,VOL,0,0,0);
	KFLAG=(k->flags&BUTN_HIT);
	}
if (FindTask(NULL)!=task) BroadcastEventParmsLessOne(se_cmd_set_input_gain,mod,hand,x, 0, 0, 0, 0);
}
/*****************************************************************/
void RecordStats()
{
int q,w;
char str[80];

if (RECORD)
	{
	q=(TempSize*2)/(1024*1024);
	w=(((TempSize*2-(q*1024*1024))/1024)*1000)/1024;
	sprintf(str,"Size:%3d.%03d M",q,w);
	LockGadLibRender();
	SetBPen(wind->RPort,StudioBase->defaults.colors.gadget_window.background);
	SetAPen(wind->RPort,StudioBase->defaults.colors.gadget_window.bottomright_3d);
	SetDrMd(wind->RPort,JAM2);
	Move(wind->RPort,px1+12,py4+8);
	Text(wind->RPort, str,strlen(str));
	UnLockGadLibRender();
	}
}
/*****************************************************************/
int DiskSpaceAvail()
{
struct InfoData diskinfo;
struct FileLock      *lock = NULL;
int x;
int RamSpaceAvail();

if (!stricmp(StudioBase->defaults.temp_files_path,"ram:"))
	return(RamSpaceAvail());

lock=(struct FileLock *)Lock(StudioBase->defaults.temp_files_path,ACCESS_READ);
if (lock)
	{
	if(Info(lock,&diskinfo))
		{


		x= 
(
	(
		((diskinfo.id_NumBlocks-diskinfo.id_NumBlocksUsed)-(diskinfo.id_NumBlocks-diskinfo.id_NumBlocksUsed)/70)
		*
		(diskinfo.id_BytesPerBlock/2)
	)
	-
	(
		(
			(StudioBase->defaults.active_buf_size*4)
			+
			StudioBase->defaults.record_safety_size
		)
	)
);
		}
	UnLock(lock);
	return(x);
	}
else
	{
	telluser("Transport:","Does the disk volume exist?");
/*	but[2].gad_ptr->flags&=~BUTN_HIT;
	refresh_gadget(but[2].gad_ptr);*/
	return(0);
	}
}
/*****************************************************************/
int RamSpaceAvail()
{
int x;
int RamSpaceAvail();

		x= 
(
	(
		((AvailMem (NULL)*70)/71)/2
	)
	-
	(
		(
			(StudioBase->defaults.active_buf_size*8)
			+
			StudioBase->defaults.record_safety_size
		)
	)
);

return(x);
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/