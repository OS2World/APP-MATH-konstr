N:Umkreis (3 Punkte)
P:Punkt_1
L:W„hlen Sie einen Punkt als Parameter!
T:point
P:Punkt_2
L:W„hlen Sie einen Punkt als Parameter!
T:point
P:Punkt_3
L:W„hlen Sie einen Punkt als Parameter!
T:point
S:"Kreis_1"(hidden)=circle("Punkt_1","Punkt_2");
S:"Kreis_2"(hidden)=circle("Punkt_2","Punkt_1");
S:"Schnitt_2"(hidden),"Schnitt_1"(hidden)=intersection("Kreis_1","Kreis_2");
S:"Gerade_1"(hidden)=line("Schnitt_2","Schnitt_1");
S:"Kreis_3"(hidden)=circle("Punkt_2","Punkt_3");
S:"Kreis_4"(hidden)=circle("Punkt_3","Punkt_2");
S:"Schnitt_4"(hidden),"Schnitt_3"(hidden)=intersection("Kreis_3","Kreis_4");
S:"Gerade_2"(hidden)=line("Schnitt_4","Schnitt_3");
S:"Schnitt_5"(symbol:circle)=intersection("Gerade_1","Gerade_2");
S:"Kreis_5"()=circle("Schnitt_5","Punkt_2");
