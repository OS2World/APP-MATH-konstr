N:Verdoppele (nur mit Zirkel)
P:A
L:W„hlen Sie einen Punkt als Parameter!
T:point
P:B
L:W„hlen Sie einen Punkt als Parameter!
T:point
S:"Kreis_1"(hidden)=circle("B","A");
S:"Kreis_2"(hidden)=circle("A","B");
S:"Schnitt_2"(hidden),"Schnitt_1"(hidden)=intersection("Kreis_1","Kreis_2");
S:"Kreis_3"(hidden)=circle("Schnitt_1","B");
S:"Schnitt_4"(hidden),"Schnitt_3"(hidden)=intersection("Kreis_3","Kreis_1");
S:"Kreis_4"(hidden)=circle("Schnitt_4","Schnitt_1");
S:"Schnitt_6"(hidden),"Schnitt_5"(symbol:circle)=intersection("Kreis_1","Kreis_4");
