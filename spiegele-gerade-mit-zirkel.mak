N:Gerade an Kreis spiegeln
P:Punkt_3
L:Erster Punkt auf Gerade
T:point
P:Punkt_4
L:Zweiter Punkt auf Gerade
T:point
P:Punkt_1
L:Kreismittelpunkt
T:point
P:Kreis_1
L:Kreis
T:circle
S:"Kreis_29"(hidden)=circle("Punkt_3","Punkt_1");
S:"Kreis_30"(hidden)=circle("Punkt_1","Punkt_3");
S:"Schnitt_40"(hidden),"Schnitt_39"(hidden)=intersection("Kreis_29","Kreis_30");
S:"Kreis_31"(hidden)=circle("Schnitt_39","Punkt_3");
S:"Schnitt_42"(hidden),"Schnitt_41"(hidden)=intersection("Kreis_31","Kreis_29");
S:"Kreis_32"(hidden)=circle("Schnitt_42","Schnitt_39");
S:"Schnitt_44"(hidden),"Schnitt_43"(hidden,symbol:circle)=intersection("Kreis_29","Kreis_32");
S:"Kreis_33"(hidden)=circle("Schnitt_43","Punkt_1");
S:"Schnitt_46"(hidden),"Schnitt_45"(hidden)=intersection("Kreis_30","Kreis_33");
S:"Kreis_34"(hidden)=circle("Schnitt_45","Punkt_1");
S:"Kreis_35"(hidden)=circle("Schnitt_46","Punkt_1");
S:"Schnitt_48"(hidden),"Schnitt_47"(hidden,symbol:circle)=intersection("Kreis_34","Kreis_35");
S:"Kreis_36"(hidden)=circle("Schnitt_47","Punkt_1");
S:"Kreis_37"(hidden)=circle("Punkt_4","Punkt_1");
S:"Kreis_38"(hidden)=circle("Punkt_1","Punkt_4");
S:"Schnitt_50"(hidden),"Schnitt_49"(hidden)=intersection("Kreis_37","Kreis_38");
S:"Kreis_39"(hidden)=circle("Schnitt_49","Punkt_4");
S:"Schnitt_52"(hidden),"Schnitt_51"(hidden)=intersection("Kreis_39","Kreis_37");
S:"Kreis_40"(hidden)=circle("Schnitt_52","Schnitt_49");
S:"Schnitt_54"(hidden),"Schnitt_53"(hidden,symbol:circle)=intersection("Kreis_37","Kreis_40");
S:"Kreis_41"(hidden)=circle("Schnitt_53","Punkt_1");
S:"Schnitt_56"(hidden),"Schnitt_55"(hidden)=intersection("Kreis_38","Kreis_41");
S:"Kreis_42"(hidden)=circle("Schnitt_55","Punkt_1");
S:"Kreis_43"(hidden)=circle("Schnitt_56","Punkt_1");
S:"Schnitt_58"(hidden),"Schnitt_57"(hidden,symbol:circle)=intersection("Kreis_42","Kreis_43");
S:"Kreis_44"(hidden)=circle("Schnitt_57","Punkt_1");
S:"Schnitt_60"(hidden),"Schnitt_59"(hidden)=intersection("Kreis_44","Kreis_36");
S:"Kreis_45"(hidden)=circle("Schnitt_60","Punkt_1");
S:"Schnitt_62"(hidden),"Schnitt_61"(hidden)=intersection("Kreis_1","Kreis_45");
S:"Kreis_46"(hidden)=circle("Schnitt_61","Punkt_1");
S:"Kreis_47"(hidden)=circle("Schnitt_62","Punkt_1");
S:"Schnitt_64"(hidden),"Schnitt_63"(hidden,symbol:circle)=intersection("Kreis_46","Kreis_47");
S:"Kreis_48"(hidden)=circle("Schnitt_63","Punkt_1");
S:"Kreis_49"(hidden)=circle("Punkt_1","Schnitt_63");
S:"Schnitt_66"(hidden),"Schnitt_65"(hidden)=intersection("Kreis_48","Kreis_49");
S:"Kreis_50"(hidden)=circle("Schnitt_65","Schnitt_63");
S:"Schnitt_68"(hidden),"Schnitt_67"(hidden)=intersection("Kreis_50","Kreis_48");
S:"Kreis_51"(hidden)=circle("Schnitt_68","Schnitt_65");
S:"Schnitt_70"(hidden),"Schnitt_69"(hidden,symbol:circle)=intersection("Kreis_48","Kreis_51");
S:"Kreis_52"(hidden)=circle("Schnitt_69","Punkt_1");
S:"Schnitt_72"(hidden),"Schnitt_71"(hidden)=intersection("Kreis_49","Kreis_52");
S:"Kreis_53"(hidden)=circle("Schnitt_71","Punkt_1");
S:"Kreis_54"(hidden)=circle("Schnitt_72","Punkt_1");
S:"Schnitt_74"(hidden),"Schnitt_73"(hidden,symbol:circle)=intersection("Kreis_53","Kreis_54");
S:"Kreis_55"()=circle("Schnitt_73","Punkt_1");