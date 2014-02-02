N:Mittelsenkrechte (2 Punkte)
P:Point_1
L:Erster Punkt
T:point
P:Point_2
L:Zweiter Punkt
T:point
S:"Circle_1"(hidden)=circle("Point_1","Point_2");
S:"Circle_2"(hidden)=circle("Point_2","Point_1");
S:"Cut_2"(hidden),"Cut_1"(hidden)=intersection("Circle_1","Circle_2");
S:"Line_1"()=line("Cut_2","Cut_1");
