.********************************************************
:userdoc.
:docprof toc=123.
:title.Zirkel und Lineal fr OS/2
.******************************************
:h1 res=100. šberblick
:p.
Z.u.L. steht fr Konstruktionen mit Zirkel und Lineal. In
der Tat simuliert dieses Programm Konstruktionen mit Zirkel und Lineal 
am Rechner. Der Vorteil ist offensichtlich. Es ist z.B. m”glich
:ul.
:li. Konstruktionspunkte zu verschieben und die Wirkung auf die 
Konstruktion zu beobachten.
:li. Ortslinien anzuzeigen.
:li. Konstruktionsdetails zu verstecken und anzuzeigen nach Wunsch.
:li. Makros fr kompliziertere Konsruktionsschritte zu verwenden.
:eul.:p.
All dies ist offensichtlich nur am Rechner m”glich, was den Reiz
dieses Programms ausmacht.
:p.
Starten Sie das Programm aus einer Shell oder mit Hilfe eines
Programm-Objekts. Sie k”nnen auch eine Konstruktion (ein Datei
mit Erweiterung .zul) auf das Programm-Objekt ziehen.
:p.
Das Programm verwendet OS/2 und sein Hilfesystem. Die Schnittstelle
besteht aus dem Konstruktionsfenster und der Toolbox. Fr weitere
Informationen lesen Sie bitte.
:ul compact.
:li. :link reftype=hd res=200.Das Konstruktionsfenster.:elink.
:li. :link reftype=hd res=300.Die Toolbox.:elink.
:li. :link reftype=hd res=400.Die Mens.:elink.
:li. :link reftype=hd res=450.Makros.:elink.
:li. :link reftype=hd res=500.Die Tastatur.:elink.
:li. :link reftype=hd res=1000.šber den Autor.:elink.
:eul.
.******************************************************
:h1 res=200. Das Konstruktionsfenster.
:i1. Konstruktionsfenster
:i1. Mausbenutzung
:p.
In diesem Fenster wird Ihre Konstruktion angezeigt. Versteckte Objekte
werden allerdings nur angezeigt, wenn die ensprechende Option im Men
ausgew„hlt ist (F10). Das Fenster ist scrollbar. Zum Vergr”áern oder
verkleinern w„hlen Sie bitte die entsprechenden Menpunkte.
:p.
Nach Auswahl eines Werkzeugs k”nnen Sie mit der linken Maustaste
in diesem Fenster Punkte erzeugen, Objekte ausw„hlen, Punkte
verschieben etc.
:p.
Mit der rechten Maustaste k”nnen Sie Objekte verschieben (genau wie
mit dem Verschiebe-Werkzeug). Ein Mausklick mit der rechten Taste,
bei dem die Maus nicht verschoben wird, ruft das Edit-Werkzeug fr
das gew„hlte Objekt auf.
:p.
Die Werkzeugauswahl per Tastatur funktioniert auch, wenn dieses
Fenster aktiv ist. (:link reftype=hd res=500.Tastatur:elink.)
.******************************************************
:h1 res=300. Die Toolbox.
:i1. Toolbox
:p.
In diesem Fenster werden die m”glichen Werkzeuge (Tools) als
Sinnbilder dargestellt. Die Auswahl erfolgt mit der Maus oder durch
Drcken einer Buchstabentaste (Abschnitt ber Verwendung der
:link reftype=hd res=500.Tastatur:elink.).
:p.
Auáerdem wird erl„uternder Text zum gerade ausgew„hlten Werkzeug
angezeigt. Dieser Text kann sich w„hrend der Arbeit mit dem Werkzeug
„ndern. Er gibt meist Aufschluá ber die Aktion, die vom Benutzer
erwartet wird.
:p.
Sie k”nnen die Werkzeuge mit der Tastatur ausw„hlen, auch wenn das
Konstruktionsfenster aktiv ist. (:link reftype=hd res=500.Tastatur:elink.)
:p.
Es folgt eine Beschreibung der zur Verfgung stehenden Werkzeuge.
:dl compact.
:dt.:artwork name='carbmp\point.bmp'.
:dd.Erzeugt einen Punkt.
:dt.:artwork name='carbmp\line.bmp'.
:dd.Erzeugt eine Gerade aus zwei Punkten.
:dt.:artwork name='carbmp\ray.bmp'.
:dd.Erzeugt einen Strahl aus einem Anfangspunkt und einem anderen
Punkt.
:dt.:artwork name='carbmp\segment.bmp'.
:dd.Erzeugt eine Strecke aus einem Anfangs- und einem Endpunkt.
:dt.:artwork name='carbmp\circle.bmp'.
:dd.Erzeugt einen Kreis aus dem Mittelpunkt und einem Punkt
auf der Peripherie.
:dt.:artwork name='carbmp\move.bmp'.
:dd.Bewegt einen Punkt. Dieser Punkt darf natrlich nicht
aus anderen Objekten konstruiert sein (als Schnitt). Bewegen
Sie den Punkt durch Ziehen mit der Maus. Dieses Werkzeug
wird auch durch ziehen mit der rechten Maustaste aufgerufen.
:dt.:artwork name='carbmp\moveon.bmp'.
:dd.Bewegt einen Punkt auf einem Objekt vom Geraden- oder
Kreistyp. Zun„chst muá hier das Objekt angegeben werden.
Danach kann ein Punkt bewegt werden. Dieses Werkzeug dient
dazu, Ortslinien zu erzeugen. So kann etwa studiert werden,
sie sich ein Punkt „ndert, wenn ein anderer auf einer Geraden
bewegt wird.
:dt.:artwork name='carbmp\pointon.bmp'.
:dd.Erzeugt einen Punkt auf einem Objekt vom Geraden- oder Kreistyp.
Der Punkt kann bewegt werden. Er wird jedoch immer auf das Objekt
projiziert.
:dt.:artwork name='carbmp\intersect.bmp'.
:dd.Schneidet zwei Objekte. Es werden ein oder zwei Punkte erzeugt.
Schnittpunkte k”nnen ungltig werden, wenn die Objekte sich nicht mehr
schneiden. Dann werden auch alle abh„ngigen Objekte ungltig.
:dt.:artwork name='carbmp\track.bmp'.
:dd.Erzeugt eine Ortlinie eines Punktes, wenn ein anderer Punkt bewegt
wird. W„hlen Sie zun„chst den Punkt, dessen Ortslinie gezeichnet
werden soll und bewegen Sie dann einen Punkt. Die Ortlinie kann mit 
dem Knopf am rechten Rand der Toolbox gel”scht werden. Es k”nnen
Ortslinien von mehreren Punkten gezeichnet werden.
:dt.:artwork name='carbmp\hide.bmp'.
:dd.Versteckt ein Objekt. Falls die versteckten Objekte angezeigt
werden, so ist es hiermit auch m”glich, versteckte Objekte
wieder normal darzustellen.
:dt.:artwork name='carbmp\delete.bmp'.
:dd.L”scht ein Objekt und alle davon abh„ngigen Objekte. Also wird
etwa mit einem Punkt auch jede Gerade, die mit Hilfe dieses Punktes
konstruiert ist, gel”scht.
:dt.:artwork name='carbmp\edit.bmp'.
:dd.Mit diesem Werkzeug k”nnen verschiedene Objekteinstellungen
ver„ndert werden (Farbe, Versteckt-Status, etc.). Das Werkzeug
kann auch durch Mausklick auf ein Objekt mit der rechten Taste
ausgew„hlt werden.
:dt.:artwork name='carbmp\angle.bmp'.
:dd.Erzeugt einen Winkel, der durch drei Punkte definiert ist. Dieser
dient nur optischen Zwecken. Mit
ihm kann nichts konstruiert werden. Der Winkel ist immer mathematisch
positiv orientiert.
:edl.:p.
:hp2.Bemerkung &colon.:ehp2. Die folgenden Werkzeuge k”nnen durch
reine Konstruktionen mit Zirkel und Lineal ersetzt werden. Sie bieten
aber eine enorme Erleichterung bei komplexen Konstruktionen.
:dl compact.
:dt.:artwork name='carbmp\lot.bmp'.
:dd.Erzeugt ein Lot vom einem Punkt auf eine Gerade.
:dt.:artwork name='carbmp\parallel.bmp'.
:dd.Zieht eine Parallele durch einen Punkt zu einer Geraden.
:dt.:artwork name='carbmp\circle3.bmp'.
:dd.Erzeugt einen Kreis aus einem Mittelpunkt, und zwei Punkten deren
Abstand den Radius angibt.
:dt.:artwork name='carbmp\middle.bmp'.
:dd.Erzeugt den Mittelpunkt zwischen zwei Punkten.
:edl.:p.
Auáerdem gibt es noch folgende Schalter.
:dl compact.
:dt.:artwork name='carbmp\deltrack.bmp'.
:dd.L”scht alle Ortslinien.
:dt.:artwork name='carbmp\delhide.bmp'.
:dd.Zeigt alle versteckten Objekte an.
:dt.:artwork name='carbmp\colors.bmp'.
:dd.Setzt die Zeichenfarbe fr zuknftige Objekte.
:dt.:artwork name='carbmp\undo.bmp'.
:dd.L”scht das letzte Objekt.
:edl.
.******************************************************
:h1 res=400. Die Mens.
:i1. Mens
:p.
Alle Menpunkte sind eigentlich selbsterkl„rend. Viele k”nnen mit einem
Tastaturkommando angew„hlt werden. Diese Kommandos werden im Mentext
angegeben.
.******************************************************
:h1 res=450. Makros
:i1. Makros
:p.
Makros erlauben es, komplizierte Konstruktionsschritte in einem Rutsch 
durchzufhren. Dazu muá das Makro definiert werden. Die Definition erfolgt 
durch Kopieren einiger Schritte aus der aktuellen Konstruktion.
:p.
W„hlen Sie dazu die Parameter- und die Zielobjekte. Die Typen der Parameter 
(Punkt, Linie oder Kreis) werden im Makro gespeichert. Alle 
Konstruktionsschritte, die n”tig sind, um die Zielobjekte aus den 
Parametern zu konstruieren, werden ebenfalls gespeichert. Sie k”nnen zudem 
fr jeden Parameter einen eigenen Prompt eingeben. Dieser Text erscheint, 
wenn der Benutzer des Makros den Parameter w„hlen muá.
:p.
Bei Aufruf eines Makros mssen nur die Parameter aus der Konstruktion 
gew„hlt werden. Dann werden die Konstruktionsschritte ausgefhrt.
:p.
Beachten Sie, daá alle Details, wie etwa die Farbe und die 
Punktdarstellung, gespeichert werden. Sie wollen daher eventuell die 
Zwischenschritte des Makros vestecken, bevor Sie das Makro definieren.
:p.
Auáerdem ist zu beachten, daá durch die Angabe etwa einer Strecke nicht
automatisch die Endpunkte als Parameter festgelegt sind. Als Parameter 
mssen daher die Endpunkte eingegeben werden.
.******************************************************
:h1 res=500. Die Tastatur.
:i1. Tastatur
:p.
Die Werkzeuge k”nnen mit der Tastatur ausgew„hlt werden. Dies funktioniert
auch dann, wenn das Konstruktionsfenster aktiv ist. Die Tastencodes sind
von links nach rechts "qwertzuiop" und dann in der n„chsten Reihe "asdfgh".
Dies erschien mir einfacher, als nach sinnvollen Abkrzungen fr die
Werkzeuge zu suchen. Es ist jedoch auch m”glich, die Pfeiltasten rechts
und links zu benutzen, um durch die Werkzeuge zu scrollen.
:p.
Es folgt eine Liste der Tasten, die Aktionen in CaR ausl”sen.
:dl compact.
:dthd.:hp2.Taste:ehp2.
:ddhd.:hp2.Zweck:ehp2.
:dt.ESC
:dd.Setzt eine begonnene Aktion auf den Anfangszustand
zurck. Wenn etwa nach dem zweiten Punkt einer Strecke
gefragt wird, so wird die Auswahl des ersten Punktes
zurckgenommen und wieder nach dem ersten Punkt gefragt.
:dt.F1
:dd.Hilfe (auch in den Dialogfenstern)
:dt.F2
:dd.Index fr Hilfe.
:dt.F3
:dd.Hilfe fr Tasten (diese Seite).
:dt.F4
:dd.Inhaltsverzeichnis der Hilfe.
:dt.F5
:dd.Vergr”áern.
:dt.F6
:dd.Verkleinern.
:dt.F7
:dd.Makro ausfhren.
:dt.F8
:dd.Wahl der Default-Farbe fr neue Objekte.
:dt.F9
:dd.Zeige den Kommentar zur Konstruktion an.
:dt.F10
:dd.Zeige alle Objekte, bzw. zeige nur die nicht versteckten.
:dt.<--
:dd.L”scht das zuletzt konstruierte Objekt. Falls das Objekt unsichtbar
war, so kann man natrlich diesen L”schvorgang nicht mitverfolgen.
:dt.Entf
:dd.Diese Taste l”scht alle Ortslinien.
:dt.Shift-F7
:dd.Zentrieren des Fensters auf die Koordinate (0,0).
:dt.Shift-F10
:dd.W„hle Farben die absolut versteckt werden sollen.
:edl.
.******************************************************
:h1 res=600. Die Objektauswahl
:p.
Die Objektauswahl erscheint, wenn die Auswahl auf dem Konstruktionsfenster
mit der Maus nicht eindeutig war. W„hlen Sie das richtige Objekt in der 
Liste mit einem Doppelklick oder OK aus.
.******************************************************
:h1 res=1000. šber den Autor.
:i1. Autor
:artwork align=left name='rene.bmp'.
:p.
Das Programm wurde mit Hilfe einer eigengestrickten C++-Bibliothek
entwickelt. Es sollte Ihnen im Source-Code vorliegen. Fr Fragen und 
Anregungen stehe ich jederzeit zur Verfgung. Selbstverst„ndlich hafte
ich keiner Weise fr dieses Programm.
:xmp.
Dr. R. Grothmann
Ahornweg 5a
85117 Eitensheim

EMail&colon.
rene.grothmann@ku-eichstaett.de
:exmp.:p.
.****
:h1 res=2000 hide. Objektauswahl
:p.
W„hlen Sie hier das gewnschte Objekt mit Doppelklick aus. Oder 
drcken Sie OK.
.****
:h1 res=2010 hide. Objekteigenschaften
:p.
Hier k”nnen Sie die Eigenschaften fr das ausgew„hlte Objekte einstellen.
Nicht alle Felder sind ver„nderbar. So kann ein Punkt nur dann verschoben
werden, wenn er nicht konstruiert sondern eingegeben wurde.
:p.
Bewegliche Punkte lassen sich an ihrer Position fixieren. Geben Sie dazu 
die Koordinaten des Punktes ein und fixieren Sie ihn. Konstruierte Punkte 
k”nnen nicht fixiert werden.
.****
:h1 res=2020 hide. Fehler
:p.
In der Eingabedatei befindet sich ein Fehler. Dies sollte nur auftreten, 
wenn die Datei von Hand erstellt oder ver„ndert wurde.
:p.
Um die korrekte Syntax der Datei zu erlernen, studieren Sie einfach
Beispieldateien.
.****
:h1 res=2030 hide. Default-Farbe
:p.
Die Default-Farbe fr zuknftige Objekte wird hier eingestellt.
.****
:h1 res=2040 hide. Kommentar
:p.
Dieser Dialog erlaubt die Eingabe eines Kommentars. Der Kommentar wird
mit Ihrer Konstruktion abgespeichert.
.****
:h1 res=2050 hide. Farben
:p.
Sie k”nnen hier ausw„hlen, welche Objekte dargestellt werden sollen.
Es werden nur Objekte gezeigt, die eine der ausgew„hlten Farben
haben.
.****
:h1 res=2060 hide. Makroauswahl
:p.
W„hlen Sie eines der Makros aus. Dazu k”nnen Sie einfach auf das
Makro doppelklicken.
.****
:h1 res=2070 hide. Parameter Prompt
:p.
Diese Prompt erscheint, wenn bei der Ausfhrung des Makros dieser
Parameter eingegeben werden muá.
.****
:h1 res=2080 hide. Makroname
:p.
Jedes Makro hat einen Namen. Falls schon ein Makro mit demselben Namen
existiert, werden Sie gefragt, ob Sie das alte Makro l”schen wollen.
.****
:h1 res=2090 hide. Bitmap Speichern
:p.
Sie k”nnen die Grafik als Bitmap in beliebiger Gr”áe speichern. Falls Sie 
das Seitenverh„ltnis beibehalten, so brauchen Sie nur eine der Koordinaten 
zu „ndern.
.****
:h1 res=2100 hide. Gittereinstellung
:p.
Hier kann gew„hlt werden, ob die Achsen x=0 und y=0 angezeigt werden, falls 
das Gitter angezeigt wird. Der Abstand der Gitterlinien kann ebenfalls 
gew„hlt werden. Es gibt zwei verschiedene Gitterlinien, n„mlich 
durchgezogene und gepunktete.
.*************************************************
:euserdoc.
