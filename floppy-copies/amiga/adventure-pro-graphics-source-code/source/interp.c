
/* The Professional Adventure System ver 4.0 */
/* Amiga version */
/* (c) 1986 Anthony Wood */
/* (c) 1987 Thom Robertson */

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <stdio.h>
#include <graphics/gfx.h>
#include <devices/audio.h>
#include "adventure.h"
#include "sed1.h"

char command[80],word[80];
struct loc2 rm;
char *nounlist[MAXLIST];
char *verblist[MAXLIST];
char *adjlist[MAXLIST];
char *extlist[MAXLIST];
char *ext2list[MAXRMS];
char *ext3list[MAXLIST];
char *prepolist[MAXLIST];
char *typelist[MAXLIST];
int objnum[2],objx[2][10],objy[2][10],objlist[2][100];
struct Image *object_images[MAXLIST];
int offx[100],offy[100];
int planemask,numplanes,secnum,secnum2,noun_pics[MAXLIST];
struct sequence *sequences[MAXLIST],*s;
int xnum,ynum,eeenum,varval[MAXLIST];
UBYTE *code;
struct IOAudio IOB[4];
struct Samp samp[4];
int codekount,slave[4][2];
UBYTE cmap[32*3];
char userfile[10];
int picx,picy,spckeys,keys[3][20];
char last_com[80];

main()
{
FILE *fp,*fp2;
int rmsec,wxwx,i,ii,iq,k,rk,rk2,r,token,prev_rm,rnum,nnum;
int mcnum,nrflag,orflag,andflag;
int screenw,screenh;
int *ptr1,xx1,xx2,xx3,winx,winy,winw,winh;
char injector,c,cn,terminator;
char thing[200];
int inmode,fragend,mainnum;
int backnum;
char *preplist[8];
char *listlist[5];
char roomdes[80],program[80];     /* filenames */
int noun,verb,prep,adj,prepobj,l,*ll;
int comcase,comlen,comif;
struct winprint winp1,winp2;
struct Window *win1,*win2;
struct Screen *screen;
struct graphic graph1;
struct Image image,*tpoint;

openlibs();

ll = &l;

/* create list of lists */

listlist[0]=0;
listlist[1]=verblist;
listlist[2]=nounlist;
listlist[3]=adjlist;
listlist[4]=prepolist;
 
/* define known prepositions */

preplist[0]="IN";
preplist[1]="ON";
preplist[2]="UNDER";
preplist[3]="WITH";
preplist[4]="BEHIND";
preplist[5]="USING";
preplist[6]="INTO";
preplist[7]="INSIDE";

fp = fopen("symbol.adv","r");
if (fp == NULL) {
   printf("can't open symbol.adv!!");
   exit(10);
   }
fscanf(fp,"%d,",&nnum);
fscanf(fp,"%d,",&rnum);
fscanf(fp,"%d,",&mcnum);
fscanf(fp,"%d,",&xnum);
fscanf(fp,"%d,",&ynum);
fscanf(fp,"%d,",&eeenum);
fscanf(fp,"%d,",&rk);
fscanf(fp,"%d,%d,",&mainnum,&backnum);

rk2 = 1;
do {
   fscanf(fp,"%d,%d,",&rm.desc[rk2],&rm.pic[rk2]);
   fscanf(fp,"%d,%d,%d,%d,%d,%d,",&rm.n[rk2],&rm.s[rk2],&rm.e[rk2],
          &rm.w[rk2],&rm.u[rk2],&rm.d[rk2]);
   i = 0;
   while (i<MAXLIST-1)
      rm.item[rk2][i++] = 0;
   i = 1;
   do {
      fscanf(fp,"%d,",&rm.item[rk2][i]);
      } while(i<MAXLIST && rm.item[rk2][i++] > 0);
   } while(++rk2 < rk);
   rm.desc[rk2] = 0;

   getlist1(fp,rk);


   fscanf(fp,"%d,%d,",&screenw,&screenh);
   fscanf(fp,"%d,",&numplanes);
   fscanf(fp,"%d,%d,",&picx,&picy);
   fscanf(fp,"%d,%d,",&winx,&winy);
   fscanf(fp,"%d,%d,",&winw,&winh);

   fscanf(fp,"%d,",&objnum[0]);
   i = 0;
   while (i < objnum[0])
      fscanf(fp,"%d,%d,",&objx[0][i],&objy[0][i++]);
   
   fscanf(fp,"%d,",&objnum[1]);
   i = 0;
   while (i < objnum[1])
      fscanf(fp,"%d,%d,",&objx[1][i],&objy[1][i++]);

   if (numplanes == 1)
      planemask = 1;
   if (numplanes == 2)
      planemask = 3;
   if (numplanes == 3)
      planemask = 7;
   if (numplanes == 4)
      planemask = 15;
   if (numplanes == 5)
      planemask = 31;

   i = 0;
   while (i++ <MAXLIST) {
      fscanf(fp,"%d,",&noun_pics[i]);
      }

   fscanf(fp,"%d,",&i);
   spckeys = i;
   i = 0;
   while(i < spckeys) {
      fscanf(fp,"%d,%d,%d,",&keys[0][i],&keys[1][i],&keys[2][i]);
      i++;
      }

   fscanf(fp,"%d\n",&secnum);
   if (secnum == 0)
      goto xxsemester;
   i = 0;
   while(i < secnum) {
      sequences[i] = AllocMem(sizeof(struct sequence),0);
      if (sequences[i] == 0) {
         printf("no space for sequence structs, dammit!");
         goto ender1;
         }
      s = sequences[i];
      s->flags = 0;
      fscanf(fp,"%d,%d,%d,",&xx1,&xx2,&xx3);
      s->speed = (char) xx1;
      s->number = (char) xx2;
      s->flags = (1&xx3)*REPEAT + ((2&xx3)*PERM/2) ;
      fscanf(fp,"%d,%d,",&xx1,&xx2);
      s->room = (char) xx1;
      if (xx2 ==1)
         s->flags = s->flags | SIMPLE;
      if (xx2 ==2)
         s->flags = s->flags | SUPER;

      l = 0;
      while (l <s->number) {
      fscanf(fp,"%d,%d,%d,",&(s->sec[l]),&(s->x[l]),&(s->y[l]));
         l++;
         }
      l = 0;
      fscanf(fp,"%d,",&xx1);
      s->framenum = (char) xx1;
      while (l <s->framenum) {
         fscanf(fp,"%s\n",&thing[0]);
         xx1 = strlen(&thing[0]);
         s->names[l] = AllocMem(xx1 + 1,0);
         strcpy(s->names[l],&thing[0]);
         s->graphics[l] = 0;
         l++;
         }
      s->background.Planes[0] = 0;
      loadperm(s);
      i++;
      }

xxsemester:

   
fscanf(fp,"%d,",&codekount);
code = AllocMem(codekount,0);
if (code == 0) {
   printf("No space for code!");
   goto ender1;
   }
l = -1;
do  {
   ++l;
   fscanf(fp,"%d,",&i);
   code[l]=i;
   } while (l <= codekount);

printf("File loaded.\n");
fclose(fp);

/*  open windows, etc.  */
wxwx = 0;
i = 0;
i = getgraphic(&image,ext3list[1],cmap);
if (i != 0) {
   goto ender1;
   }
screen = openscreen(screenw,screenh,numplanes);
win1 = nwwin(screen,0,0,screenw,screenh);
i = 0;
while(i < (2<<(numplanes-1)) * 3)
   SetRGB4(&(screen->ViewPort),i/3,cmap[i++]>>4,cmap[i++]>>4,cmap[i++]>>4);

DrawImage(win1->RPort,&image,0,0);
cleargrap(&image);

newpointer(screen,BUSY);

winp1.WIN = win1;
winp1.stopnum = 0;
winp1.X = winx;
winp1.Y = winy;
winp1.H = winh;
winp1.W = winw;
winp1.x = 0;
winp1.y = 0;
printx(&winp1,"^c");
rmsec = 0;

i = 2;
while (ext3list[i][0] != 'z' || ext3list[i][1] != 'y' || ext3list[i][2] != 'x') {
   object_images[i] = AllocMem(sizeof(struct Image),MEMF_CHIP);
   if (object_images[i] == 0) {
      printf("no space for object images, dammit!");
      goto ender1;
      }
   l = 0;
   l = getgraphic(object_images[i],ext3list[i],0);
   if (l != 0) {
      printf("can't get an  object, dammit!");
      goto ender1;
      }
                i++;
   }
while(i<MAXLIST)
   object_images[i++] = 0;

/*  clear inventory   */
i = 1;
do {
  rm.item[0][i] = 0;
  i++;
  } while(i <= MAXLIST-1);

/*  clear variables   */
userfile[0] = 0;

i = 0;
do {
  varval[i] = 0;
  } while(++i <= MAXLIST-1);

i = 0;
do {
  objlist[0][i] = 0;
  } while(++i <100);

i = 0;
do {
  objlist[1][i] = 0;
  } while(++i <100);

i = 0;
do {
  samp[i].lmem = 0 ;
  samp[i].name = 0 ;
  } while(++i < 4);


/************************************************/
/**      The following code runs the adventure **/
/************************************************/

printx(&winp1,"Welcome to The Professional Adventure Runtime System^n^n");

nrflag = 1;
l=1;
prev_rm=l;

inmode = 1;
injector = 0;
goto interp;

newinput:

if (l<=0 && injector == 0) {
   printx(&winp1,"Sorry, I can't  go that way.^n");
   l=prev_rm;
   nrflag = 0;
   }
varval[rnum] = l;

if (nrflag > 0 && injector == 0) {
   endrmsec(prev_rm,win1);

   /***** load standard room graphic here. *****/
i = 0;
i = getgraphic(&image,ext2list[rm.pic[l]],0);
if (i != 0) {
   goto ender1;
   }
WaitTOF();
DrawImage(win1->RPort,&image,picx,picy);
cleargrap(&image);
nrflag = 0;

startrmsec(l,win1);

if (wxwx == 1)
   eraseroomobjects(win1);
else wxwx = 1;
showroomobjects(l,win1);

printx(&winp1,typelist[rm.desc[l]]);
if (rm.n[l]+rm.s[l]+rm.w[l]+rm.e[l]+rm.u[l]+rm.d[l] != 0) {
   printx(&winp1,"Obvious exits are: ");
   if (rm.n[l] > 0)
      printx(&winp1,"North ");
   if (rm.s[l] > 0)
      printx(&winp1,"South ");
   if (rm.e[l] > 0)
      printx(&winp1,"East ");
   if (rm.w[l] > 0)
      printx(&winp1,"West ");
   if (rm.u[l] > 0)
      printx(&winp1,"Up ");
   if (rm.d[l] > 0)
      printx(&winp1,"Down ");
   }

printx(&winp1,"^n");

k = 0;
for(i=0;i<ITEMSINROOM;i++)
   if (rm.item[l][i]!=0)  k = 1;
if (k != 0) {
   printx(&winp1,"You see: ");
   for(i=0;i<ITEMSINROOM;i++)
      if (rm.item[l][i]!=0) {
         if (i > 1)
            printx(&winp1,", ");
         printx(&winp1,nounlist[rm.item[l][i]]);
         }
      printx(&winp1,".^n");
   }
}
do {
   if (injector == 0)  {
      if (nrflag >= 0)
         printx(&winp1,"What is your command? ");
      winp1.stopnum = 0;
      getx(&winp1);
      }
   else {
      printx(&winp1,&command[0]);
      printx(&winp1,"^n");
      }
   injector = 0;
   prev_rm = l;
   nrflag = 0;
   } while(command[0] == 0);

cap(command);        /* make string all upper case */

if (strcmp(command,"SHOW")==0) {
   i = 0;
   do  {
      ++i;
      } while (i <= codekount);
   goto newinput;
   }

if (1==0) {
   ender1:
   ptr1 = newpointer(screen,NORMAL);
   if (win1 > 0)
      CloseWindow(win1);
   if (screen > 0)
      CloseScreen(screen);
   objectclear(secnum,ptr1);

   printf("Exiting the Adventure system.\n\n");
   exit(0);
   }
if (strcmp(command,"NORTH")==0||strcmp(command,"N")==0) {
   l=rm.n[l];
   inmode = 3;
   nrflag = 1;
   goto interp;
   }
if (strcmp(command,"SOUTH")==0||strcmp(command,"S")==0) {
   l=rm.s[l];
   inmode = 3;
   nrflag = 1;
   goto interp;
   }
if (strcmp(command,"EAST")==0||strcmp(command,"E")==0) {
   l=rm.e[l];
   inmode = 3;
   nrflag = 1;
   goto interp;
   }
if (strcmp(command,"WEST")==0||strcmp(command,"W")==0) {
   l=rm.w[l];
   inmode = 3;
   nrflag = 1;
   goto interp;
   }
if (strcmp(command,"UP")==0||strcmp(command,"U")==0) {
   l=rm.u[l];
   inmode = 3;
   nrflag = 1;
   goto interp;
   }
if (strcmp(command,"DOWN")==0||strcmp(command,"D")==0) {
   l=rm.d[l];
   inmode = 3;
   nrflag = 1;
   goto interp;
   }

if (strcmp(command,"INVENTORY")==0||strcmp(command,"INV")==0) {
   printx(&winp1,"You are carrying: ");
   if (rm.item[0][0] == 0) {
      printx(&winp1,"Nothing.^n");
      goto newinput;
      }
   i = 1;
   while( i <= ITEMSINROOM) {
      if (rm.item[0][i]!=0) {
         if (i > 1)
            printx(&winp1,", ");
         printx(&winp1,nounlist[rm.item[0][i]]);
         }
      i++;
      }
   printx(&winp1,".^n");
   goto newinput;
   }

/* parse the input */
i = 0;
rk = 0;
noun = 0;
verb = 0;
prep = 0;
adj = 0;
prepobj = 0;

while (command[ i ]!= 0) {
   i = getword(i);
   if (verb == 0)
      if ((k = scanlist2( word,verblist) )!= 0) {
         verb = k;
         goto skip;
         }

   if (noun == 0)
      if ((k = scanlist2(word,nounlist) )!= 0) {
         noun = k;
         goto skip;
         }

   if (adj == 0)
      if ((k = scanlist2(word,adjlist) )!= 0) {
         adj = k;
         goto skip;
         }

   if (prep == 0)
      if ((k = scanlist3(word,preplist) )!= 0) {
         prep = k;
         if (command [ i ] == 0) {
            printx(&winp1,"Your preposition needs an object.^n");
            goto newinput;
            }
         i = getword (i);
         if ((prepobj = scanlist2( word,prepolist )) == 0) {
            printx(&winp1,"I don't understand that preposition.^n");
            goto newinput;
            }
          }
skip:

} /* end parser while loop */

inmode = 2;

/*   interpreter     */

interp:

if (l < 1 || injector == 1)
   goto newinput;

if (inmode == 1) {
   inmode = 0;
   rk = 0;
   comif = 0;
   comcase = 0;
   fragend = mainnum;
   goto ijump;
   }

if (inmode == 2) {
   inmode = 3;
   rk = mainnum;
   comif = 0;
   comcase = 0;
   fragend = backnum;
   goto ijump;
   }

if (inmode == 3 && backnum != codekount) {
   inmode = 0;
   rk = backnum;
   comif = 0;
   comcase = 0;
   fragend = codekount;
   varval[rnum] = l;
   goto ijump;
   }

goto newinput;


ijump:

switch (code[rk]) {
 
   case VERB:
      comlen = caseverb(&comcase,&code[rk],verb);
      break;

   case NOUN:
    if (comcase != 1)
      comcase = 2;
      i = 1;
      while (code[rk + i] > 0)  {
         if (code[rk + i] == noun && comcase != 1)
            comcase = 0;
         i++;
         }
      comlen = i+1;
      break;

   case ADJ:
    if (comcase != 1 && comcase != 2)
      comcase = 3;
      i = 1;
      while (code[rk + i] > 0)  {
         if (code[rk + i] == adj && comcase != 1 && comcase != 2)
            comcase = 0;
         i++;
         }
      comlen = i+1;
      break;

   case PREP:
    if (comcase <1 || comcase > 3)
      comcase = 4;
      i = 1;
      while (code[rk + i] > 0)  {
         if (code[rk + i] == prepobj && (comcase < 1 || comcase > 3 ))
            comcase = 0;
         i++;
         }
      comlen = i+1;
      break;

   case TYPE:
      comlen = casetype(comcase,rk,&winp1);
      break;

   case NUMTYPE:
      if (comcase == 0)
         printy(&winp1,varval[code[rk + 1]]);
      comlen = 2;
      break;

   case OR:
      i = 1;
    if (comif < 1) {
      if (comcase== 0 || comcase== 5) {
         if (comcase== 0) {
            if(code[rk+1] == IFRMCON || code[rk+1] == IFINCON)
               i = i + 2;
            else if(code[rk+1] == IFVARLT || code[rk+1] == IFVAREQ || code[rk+1] == IFVARGT)
               i = i + 4;
            else {
               printx(&winp1,"^nOR not followed by if statment!!!^n");
               exit(10);
               }
            }
         else
            comcase = 0;
         }
       }
      else comif--;

      comlen = i;
      break;
     
   case AND:
      i = 1;
      if (comcase > 0)
         comif--;
      comlen = i;
      break;
     
    case DONE:
    comlen = casedone(&comcase);
    break;

    case SDONE:
    comlen = casesdone(&comcase,&nrflag);
    break;

    case NDONE:
    comlen = casendone(&comcase,&nrflag);
    break;

   case POEND:
      if (comcase < 1 || comcase > 3)
      comcase = 0;
      comlen = 1;
      break;

   case ADJEND:
      if (comcase < 1 || comcase > 2)
      comcase = 0;
      comlen = 1;
      break;

   case NEND:
      if (comcase != 1)
      comcase = 0;
      comlen = 1;
      break;

   case VEND:
      comcase = 0;
      comlen = 1;
      break;

   case ENDIF:
      if (comcase == 5) {
         if (comif < 1)
            comcase = 0;
         comif--;
         }
      comlen = 1;
      break;

   case END:
    if (comcase == 0) {
      printx(&winp1,"^n^n   This Adventure brought to you by");
      printx(&winp1,"^n         Sunrize Industries.^n^h");
    end12321:
      ptr1 = newpointer(screen,NORMAL);
      if (win1 > 0)
         CloseWindow(win1);
      if (screen > 0)
         CloseScreen(screen);
      objectclear(secnum,ptr1);
      exit(0);
      }
    comlen = 1;
    break;

   case TOROOM:
    if (comcase == 0) {
       if (code[rk + 1] == VARIABLE)
          l = varval[code[rk + 2]];
       else
          l = code[rk + 2];
       nrflag = 1;
       varval[rnum] = l;
       }
    comlen = 3;
    break;

   case INJECT:
    if (comcase == 0) {
       caseinject(code[rk + 1]);
       injector = 1;
       }
    comlen = 2;
    break;

    case LOADS:
    comlen = caseloads(comcase,rk,win1);
    break;

    case PLAYS:
    comlen = caseplays(comcase,rk,win1);
    break;

    case STOPS:
    comlen = casestops(comcase,rk,win1);
    break;

    case CLEARS:
    comlen = caseclears(comcase,rk,win1);
    break;

    case SLAVE:
    comlen = caseslave(comcase,rk);
    break;

    case UNSLAVE:
    comlen = caseunslave(comcase,rk);
    break;

    case RND:
    comlen = casernd(comcase,rk);
    break;

    case DG:
    comlen = casedg(comcase,rk,win1,&image);
    if (comlen == -1)
       goto end12321;
    break;

    case DS:
    comlen = caseds(comcase,rk,win1);
    break;

    case ENDS:
    comlen = caseends(comcase,rk,win1);
    break;

    case ACTS:
    comlen = caseacts(comcase,rk);
    break;

    case WAIT:
    comlen = casewait(&winp1,comcase,rk);
    break;

    case COBJG:
    comlen = casecobjg(comcase,rk,l,win1,&winp1);
    break;

    case CRMS:
    comlen = casecrms(comcase,rk,l,win1);
    break;

    case CRMG:
    comlen = casecrmg(comcase,rk,l,win1,&image,picx,picy);
    break;

    case OPEN:
    fp2 = caseopen(comcase,rk);
    if (fp2 != 0)
       fp = fp2;
    comlen = 3;
    break;

    case READ:
    comlen = caseread(comcase,rk,fp);
    break;

    case WRITE:
    comlen = casewrite(comcase,rk,fp);
    break;

    case CLOSEF:
    comlen = caseclose(comcase,rk,fp);
    break;

    case STOREG:
    comlen = casestoreg(comcase,rk,rk2,l);
    break;

    case LOADG:
    comlen = caseloadg(comcase,rk,rk2,&l,win1,&winp1,&nrflag,&prev_rm);
    break;

   case RMPLUS:
    if (comcase == 0) {
      i = code[rk + 1];
      if (i == 0)
        i = l;
      cn = code[rk + 2];
      if (cn == nnum)
         cn = noun;
      eraseroomobjects(win1);
      add_r_i(i,cn);
      showroomobjects(l,win1);
      }
      comlen = 3;
      break;

   case RMMINUS:
    if (comcase == 0) {
      cn = code[rk + 1];
      if (cn == nnum)
         cn = noun;
      eraseroomobjects(win1);
      sub_r_i(l,cn);
      showroomobjects(l,win1);
      }
      comlen = 2;
      break;


   case INPLUS:
    if (comcase == 0) {
      cn = code[rk + 1];
      if (cn == nnum)
         cn = noun;
      caseinplus(cn,win1);
      add_r_i(0,cn);
      rm.item[0][0]++;
      }
      comlen = 2;
      break;


   case INMINUS:
    if (comcase == 0) {
      cn = code[rk + 1];
      if (cn == nnum)
         cn = noun;
      i = 0;
      while (i<=MAXRMS) {
        if (rm.item[0][i] == cn)
           rm.item[0][0]--;
        i++;
        }
      sub_r_i(0,cn);
      caseinminus(cn,win1);
      }
      comlen = 2;
      break;


   case INSTEAD:
      if (comcase == 5 && comif <1)
         comcase = 0;
      else
      if (comcase == 0 && comif <1) {
         comif = 0;
         comcase = 5;
         }
      comlen = 1;
      break;

   case DESTROY:
    i = 1;
    if (comcase == 0 ) {
      cn = code[rk+1];
      if (cn == nnum)
         cn = noun;
      while (i<MAXRMS) {
        if (rm.item[0][i] == cn) {
           rm.item[0][i] = 0;
           rm.item[0][0]--;
           }
        i++;
        }
      iq = 1;
      while (iq <rk2) {
         i = 1;
         while (i<MAXRMS) {
           if (rm.item[iq][i] == cn)
              rm.item[iq][i] = 0;
           i++;
           }
         iq++;
         }
      }
      comlen = 2;
      break;

   case IFRMCON:
    i = 0;
    if (comcase == 5 )
       comif++;
    if (comcase == 0 ) {
      cn = code[rk+1];
      if (cn == nnum)
         cn = noun;
      while (rm.item[l][i] != cn && i <MAXRMS +1)
         i++;
      if (i == MAXRMS + 1) {
         comif = 0;
         comcase = 5;
         }
      }
      comlen = 2;
      break;

 
   case IFINCON:
    if (comcase == 5)
       comif++;
    if (comcase == 0) {
      i = 0;
      cn = code[rk+1];
      if (cn == nnum)
         cn = noun;
      while (rm.item[0][i] != cn && i <MAXRMS +1)
         i++;
      if (i == MAXRMS + 1) {
         comif = 0;
         comcase = 5;
         }
      }
      comlen = 2;
      break;


   case ASSIGN:
    i = 5;
    if (comcase == 0) {
       i = caseassign(comcase,rk);
       }
       else {
          while (code[rk+i] != 0)
             i = i + 4;
          }
      comlen = i+1;
      break; /* assign end  */ 

   case IFVARLT:
      caseifvarlt(&comcase,&comif,rk);
      comlen = 5;
      break;

   case IFVAREQ:
      caseifvareq(&comcase,&comif,rk);
      comlen = 5;
      break;

   case IFVARGT:
      caseifvargt(&comcase,&comif,rk);
      comlen = 5;
      break;

   case CONNECT:
      if (comcase == 0) {
         casecon(rk);
         }
      comlen = 5;
      break;

  default:    
      printf("bad command: line %d!\n\n\n",rk);
      printf(" %d %d %d\n",code[rk],code[rk+1],code[rk+2] );
      exit(10);

  }  /*  switch end */

rk = rk + comlen;
if (comcase == -1 || injector == 1)
   goto interp;
if (rk < fragend)
   goto ijump;

/* no understand, Senyore... */
if (inmode == 3) {
   printx(&winp1,"I can't understand that.^n");
   inmode = 0;
   }
goto interp;


}   /* terminates main() */

