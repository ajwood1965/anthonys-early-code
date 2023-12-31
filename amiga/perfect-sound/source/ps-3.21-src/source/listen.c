#include "exec/types.h"
#include "intuition/intuition.h"
#include "psound.h"

extern short digi_type;

/*********************************************/
/***        listen to digitizer            ***/
/*********************************************/

void listen (chan)
int chan;
{
int kk;
void set_digi_type();
void quash_bounce();
static UBYTE *base,*pa,*data,*direction,*data2,*direction2;
static unsigned short sam,i;
unsigned short *aud0lch,*aud0len,*aud0vol,*aud0per,*aud0dat,*dmaconw;
unsigned short *aud1lch,*aud1len,*aud1vol,*aud1per,*aud1dat;
unsigned short *intreq,*intreqr;
unsigned int *aud0lc,*aud1lc;
extern struct Screen *screen;    /* Perfect Sound custom screen */

direction=(UBYTE *)0xBFE301;     /* set bits here to a 1 for output */
data=(UBYTE *)0xBFE101;
data2=(UBYTE *)0xBFD000;
direction2=(UBYTE *)0xBFD200;

if (digi_type==PS_3) {
   box_plot(); 
   Disable();
	save_par();
   if (chan==LEFT)
      new_monleft();
   else
      new_monright();
/* for (kk=0; kk < 1000000; kk++) ; */

	restore_par(); 
   Enable();
   quash_bounce(0);     /** wait for button to go down **/
   quash_bounce(64);    /** wait for button to come up **/
   return;
   }
else {
   show_msg(screen->FirstWindow,255);
   return;
   }

#if 0 /* this following is for older perfect sound hardware, no longer
         supported */
*direction=0;           /* all bits are read */
*data=0;
*direction2=2+4;      /* pa1 & pa2 write */

base=(UBYTE *)0xDFF000;
/* channel 0 chip addresses */
dmaconw=(unsigned short *)(base+0x096);
aud0lch=(unsigned short *)(base+0x0A0);
aud0len=(unsigned short *)(base+0x0A4);
aud0vol=(unsigned short *)(base+0x0A8);
aud0per=(unsigned short *)(base+0x0A6);
aud0dat=(unsigned short *)(base+0x0AA);
aud0lc=(int *)aud0lch;
/* channel 1 chip addresses */
aud1lch=(unsigned short *)(base+0x0B0);
aud1len=(unsigned short *)(base+0x0B4);
aud1vol=(unsigned short *)(base+0x0B8);
aud1per=(unsigned short *)(base+0x0B6);
aud1dat=(unsigned short *)(base+0x0BA);
aud1lc=(int *)aud1lch;
intreq=(unsigned short *)(base+0x09C);
intreqr=(unsigned short *)(base+0x1E);
pa=(UBYTE *)0xBFE001;

*aud0vol=64;
*aud0per=1;
*aud1vol=64;
*aud1per=1;
i=0;

if (chan==STEREO)
   while ((*pa&64)==64) {
      *data2=LEFT;
      sam=(*data-128);
      sam=sam&0xFF;
      i=sam|(sam<<8);
      *aud0dat=i;
      *intreq=128;    /* not in manual, but required to resart ch 0 */

      *data2=RIGHT;
      sam=(*data-128);
      sam=sam&0xFF;
      i=sam|(sam<<8);
      *aud1dat=i;
      *intreq=256;    /* needed to restart ch 1 */
      }
*data2=chan;
if (chan==RIGHT)
   while ((*pa&64)==64) {
      sam=(*data-128);
      sam=sam&0xFF;
      i=sam|(sam<<8);
      *aud1dat=i;
      *intreq=256;
      }

if (chan==LEFT)
   while ((*pa&64)==64) {
      sam=(*data-128);
      sam=sam&0xFF;
      i=sam|(sam<<8);
      *aud0dat=i;
      *intreq=128;
      }

Enable();
quash_bounce(0);     /** wait for button to go down **/
quash_bounce(64);    /** wait for button to come up **/
*direction=0;           /* all bits are read */

#endif
}

/***************************************************************/
/** This routine waits for the mouse left button to stabilize **/
/** on the value passed                                       **/
/***************************************************************/

void quash_bounce(val)

int val;

{
short bounce = TRUE;
short i;
UBYTE *pa;
pa=(UBYTE *)0xBFE001;

while (bounce) {
   bounce = FALSE;
   while ((*pa&64)!= val) ;
   for (i=0; i < 1000; i++)     /* wait for a bbounce **/
      if ( (*pa&64)!=val)
         bounce = TRUE;
   }
}

void dec_gain()
{
UBYTE *data,*data2,*direction;

data=(UBYTE *)0xBFE101;
data2=(UBYTE *)0xBFD000;
direction=(UBYTE *)0xBFE301;     /* set bits here to a 1 for output */

*data2 &= (~4);
*data &= (~128);
*data |= 128;
}

void inc_gain()
{
UBYTE *data,*data2,*direction;

data=(UBYTE *)0xBFE101;
data2=(UBYTE *)0xBFD000;
direction=(UBYTE *)0xBFE301;     /* set bits here to a 1 for output */

*data2 |= 4;
*data &= (~128);
*data |= 128;
}

void set_auto_gain()
{
short k;
short x;
short ktop;

if (isfast())
   ktop = 96;
else
   ktop = 16;

save_par();

for (k=0; k < ktop; k++) {
   if ((x=get_max()) > 220)
      dec_gain();
   else
      inc_gain();
/* printf("max=%d\n",x); */
   }
restore_par();
}


get_max()
{
UBYTE *data2,*data,*direction,*direction2;
short k,x;
short max,val;

data=(UBYTE *)0xBFE101;
direction=(UBYTE *)0xBFE301;     /* set bits here to a 1 for output */
data2=(UBYTE *)0xBFD000;
direction2=(UBYTE *)0xBFD200;

*direction=64+128;
*direction2 = 4;

*data |= 64;           /** pulse RD high **/
*data &= (~64);        /** pulse RD low **/

max = 0;
for (k=0; k < 200; k++) {
   for (x=0; x<20; x++) ;        /** pause roughly 100 usecs **/
   val = (*data)&0x3f;           /** get upper 6 bits **/
   val = val << 2;               /** move bits to correct positions **/
   if (val > max)
      max = val;
   *data2 |= 4;
   *data |= 64;                 /** start next sample **/
   *data &= (~64);

   for (x=0; x<20; x++) ;        /** pause roughly 100 usecs **/
   val = (*data)&0x3f;           /** get upper 6 bits **/
   val = val << 2;               /** move bits to correct positions **/
   if (val > max)
      max = val;
   *data2 &= (~4);
   *data |= 64;                 /** start next sample **/
   *data &= (~64);
   }

return (max);
}

void set_digi_type()
{
UBYTE *data,*direction,*direction2;

data=(UBYTE *)0xBFE101;
direction=(UBYTE *)0xBFE301;     /* set bits here to a 1 for output */
direction2=(UBYTE *)0xBFD200;

save_par();
*direction2=0;
*direction=0;        /** all read **/
if ((*data)&0x3f==0x3f && (*data)&0x3f==0x3f)
   digi_type=PS_3;
else
   digi_type=PS_2;
restore_par();
}

