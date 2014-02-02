N:Winkelhalbierende
P:Punkt_2
L:Scheitelpunkt
T:point
P:Punkt_1
L:Erster Winkelpunkt
T:point
P:Punkt_3
L:Zweiter Winkelpunkt
T:point
S:"Strahl_5"(hidden)=ray("Punkt_2","Punkt_3");
S:"Kreis_4"(hidden)=circle("Punkt_2","Punkt_1");
S:"Schnitt_6"(hidden),"Schnitt_5"()=intersection("Strahl_5","Kreis_4");
S:"Kreis_5"(hidden)=circle("Punkt_1","Schnitt_6");
S:"Kreis_6"(hidden)=circle("Schnitt_6","Punkt_1");
S:"Schnitt_8"(hidden),"Schnitt_7"(hidden)=intersection("Kreis_6","Kreis_5");
S:"Gerade_1"()=line("Schnitt_8","Schnitt_7");
