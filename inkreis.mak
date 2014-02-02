N:Inkreis (3 Punkte)
P:Punkt_1
L:Erster Punkt
T:point
P:Punkt_2
L:Zweiter Punkt
T:point
P:Punkt_3
L:Dritter Punkt
T:point
S:"Strahl_5"(hidden)=ray("Punkt_2","Punkt_3");
S:"Kreis_4"(hidden)=circle("Punkt_2","Punkt_1");
S:"Schnitt_6"(hidden),"Schnitt_5"()=intersection("Strahl_5","Kreis_4");
S:"Kreis_5"(hidden)=circle("Punkt_1","Schnitt_6");
S:"Kreis_6"(hidden)=circle("Schnitt_6","Punkt_1");
S:"Schnitt_8"(hidden),"Schnitt_7"(hidden)=intersection("Kreis_6","Kreis_5");
S:"Gerade_1"(hidden)=line("Schnitt_8","Schnitt_7");
S:"Strahl_6"(hidden)=ray("Punkt_1","Punkt_3");
S:"Kreis_7"(hidden)=circle("Punkt_1","Punkt_2");
S:"Schnitt_10"(hidden),"Schnitt_9"()=intersection("Strahl_6","Kreis_7");
S:"Kreis_8"(hidden)=circle("Punkt_2","Schnitt_10");
S:"Kreis_9"(hidden)=circle("Schnitt_10","Punkt_2");
S:"Schnitt_12"(hidden),"Schnitt_11"(hidden)=intersection("Kreis_9","Kreis_8");
S:"Gerade_2"(hidden)=line("Schnitt_12","Schnitt_11");
S:"Schnitt_13"(symbol:circle)=intersection("Gerade_2","Gerade_1");
S:"Gerade_3"(hidden)=line("Punkt_3","Punkt_1");
S:"Objekt_36"(hidden),"Gerade_4"(hidden)=rectangular("Schnitt_13","Gerade_3");
S:"Schnitt_14"(hidden)=intersection("Gerade_3","Gerade_4");
S:"Kreis_10"()=circle("Schnitt_13","Schnitt_14");
