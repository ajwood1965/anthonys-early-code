#include "exec/types.h"
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <stdio.h>
#include "intuition/intuition.h"
#include "exec/memory.h"
#include "clib/macros.h"
#include "/include/psound.h"
/*******************************************************************/
/*******************************************************************/
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
struct Library *GadLibBase=NULL;
struct StudioBase *StudioBase=NULL;
struct LayersBase *LayersBase=NULL;
/*******************************************************************/
/*******************************************************************/
int AddMe(x,y)
int x,y;
{
return(x+y);
}
/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/
BOOL init_Interface()
{
if ((StudioBase=(struct StudioBase *)OpenLibrary("studio.library",0))==NULL)
	{return(FALSE);}
if ((GadLibBase=(struct Library *)OpenLibrary("gadlib.library",0))==NULL)
	{return(FALSE);}
if ((IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",33))==NULL)
	{telluser("Interface Library:","Use Workbench 1.2 or greater.",0);return(FALSE);}
if ((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",0))==NULL)
	{telluser("Interface Library:","Can't open graphics library!",0);return(FALSE);}
if ((LayersBase=(struct Layers *)OpenLibrary("layers.library",0))==NULL)
	{telluser("Interface Library:","Can't open Layers library!",0);return(FALSE);}
return(TRUE);
}
/*******************************************************************/
/*******************************************************************/
void rem_Interface()
{
if (IntuitionBase) CloseLibrary(IntuitionBase);
if (GfxBase)	CloseLibrary(GfxBase);
if (GadLibBase)	CloseLibrary(GadLibBase);
if (StudioBase)	CloseLibrary(StudioBase);
if (LayersBase)	CloseLibrary(LayersBase);
}
/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void XCEXIT() { }
void _ONBREAK() { }
int _OSERR;
void _oserr() { }

ULONG	_base,_StackPtr;
char _ProgramName[] = {"INTERFACE.LIB"};
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*******************************************************************/
/*******************************************************************/