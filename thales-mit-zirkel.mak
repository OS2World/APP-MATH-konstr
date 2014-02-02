N:Thaleskreis (nur mit Zirkel)
P:C
L:W„hlen Sie einen Punkt als Parameter!
T:point
P:D
L:W„hlen Sie einen Punkt als Parameter!
T:point
S:"Kreis_5"(hidden)=circle("D","C");
S:"Kreis_6"(hidden)=circle("C","D");
S:"Schnitt_8"(hidden),"Schnitt_7"(hidden)=intersection("Kreis_5","Kreis_6");
S:"Kreis_7"(hidden)=circle("Schnitt_7","D");
S:"Schnitt_10"(hidden),"Schnitt_9"(hidden)=intersection("Kreis_7","Kreis_5");
S:"Kreis_8"(hidden)=circle("Schnitt_10","Schnitt_7");
S:"Schnitt_12"(hidden),"Schnitt_11"(hidden,symbol:circle)=intersection("Kreis_5","Kreis_8");
S:"Kreis_9"(hidden)=circle("Schnitt_11","C");
S:"Schnitt_14"(hidden),"Schnitt_13"(hidden)=intersection("Kreis_6","Kreis_9");
S:"Kreis_10"(hidden)=circle("Schnitt_13","C");
S:"Kreis_11"(hidden)=circle("Schnitt_14","C");
S:"Schnitt_16"(hidden),"Schnitt_15"(hidden,symbol:circle)=intersection("Kreis_10","Kreis_11");
S:"Kreis_12"()=circle("Schnitt_15","C");
