MODULE calib;

IMPORT InOut, SYSTEM;

VAR

a,b,x :INTEGER;

BEGIN

FOR x:=1 TO 200 DO

SYSTEM.CODE(13FCH,0000H,00BFH,0E301H,1039H,00BFH,0E101H,
                          13FCH,00FFH,00BFH,0E301H,0E340H);
b:=INTEGER(SYSTEM.BYTE(SYSTEM.REGISTER(0)));

InOut.WriteString("value: ");

InOut.WriteInt(b,4);

InOut.WriteLn;


END;


END calib.
