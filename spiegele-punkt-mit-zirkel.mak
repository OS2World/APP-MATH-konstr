N:Punkt an Kreis Spiegeln (mit Zirkel)
P:Punkt_1
L:Mittelpunkt des Kreises
T:point
P:Kreis_1
L:Kreis
T:circle
P:Punkt_3
L:Punkt
T:point
S:"Kreis_2"(hidden)=circle("Punkt_3","Punkt_1");
S:"Schnitt_2"(hidden),"Schnitt_1"(hidden)=intersection("Kreis_1","Kreis_2");
S:"Kreis_3"(hidden)=circle("Schnitt_1","Punkt_1");
S:"Kreis_4"(hidden)=circle("Schnitt_2","Punkt_1");
S:"Schnitt_4"(hidden),"Schnitt_3"(symbol:circle)=intersection("Kreis_3","Kreis_4");
