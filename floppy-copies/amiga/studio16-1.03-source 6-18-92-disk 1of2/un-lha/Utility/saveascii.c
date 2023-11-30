#include "exec/types.h"
#include "/include/studio16.h"

/*****************/
/*  Save ASCII ***/
/*****************/

SaveASCII(wfp,rsp)

long wfp;
struct SampleFilePointer *rsp;
{
static short mydata[1024];
long i;
char str[80];
int len;
int sl;

do {
	len=SampleRead(rsp,mydata,1024);
		for (i=0; i <len; i=i+8) {
			if ((len-i) > 7) {
				sprintf(str,"%d,%d,%d,%d,%d,%d,%d,%d\n",mydata[i],mydata[i+1],mydata[i+2],mydata[i+3],mydata[i+4],mydata[i+5],mydata[i+6],mydata[i+7]);
				sl=strlen(str);
				if (Write(wfp,str,sl)!=sl) {
					return(WRITE_ERROR);
					}
				}
			else {	/** handle last line which is < 8 **/
				do {
					sprintf(str,"%d",mydata[i++]);
					Write(wfp,str,strlen(str));
					if (i<len)
						Write(wfp,",",1);
					else
						Write(wfp,"\n",1);
					} while (i < len);
				}
			}
	} while (len==1024);
return(NO_ERROR);
}