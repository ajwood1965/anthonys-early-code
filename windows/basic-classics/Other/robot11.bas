��jd :��

****************************************
ROBOTS BY LANCE MICKLUS, COPYRIGHT 1978
VERSION 1.1
****************************************

 �jx � A�Z �j� � PO(50) �j� � 9k� �@320,"USING THE UP, DOWN, LEFT, & RIGHT ARROW KEYS, SEE IF YOU" {k� �"CAN KEEP YOUR MAN ''X'' FROM GETTING ZAPPED BY THE ROBOTS." �k� �:�"HIT ''ENTER'' TO BEGIN";A$ �k� FE�5 : RT�20 �k:��

GET INITIAL CONDITIONS �k� : � �20) "ROBOT BY LANCE MICKLUS" : � : � l,�"FENCES =";�(FE) +l@�"ROBOTS =";�(RT) 1lT� _lh�"TYPE FENCES TO CHANGE NUMBER OF FENCES" �l|�"     ROBOTS TO CHANGE NUMBER OF ROBOTS" �l��"     PLAY IF READY TO PLAY THE GAME" �l��"     QUIT TO END THE GAME" �l�� �l��"OPTION";A$ �l�A$��(A$,1) m�� A$�"F" � 620 %m� A$�"R" � 680 8m� A$�"P" � 740 Lm0� A$�"Q" � 2600 VmD� 280 pmX� ** CHANGE FENCES ** �ml�"HOW MANY FENCES (1-15)";FENCES �m�� FE�1 � FE�15 � 620 :� � 280 �m�� ** CHANGE ROBOTS ** �m��"HOW MANY ROBOTS (1-50)";RT n�� RT�1 � RT�50 � 680 :� � 280 0n�:��

DRAW PLAYING FIELD 6n�� En�� Y�0 � 47 Pn�(0,Y) [n �(1,Y) hn4�(126,Y) unH�(127,Y) {n\� �np� X�2 � 125 �n��(X,0) �n��(X,47) �n�� �n�:�� �n�� ** DRAW FENCES *** �n�:�� �n�� I�1 � FE 
o� �(4) � 1100, 1200, 1300, 1400, 1500 o$� o8� 1540 ;oLY��(43)�2 : � �(2,Y) � 1100 Oo`� X�2 � 5��(40) zot� �(X�1,Y) � �(X�1,Y�1) � �(X�1,Y�1) � �o��(X,Y) �o�� : � �o�Y��(43)�2 : � �(2,Y) � 1200 �o�� X�125 � 120��(40) � �1 �o�� �(X�1,Y) � �(X�1,Y�1) � �(X�1,Y�1) � p��(X,Y) p � : � -pX��(122)�2 : � �(X,1) � 1300 Ap(� Y�1 � 3��(12) yp<� �(X,Y�1) � �(X�1,Y�1) � �(X�1,Y�1) � �(X�2,Y�1) � �pP�(X,Y) : �(X�1,Y) �pd� : � �pxX��(122)�2 : � �(X,1) � 1300 �p�� Y�46 � 44��(12) � �1 q�� �(X,Y�1) � �(X�1,Y�1) � �(X�1,Y�1) � �(X�2,Y�1) � #q��(X,Y) : �(X�1,Y) -q�� : � 8q�� 1100 Tq�� ** POSITION PIECES ** eqHU�64��(893) �q� �(15360�HU)��32 � 1540 :� � 15360�HU,88 �q,RO�RT �q@� I�1 � RO �qTJ�64��(893) �qh� �(15360�J)��32 � 1620 �q|PO(I)�J : � 15360�J,79 �q�� r�� ��"" � 1700 r�:��

GAME LOOP &r�:�� @r�� ** PLAYER'S MOVE ** Hr�:�� QrA$�� �r� A$��(8) � 15360�HU,32 : HU�HU�1 : � 1920 �r0� A$��(9) � 15360�HU,32 : HU�HU�1 : � 1920 �rD� A$��(91) � 15360�HU,32 : HU�HU�64 : � 1920 sX� A$��(10) � 15360�HU,32 : HU�HU�64 : � 1920 sl� 2080 Hs�� �(15360�HU)�32 � 15360�HU,88 : � 1800 es�� �(15360�HU)��79 � 2000 �s�� : � �(23) : �@320,"ZAPPED BY A ROBOT" �s�� X�1 � 6000 : � : � 260 �s�� : � �(23) : �@ 320,"ZAPPED BY THE FENCE" : � 1980 �s�:�� t�� ** ROBOTS MAKE A MOVE ** t:�� &t I � �(10�RO) :t4� I � RO � 1800 HtHJ � PO(I) Yt\� 15360�J,32 utpY1��(J�64) : X1�J�64�Y1 �t�Y2��(HU�64) : X2�HU�64�Y2 �t�J1�J�64��(Y2�Y1)��(X2�X1) �t�� �(15360�J1)�32 � 15360�J1,79 : PO(I)�J1 : � 1800 u�� �(15360�J1)�88 � 1960 u�� 2460 !u�� RO�0 � 1800 1u�� : � �(23) Iu	�@ 320,"YOU WIN!!!" Tu$	� 1980 Zu8	� ouL	:��

SUBROUTINES wu`	:�� �ut	� ** DELETE A ROBOT FROM LIST ** �u�	:�� �u�	� I1�I�1 � RO �u�	PO(I1�1)�PO(I1) �u�	� �u�	RO�RO�1 �u�	� �u 
:�� �u
� �u(
�   