#include "easy.h"
#include "car.h"

#include <math.h>
#include <ctype.h>

Program program;

const int maxcomment=2048;

String loadname("");

FontSelector fontselector;

Profile profile("CAR");

int colorprint=0;
int printsize=80;
String printername("");
int blackwhite=0;

int grid=0;
int axes=1;
double grid1=0.1;
double grid2=1.0;

// ***** Some help functions **************

char *stripnl (char *p)
{	long size=strlen(p);
	if (size>0 && p[size-1]=='\n') p[size-1]=0;
	return p;
}

// *********** Counters **************************

class Count;
class Counts
{	Count *First;
	public :
	Counts () : First(0) {}
	void add (Count *c);
	void reset ();
} counts;

class Count
{	long C;
	Count *Next;
	public :
	Count () : C(1),Next(0) { counts.add(this); }
	Count (int start) : C(start),Next(0) {}
	void reset () { C=1; }
	long count () { C++; return C-1; }
	void next (Count *n) { Next=n; }
	Count *next () { return Next; }
};

void Counts::add (Count *c)
{	c->next(First);
	First=c;
}

void Counts::reset ()
{	Count *c=First;
	while (c)
	{	c->reset();
		c=c->next();
	}
}

//*************** Translation of Names **************

class TranslationList
{	String Original,Pseudo;
	TranslationList *Next;
	public :
	TranslationList (char *o, char *ps) :
		Next(0),Original(o),Pseudo(ps)
	{}
	~TranslationList ()
	{	if (Next) delete Next;
	}
	void next (TranslationList *n) { Next=n; }
	TranslationList *next () { return Next; }
	void append (TranslationList *n)
	{	if (!Next) Next=n;
		else Next->append(n);
	}
	char *translate (char *ps);
};

TranslationList *translation=0;

// ******* Action classes, handle user click in window ********

class Action // generic
{	public :
	Flag drag; // drag action or click action
	Action () {}
	virtual void perform (long x, long y) {}
		// called, if the user clicked with botton 1
	virtual void dragperform (int type, long x, long y) {}
		// called, if the user drags with the mouse and if
		// supported by the specific action
	virtual void inform () {}
		// inform user through toolbox
	virtual void start () {}
		// called, if the user chooses the action
	virtual void abort () {}
		// called, if the user chooses a different action
	virtual int finished () { return 1; }
};

class NoAction : public Action // unimplemented action
{	public :
	virtual void inform ();
} noaction;

// ******* Window class ********

class MyBitmapPS : public BitmapPS
{	public :
	Flag invalid;
	MyBitmapPS (Window &w) : BitmapPS(w),invalid(0) {}
};

MyBitmapPS *bitmap=0;
	// Pointer to window bitmap presentation space
	// for keeping the drawing

class MainWindow : public StandardWindow
{	double X1,X2,Y1,Y2, // math coordinates
		Pixel, // pixel size in math coordinates
		Xm,Ym; // midpoint of window
	Action *A; // present action called through click handler
	Action *OldA; // note the previous action
	void compute (); // fit the coordinates to window size
	public :
	Flag showhidden, // show hidden objects or not
		active, // window can redraw objects
		square, // window must be square
		sizing; // windows does already react to resizing
	MainWindow () :
		StandardWindow(ID_Window,String(1),
			StandardWindow::normal|
			StandardWindow::menu|
			StandardWindow::keys|
			StandardWindow::vscrollbar|
			StandardWindow::hscrollbar),
			showhidden(0),active(0),sizing(0)
	{   init();
		X1=-1.001; X2=1.001; Y1=-1.001; Y2=1.001;
		A=&noaction;
		compute();
		settitle();
	}
	virtual void sized ();
	virtual void redraw (PS &ps);
	virtual void clicked (LONG x, LONG y, int click,
		int state);
		// click handler
	virtual int key (int flags, int code, int scan);
		// key handler
	long col (double x, PS &ps)
	{	return (x-X1)/(X2-X1)*ps.width();
	}
	long row (double y, PS &ps)
	{	return (y-Y1)/(Y2-Y1)*ps.height();
	}
	double x (long col)
	{	return (double)col/width()*(X2-X1)+X1;
	}
	double y (long row)
	{	return (double)row/height()*(Y2-Y1)+Y1;
	}
	void abort ()
	{	A->abort(); A->start(); A->inform();
	}
	void action (Action &a)
	{	OldA=A; A->abort(); A=&a; a.start(); a.inform();
	}
	void old ()
	{	if (!OldA) return;
		A=OldA; A->start(); A->inform();
	}
	double x1 () { return X1; }
	double x2 () { return X2; }
	double y1 () { return Y1; }
	double y2 () { return Y2; }
	double pixel (PS &ps) { return (X2-X1)/ps.width(); }
	double pixel () { return Pixel; }
	virtual int vscrolled (int scroll, int pos);
	virtual int hscrolled (int scroll, int pos);
	void zoomin ();
	void zoomout ();
	void center ();
	void setsize (double x1, double x2, double y1, double y2);
	virtual void pres_changed () { sized(); update(); }
	void settitle ();
	virtual int close ();
	virtual void semaphore (int n, long bits)
	{	if (n==1)
		{	Warning(String(81),String(1),*this);
			return;
		}
	}
} window;

Menu menu(window);
Help help(window,ID_Helptable,String(19),String(20));

// ********* Dialog for Multiple Object Selection ***

Dialog objectsdialog(window,IDD_Objects,help,2000); // the dialog

class ObjectsItem : public ListItem // list box handling class
{	public :
	ObjectsItem() : ListItem(ID_Objects,objectsdialog) {}
	virtual void init ();
	virtual void notify ();
} objectsitem;

// ******* Input/Output class **********************

const int maxstrings=8;

class Strings
{	String S[maxstrings];
	int N;
	public :
	Strings () { N=0; }
	int add (char *text);
	String & operator [] (int i) { return S[i]; }
	String * s () { return S; }
	int n () { return N; }
	void clear () { N=0; }
};

const int maxleft=8;

int Strings::add (char *text)
{   if (N>=maxstrings) return 0;
	if (strlen(text)+1>S[N].size()) return 0;
	strcpy(S[N++],text);
	return 1;
}

class Object;
class Input
{	Strings Left[maxleft];
	int Nleft;
	String Name;
	Strings Right;
	public :
	Input () { Nleft=0; }
	char *getstring (String &s, char *p);
	int read (char *line);
	int nleft () { return Nleft; }
	Strings * left () { return Left; }
	int nright () { return Right.n(); }
	String *right () { return Right.s(); }
	char *name () { return Name; }
	Object *object (int n, int flags);
	int set (Object &o, int n);
} input;

int Input::read (char *l)
{   String s;
	Nleft=0;
	while (1)
	{	if (Nleft>=maxleft) return 0;
		Left[Nleft].clear();
		l=getstring(s,l); Left[Nleft].add(s);
		if (*l++!='(') return 0;
		if (*l!=')')
			while (1)
			{   l=getstring(s,l);
				if (!Left[Nleft].add(s)) return 0;
				if (*l==')') break;
				if (*l++!=',') return 0;
			}
		l++;
		Nleft++;
		if (*l=='=') break;
		if (*l++!=',') return 0;
	}
	l++;
	l=getstring(Name,l);
	if (*l++!='(') return 0;
	Right.clear();
	while (1)
	{	l=getstring(s,l); if (!Right.add(s)) return 0;
		if (*l==')') break;
		if (*l++!=',') return 0;
	}
	l++;
	if (*l!=';') return 0;
	return 1;
}

char *Input::getstring (String &s, char *p)
{	char *q=s;
	long length=0;
	while (*p==' ') p++;
	if (*p=='"')
	{   p++;
		while (*p && *p!='"')
		{	*q++=*p++; length++;
			if (length>=String::defaultsize-1) break;
		}
		if (*p=='"') p++;
	}
	else
	{	while (*p && *p!=',' && *p!=')' && *p!='=' && *p!='(')
		{	*q++=*p++; length++;
			if (length>=String::defaultsize-1) break;
		}
	}
	while (*p==' ') p++;
	*q=0;
	return p;
}

class Object;
class IO
	// serves as elder class for objects input and output
{   protected :
	char S[256];
	String Name;
	IO *Next;
	public :
	IO (char *name);
	char *end () { return S+strlen(S); }
	virtual char *write (Object &o);
		// write objectname(flags),...=ioname(...)
	void resadd (Object &o);
		// add name of object, plus flags
		// for use in reswrite
	virtual void reswrite (Object &o) {}
		// write left side in write
	void depadd (Object &o);
		// add name of object, for use in depwrite
	virtual void depwrite (Object &o) {}
		// write name of dependends ..., in write
	void comma ();
	IO *next () { return Next; }
	void next (IO *io) { Next=io; }
	char *name() { return Name; }
	virtual int interpret (Input &i)
		// interpret scanned input line
	{	return 0;
	}
} defaultio("unimplemented");
	// default IO is empty string

class IOList
	// A list of IO instances for reading from file
{	IO *First;
	public :
	IOList ()
	{	First=0;
	}
	IO *first () { return First; }
	void append (IO *io) { io->next(First); First=io; }
	int interpret (char *line);
	char *getstring (String &s, char *line);
} iolist;

IO::IO (char *name) : Name(name)
{	Next=0;
	iolist.append(this);
}

// ******** Objects, displayed in window ***********

class ObjectColors
{   static long Light[6],Dark[6];
	static char *Name[6];
	int Defcolor;
	int Chosen[6];
	public :
	void all ();
	void none ();
	ObjectColors () : Defcolor(0) { all(); }
	int defcolor () { return Defcolor; }
	void defcolor (int c) { Defcolor=c; }
	long light (int i) { return Light[i]; }
	long dark (int i) { return Dark[i]; }
	long highlight () { return Colors::red; }
	char *name (int i)
	{	if (i>=0 && i<6) return Name[i];
		else return Name[0];
	}
	int index (char *name);
	void choose (int i, int f) { Chosen[i]=f; }
	int chosen (int i) { return Chosen[i]; }
	long gridcolor () { return Colors::darkgray; }
} objectcolors;

int ObjectColors::index (char *name)
{	int i;
	for (i=0; i<6; i++)
		if (!strcmp(name,Name[i])) return i;
	return 0;
}

void ObjectColors::all ()
{	for (int i=0; i<6; i++) Chosen[i]=1;
}

void ObjectColors::none ()
{	for (int i=0; i<6; i++) Chosen[i]=0;
}

long ObjectColors::Light[6]=
{	Colors::darkgray,Colors::blue,
	Colors::pink,Colors::green,
	Colors::cyan,Colors::yellow
};

long ObjectColors::Dark[6]=
{	Colors::black,Colors::darkblue,
	Colors::darkpink,Colors::darkgreen,
	Colors::darkcyan,Colors::brown
};

char *ObjectColors::Name[6]=
{	"black","blue","pink","green","cyan","brown"
};

#define MAXDEP 3 // Maximal numbers of elders
class Object // generic object designed for list membership (heap)
{	Object *Next,*Previous; // chaining information
	int Class; // class flags (point,line,moveable,circle,angle)
	long Color; // color
	String Name; // unique name
	IO *Io; // pointer to output class
	public :
	Flag mark, // mark for several purposes
		valid, // object valid (intersections may become invalid)
		showname, // show name on screen
		ishidden; // if hidden from output
	FixFlag
		highlight; // if chosen for input
	enum { point=1,moveable=2,line=4,circle=8,angle=16,notfixed=32};
		// class flags
	Object ();
	virtual void deletetrack () {}
		// remove track from a point, if it has one
	~Object () { deletetrack(); }
	Object *next() { return Next; }
	Object *previous() { return Previous; }
	void next (Object *o) { Next=o; }
	void previous (Object *o) { Previous=o; }
	virtual void draw (PS &ps) {};
	int objectclass () { return Class; }
	void objectclass (int c) { Class=c; }
	virtual double distance (double x, double y) { return 1e10; }
		// compute distance from (x,y) to object
	virtual void update () {} // compute new position
	virtual int depmarked (); // any elders marked?
	virtual int dep (Object *o[]) { return 0; }  // retunr elders
	virtual Object *original () { return this; }
		// return original for intersections
	long color () // return color of object
	{	if (highlight) return objectcolors.highlight();
		else if (ishidden) return objectcolors.light(Color);
		else return objectcolors.dark(Color);
	}
	long truecolor () { return Color; }
	void setcolor (long c) { Color=c; }
	int hidden ();
		// object cannot be displayed,
		// because it is either hidden and hidden objects are
		// not shown, or it is invalid
	char *name() { return Name; }
	void setname (char *prefix, Count &count);
	void setname (char *name);
	void setname1 (char *name);
	void setio (IO &io) { Io=&io; }
	virtual int project (double &x, double &y) { return 1; }
	char *write () { return Io->write(*this); }
	virtual void writespecialflags (IO &io) {}
	int setflags (Strings &s); // for input from file
	virtual int setspecialflag (char *s) { return 0; }
	virtual void edit () {} // edit dialog for object
};

class ObjectList // list of generic objects
{	Object *First,*Last;
	String Comment;
	public :
	Flag changed;
	ObjectList () :
		changed(0),Comment("",maxcomment),
		First(0),Last(0)
		{}
	~ObjectList ();
	void draw (PS &ps);
	void add (Object *o);
	void remove (Object *o);
	void insert (Object *o, Object *after);
	int select (double x, double y, Object * &p,
		int flags=0, int select=1, int mayhighlight=1);
	void unmark (); // unmark all objects
	void mark (Object *p); // mark p and dependends
	void update (); // update all objects
	void unhighlight (); // update all objects
	void update (Object *p); // update p and dependends
	void redraw (); // redraw bitmap and copy to screen
	Object *first () { return First; }
	Object *last () { return Last; }
	void clear (); // remove all objects
	void clear (Object *o); // remove Object and dependends
	void deletetracks (); // remove Tracks from points
	void save (char *filename); // save objects to file
	void load (char *filename); // load objects from file
	Object *find (char *name, int flags=0, Object *last=0);
	String &comment () { return Comment; }
	void copy (); // copy to clipboard
	int empty () { return First==0; }
	int exists (char *name);
} objects;

Count objectcount;

Object::Object () : valid(1)
{	Next=0; Previous=0; Class=0; Color=objectcolors.defcolor();
	setname(String(2),objectcount);
	setio(defaultio);
	objects.add(this);
}

inline int Object::hidden ()
{	return ((ishidden && !window.showhidden) || !valid
		|| !objectcolors.chosen(Color));
}

void Object::setname (char *s)
{   if (objects.find(s)) return;
	setname1(s);
}

void Object::setname1 (char *s)
{	char *p=s;
	while (*p)
	{	if (*p=='"') *p='\'';
		p++;
	}
	Name.copy(s);
}

int Object::depmarked (void)
{	Object *o[MAXDEP];
	int i,n;
	n=dep(o);
	for (i=0; i<n; i++)
		if (o[i]->mark) return 1;
	return 0;
}

void Object::setname (char *prefix, Count &count)
{   char s[256];
	do
	{	sprintf(s,"%s_%ld",prefix,count.count());
	} while (objects.exists(s));
	setname(s);
}

ObjectList::~ObjectList ()
{	Object *p=First,*p1;
	while (p)
	{	p1=p;
		p=p->next();
		delete p1;
	}
}

void drawgrid (PS &ps)
{	long n;
	long col=objectcolors.gridcolor();
	double h;
	if (!grid) return;
	if (axes)
	{	if (window.x1()<=0 && window.x2()>=0)
		{	n=window.col(0,ps);
			ps.linetrunc(n-1,0,n-1,ps.height()-1,col);
			ps.linetrunc(n,0,n,ps.height()-1,col);
			ps.linetrunc(n+1,0,n+1,ps.height()-1,col);
		}
		if (window.y1()<=0 && window.y2()>=0)
		{	n=window.row(0,ps);
			ps.linetrunc(0,n-1,ps.width()-1,n-1,col);
			ps.linetrunc(0,n,ps.width()-1,n,col);
			ps.linetrunc(0,n+1,ps.width()-1,n+1,col);
		}
	}
	if (window.col(h,ps)+2>=window.col(h+grid1,ps)) return;
	h=floor(window.x1()/grid1)*grid1;
	ps.linetype(LineType::alternate);
	while (h<=window.x2())
	{   if (h>=window.x1())
		{	n=window.col(h,ps);
			ps.linetrunc(n,0,n,ps.height()-1,col);
		}
		h+=grid1;
	}
	h=floor(window.y1()/grid1)*grid1;
	while (h<=window.y2())
	{   if (h>=window.y1())
		{	n=window.row(h,ps);
			ps.linetrunc(0,n,ps.width()-1,n,col);
		}
		h+=grid1;
	}
	ps.linetype();
	h=floor(window.x1()/grid2)*grid2;
	while (h<=window.x2())
	{   if (h>=window.x1())
		{	n=window.col(h,ps);
			ps.linetrunc(n,0,n,ps.height()-1,col);
		}
		h+=grid2;
	}
	h=floor(window.y1()/grid2)*grid2;
	while (h<=window.y2())
	{   if (h>=window.y1())
		{	n=window.row(h,ps);
			ps.linetrunc(0,n,ps.width()-1,n,col);
		}
		h+=grid2;
	}
}

void ObjectList::draw (PS &ps)
{	Object *p=First;
	if (blackwhite) ps.notsupport(Support::color);
	else ps.support(Support::color);
	if (ps.supports(Support::color)) ps.erase();
	else
	{	ps.move(0,0);
		ps.area(ps.width(),ps.height(),0,Colors::white);
	}
	ps.clip(0,0,ps.width(),ps.height());
	drawgrid(ps);
	while (p)
	{	p->draw(ps);
		p=p->next();
	}
}

void ObjectList::add (Object *o)
{   if (Last) Last->next(o);
	o->previous(Last);
	o->next(0);
	Last=o;
	if (!First) First=Last;
	changed.set();
}

void ObjectList::remove (Object *o)
{	Object *n=o->next(),*p=o->previous();
	if (n) n->previous(p);
	if (p) p->next(n);
	if (Last==o) Last=p;
	if (First==o) First=n;
	delete o;
	changed.set();
}

void ObjectList::clear ()
{	Object *o=First,*on;
	while (o)
	{   on=o->next();
		delete o;
		o=on;
	}
	First=Last=0;
	changed.clear();
}

void ObjectList::clear (Object *o)
{   Object *p=First,*pn;
	o=o->original();
	mark(o);
	while (p)
	{   pn=p->next();
		if (p->mark) remove(p);
		p=pn;
	}
}

void ObjectList::insert (Object *o, Object *after)
{	Object *p=after->next();
	after->next(o);
	o->next(p);
	if (p) p->previous(o);
	o->previous(after);
	if (Last==after) Last=o;
}

void ObjectList::unmark ()
{	Object *p=First;
	while (p)
	{	p->mark.clear();
		p=p->next();
	}
}

void ObjectList::mark (Object *p)
{	unmark();
	p=p->original();
	p->mark.set();
	while (p)
	{	if (p->depmarked()) p->mark.set();
		p=p->next();
	}
}

void ObjectList::update ()
{	Object *p=First;
	while (p)
	{	p->update();
		p=p->next();
	}
}

void ObjectList::unhighlight ()
{	Object *p=First;
	while (p)
	{	p->highlight.clear();
		p=p->next();
	}
}

void ObjectList::update (Object *p)
{   mark(p);
	while (p)
	{	if (p->mark) p->update();
		p=p->next();
	}
	changed.set();
}

void ObjectList::redraw ()
{   if (bitmap)
	{	draw(*bitmap);
		bitmap->copy(WindowPS(window));
	}
}

int ObjectList::select (double x, double y, Object * &pret,
	int flags, int select, int mayhighlight)
{	double r=window.pixel()*4;
	Object *p=First,*ps=0;
	int count=0,n;
	objects.unmark();
	while (p)
	{	if ((!flags || (p->objectclass()&flags))
			&& p->valid && !p->hidden() && p->distance(x,y)<r
			&& (mayhighlight || !p->highlight))
		{   p->mark.set();
			if (pret && p==pret) return 1;
			ps=p;
			count++;
		}
		p=p->next();
	}
	if (ps) pret=ps;
	if (count==0) return 0;
	if (count==1) return count;
	if (!select) return count;
	objectsdialog.carryout();
	if (objectsdialog.result()!=DID_OK) return 0;
	n=objectsitem;
	p=objects.first();
	while (p && n>=0)
	{   if (p->mark)
		{	pret=p; n--;
		}
		p=p->next();
	}
	return count;
}

void ObjectsItem::init ()
{   int count=0;
	Object *p=objects.first();
	while (p)
	{	if (p->mark)
		{	insert(p->name());
			count++;
		}
		p=p->next();
	}
	select(count-1);
}

void ObjectsItem::notify ()
{	finish();
}

Object *ObjectList::find (char *name, int flags, Object *last)
{	Object *o=First;
	if (translation) name=translation->translate(name);
	if (!name) return 0;
	while (o!=last)
	{	if (!strcmp(name,o->name())
			&& (flags==0 || (o->objectclass()&flags))) return o;
		o=o->next();
	}
	return 0;
}

int ObjectList::exists (char *name)
{	Object *o=First;
	while (o!=last())
	{	if (!strcmp(name,o->name())) return 1;
		o=o->next();
	}
	return 0;
}

// ************* window routines, redraw etc. *********

void MainWindow::compute ()
{	Pixel=(X2-X1)/width();
	Xm=(X2+X1)/2;
	Ym=(Y2+Y1)/2;
	Y1=Ym-Pixel*height()/2;
	Y2=Ym+Pixel*height()/2;
	vscroll(50);
	hscroll(50);
}

void MainWindow::setsize (double x1, double x2, double y1, double y2)
{	if (x2<x1+1e-15 || y2<y1+1e-15) return;
	X1=x1; X2=x2; Y1=y1; Y2=y2;
	if ((y1-y2)/(x1-x2) < height()/width())
		size(width(),width()*(y2-y1)/(x2-x1));
	else
		size(height()*(x2-x1)/(y2-y1),height());
	compute();
}

void MainWindow::zoomin ()
{   Xm=(X2+X1)/2;
	X1=Xm-(X2-X1)/2/1.2;
	X2=Xm+(X2-X1)/2/1.2;
	compute();
	objects.redraw();
}

void MainWindow::zoomout ()
{   Xm=(X2+X1)/2;
	X1=Xm-(X2-X1)/2*1.2;
	X2=Xm+(X2-X1)/2*1.2;
	compute();
	objects.redraw();
}

void MainWindow::center ()
{	X1=-1;
	X2=1;
	compute();
	objects.redraw();
}

int MainWindow::vscrolled (int scroll, int pos)
{   double r=0;
	switch (scroll)
	{	case Scroll::up : r=0.1; break;
		case Scroll::down : r=-0.1; break;
		case Scroll::pageup : r=0.5; break;
		case Scroll::pagedown : r=-0.5; break;
		default : return 0;
	}
	r*=(Y2-Y1); Y1+=r; Y2+=r;
	compute();
	objects.redraw();
	objects.changed.set();
	return 1;
}

int MainWindow::hscrolled (int scroll, int pos)
{   double r=0;
	switch (scroll)
	{	case Scroll::right : r=0.1; break;
		case Scroll::left : r=-0.1; break;
		case Scroll::pageright : r=0.5; break;
		case Scroll::pageleft : r=-0.5; break;
		default : return 0;
	}
	r*=(X2-X1); X1+=r; X2+=r;
	compute();
	objects.redraw();
	objects.changed.set();
	return 1;
}

void MainWindow::sized ()
{   if (!sizing) return;
	compute();
	if (square && (width() != height()))
	{	size(height(),height()); return;
	}
	if (bitmap) delete bitmap;
	bitmap=new MyBitmapPS(*this);
	if (fontselector) bitmap->setfont(Font(fontselector));
	if (active) bitmap->invalid.set();
	objects.changed.set();
}

void MainWindow::redraw (PS &ps)
{   if (!active) return;
	if (!bitmap) { ps.erase(); return; }
	if (bitmap->invalid) objects.draw(*bitmap);
	bitmap->invalid.clear();
	bitmap->copy(ps);
}

void MainWindow::settitle ()
{	char s[256];
	strcpy(s,String(1));
	strcat(s," (");
	if (!loadname.empty()) strcat(s,loadname.filename());
	strcat(s,")");
	title(s);
}

// ******** Tracks ****************

const int tracksize=1024;

class Track
{	double X[tracksize],Y[tracksize];
	int Count;
	int Color;
	public :
	Track () { Count=0; Color=Colors::def; }
	void draw (PS &ps);
	void add (double &x, double &y);
	void start () { Count=0; }
};

void Track::draw (PS &ps)
{   int i;
	if (Count<2) return;
	ps.move(window.col(X[0],ps),window.row(Y[0],ps));
	for (i=1; i<Count; i++)
	{	ps.lineto(window.col(X[i],ps),window.row(Y[i],ps),Color);
	}
}

void Track::add (double &x, double &y)
{	if (Count==tracksize) return;
	X[Count]=x; Y[Count]=y; Count++;
}

// ********** Tool dialog definition ****************

class ToolDialog : public DialogPanel
{	public :
	ToolDialog (Window &window, int id) :
		DialogPanel(window,id,help,300) {}
	virtual int handler (int command);
	void inform (char *text) { settext(ID_Text,text); }
	virtual int key (int flags, int code, int scan);
} tools(window,IDD_Tools);

void dohidden ();
void docolor ();
void dobackspace ();

int ToolDialog::handler (int command)
{	switch(command)
	{	case ID_DeleteTracks :
			objects.deletetracks();
			objects.redraw();
			return 1;
		case ID_Hide :
			dohidden();
			return 1;
		case ID_Colors :
			docolor();
			return 1;
		case ID_Undo :
			dobackspace();
			return 1;
	}
	return 0;
}

// ********** unimplemented action ********

void NoAction::inform ()
{	tools.inform(String(4));
}

// ********** Points *******

class Pointprim : public Object // point object
{	double X,Y;
	int Symbol;
	static long Count;
	Track *T; // Track of the point
	public :
	Pointprim (double x=0, double y=0);
	double x() { return X; }
	double y() { return Y; }
	virtual void draw (PS &ps);
	virtual double distance (double x, double y);
	void set (double x, double y);
	virtual void deletetrack ()
	{	if (T) delete T;
		T=0;
		highlight.unfix();
		highlight.clear();
	}
	void starttrack ()
	{	if (!T) T=new Track;
		else T->start();
		T->add(X,Y);
		highlight.set();
		highlight.fix();
	}
	virtual void edit ();
	virtual int setspecialflag (char *s);
	virtual void writespecialflags (IO &io);
};

Count pointcount;

class Point : public Pointprim
{	public :
	Point (double x, double y) : Pointprim(x,y)
	{   objectclass(objectclass()|Object::moveable|Object::notfixed);
		setname(String(5),pointcount);
	}
};

class PointIO : public IO
{	public :
	PointIO () : IO("point") {}
	virtual void depwrite (Object &o);
	virtual int interpret (Input &i);
} pointio;

void PointIO::depwrite (Object &o)
{   Pointprim &p=(Pointprim &)o;
	sprintf(end(),"%g,%g",p.x(),p.y());
}

int PointIO::interpret (Input &i)
{	if (i.nleft()!=1 || i.nright()!=2) return 0;
	double x,y;
	if (!i.right()[0].todouble(x) || !i.right()[1].todouble(y))
		return 0;
	Point *P=new Point(x,y);
	return i.set(*P,0);
}

Pointprim::Pointprim (double x, double y) : Object()
{	X=x, Y=y; objectclass(Object::point);
	setio(pointio);
	Symbol=Markers::square;
	T=0;
}

void ObjectList::deletetracks ()
{	Object *o=First;
	while (o)
	{	o->deletetrack();
		o=o->next();
	}
}

void Pointprim::draw (PS &ps)
{   if (!valid || hidden()) return;
	ps.mark(window.col(X,ps),window.row(Y,ps),Symbol,color());
	if (T) T->draw(ps);
	if (showname)
	{	ps.move(window.col(X,ps)+ps.markersize()+1,window.row(Y,ps));
		ps.text(name(),color(),TA_LEFT,TA_BOTTOM);
	}
}

double Pointprim::distance (double x, double y)
{	double vx=x-X,vy=y-Y;
	return sqrt(vx*vx+vy*vy);
}

void Pointprim::set (double x, double y)
{	X=x; Y=y;
	if (T) T->add(x,y);
}

class PointAction : public Action // generate point
{	public :
	virtual void perform (long x, long y);
	virtual void inform ();
} pointaction;

void PointAction::inform ()
{	tools.inform(String(6));
}

void PointAction::perform (long c, long r)
{	double x=window.x(c),y=window.y(r);
	new Point(x,y);
	objects.redraw();
}

class PointColor : public ValuesetItem
{	public :
	PointColor (int id, Dialog &d, int r, int c)
		: ValuesetItem(id,d,r,c) {}
	virtual void init ();
};

void PointColor::init ()
{	int i;
	for (i=0; i<6; i++)
		setcolor(1,i+1,objectcolors.dark(i));
	ValuesetItem::init();
}

int radios[4] = { IDP_Square,IDP_Point,IDP_Circle,IDP_Cross };
int markertypes[4] = { Markers::square,
	Markers::dot,Markers::circle,Markers::cross };

class CheckEnabled : public CheckItem
{	int E;
	public :
	CheckEnabled (int id, Dialog &d, int n, int e)
		: CheckItem(id,d,n),E(e) {}
	virtual void init ()
	{	CheckItem::init();
		enable(E);
	}
};

void Pointprim::edit ()
{   int i;
	Dialog d(window,IDD_PointEdit,help,2010);
	StringItem nameitem(IDP_Name,d,name());
	PointColor coloritem(IDP_Color,d,1,truecolor()+1);
	CheckItem hideitem(IDP_Hidden,d,ishidden),
		shownameitem(IDP_ShowName,d,showname);
	StringItem lineitem(IDP_Line,d,stripnl(write()),512);
	lineitem.readonly();
	DoubleItem xitem(IDP_X,d,x()),yitem(IDP_Y,d,y());
	if (!(objectclass()&Object::moveable))
	{	xitem.readonly(); yitem.readonly();
	}
	for (i=0; i<4; i++)
		if (Symbol==markertypes[i]) break;
	if (i==4) i=0;
	RadioItem markeritem(radios,4,d,i+1);
	CheckEnabled fixed(ID_PointFixed,d,
		!(objectclass()&Object::notfixed),
		!!(objectclass()&Object::moveable));
	d.carryout();
	if (d.result()!=DID_OK) return;
	setname(nameitem);
	if (strcmp(nameitem,name()))
		Warning(String(48),String(1),window);
	if (objectclass()&Object::moveable) set(xitem,yitem);
	setcolor(coloritem.col()-1);
	ishidden=hideitem;
	showname=shownameitem;
	Symbol=markertypes[markeritem-1];
	if (objectclass()&Object::moveable)
	{	if (fixed) objectclass(objectclass()&~Object::notfixed);
		else objectclass(objectclass()|Object::notfixed);
	}
}

int Pointprim::setspecialflag (char *s)
{   if (!strncmp(s,"symbol:",7))
	{	if (!strcmp(s+7,"point")) Symbol=Markers::dot;
		else if (!strcmp(s+7,"cross")) Symbol=Markers::cross;
		else if (!strcmp(s+7,"square")) Symbol=Markers::square;
		else if (!strcmp(s+7,"circle")) Symbol=Markers::circle;
		else return 0;
		return 1;
	}
	return 0;
}

void Pointprim::writespecialflags (IO &io)
{   if (Symbol!=Markers::square)
	{   io.comma();
		strcat(io.end(),"symbol:");
		switch (Symbol)
		{	case Markers::dot : strcat(io.end(),"point"); break;
			case Markers::circle : strcat(io.end(),"circle"); break;
			case Markers::cross : strcat(io.end(),"cross"); break;
		}
	}
}

// ********** line routines ********

inline double max (double x, double y)
{	if (x>y) return x;
	return y;
}

class Lineprim : public Object // a line through two points
// this may actually be a line, a ray or a segment.
{   protected :
	double X,Y,X1,Y1,Vx,Vy,R;
	Pointprim *P1,*P2;
	public :
	Lineprim (Pointprim &p1, Pointprim &p2);
	double x() { return X; }
	double y() { return Y; }
	double x1() { return X1; }
	double y1() { return Y1; }
	double vx() { return Vx; }
	double vy() { return Vy; }
	virtual void draw (PS &ps);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P1; o[1]=P2; return 2;
	}
	virtual double distance (double x, double y);
	void intersect (Lineprim &L, Pointprim &P);
	double lambda (double x, double y);
		// det. lambda for point on line
	double length () { return R; }
	virtual int project (double &x, double &y);
	Pointprim *p1() { return P1; }
	Pointprim *p2() { return P2; }
	virtual void edit ();
	virtual int checkpoint (double x, double y) { return 1; }
		// Check if x,y is on the valid part of the ray or segment
};

Count linecount;

class Line : public Lineprim
{	public :
	Line (Pointprim &p1, Pointprim &p2) : Lineprim(p1,p2)
	{	setname(String(7),linecount);
	}
};

template<class Class>
class TwoPointIO : public IO
{	public :
	TwoPointIO (char *name) : IO(name) {}
	virtual void depwrite (Object &o)
	{	Class &l=(Class &)o;
		depadd(*l.p1()); comma(); depadd(*l.p2());
	}
	virtual int interpret (Input &i);
};

template<class Class> int TwoPointIO<Class>::interpret (Input &i)
{	if (i.nleft()!=1 || i.nright()!=2) return 0;
	Pointprim *p1=(Pointprim *)i.object(0,Object::point),
		*p2=(Pointprim *)i.object(1,Object::point);
	if (!p1 || !p2) return 0;
	Class *l=new Class(*p1,*p2);
	return i.set(*l,0);
}

TwoPointIO<Line> lineio("line");

Lineprim::Lineprim (Pointprim &p1, Pointprim &p2) : Object()
{	P1=&p1; P2=&p2;
	objectclass(Object::line);
	setio(lineio);
	update();
}

void Lineprim::update ()
{   valid.set();
	if (!P1->valid || !P2->valid)
	{	valid.clear(); return;
	}
	X=P1->x(); Y=P1->y(); X1=P2->x(); Y1=P2->y();
	Vx=X1-X; Vy=Y1-Y;
	R=sqrt(Vx*Vx+Vy*Vy);
	if (R<1e-15) valid.clear();
	else { valid.set(); Vx/=R; Vy/=R; }
}

void Lineprim::draw (PS &ps)
{   double x,y,hx,hy,d,r;
	if (!valid || hidden()) return;
	x=(window.x1()+window.x2())/2;
	y=(window.y1()+window.y2())/2;
	d=(x-X)*Vy-(y-Y)*Vx; x-=d*Vy; y+=d*Vx;
	r=max(window.x2()-window.x1(),window.y2()-window.y1())*2;
	ps.linetrunc(window.col(x-Vx*r,ps),window.row(y-Vy*r,ps),
		window.col(x+Vx*r,ps),window.row(y+Vy*r,ps),color());
	if (showname)
	{	double extend=ps.textextend(name(),Vy,-Vx);
		r=extend*window.pixel()/2;
		x+=30*window.pixel(ps)*Vx; y+=30*window.pixel(ps)*Vy;
		x-=Vy*r; y+=Vx*r;
		ps.move(window.col(x,ps),window.row(y,ps));
		ps.text(name(),color(),TA_CENTER,TA_HALF);
	}
}

double Lineprim::distance (double x, double y)
{	return fabs((x-X)*Vy-(y-Y)*Vx);
}

void Lineprim::intersect (Lineprim &L, Pointprim &P)
{	double det=Vx*L.Vy-Vy*L.Vx,l;
	if (fabs(det)<1e-15)
	{	P.valid.clear();
		return;
	}
	l=((L.X-X)*L.Vy-(L.Y-Y)*L.Vx)/det;
	P.set(X+l*Vx,Y+l*Vy);
	if (!checkpoint(P.x(),P.y()) ||
		!L.checkpoint(P.x(),P.y())) P.valid.clear();
	else P.valid.set();
}

double Lineprim::lambda (double x, double y)
{	double vx=x-X,vy=y-Y;
	if (fabs(Vx)>fabs(Vy)) return vx/Vx;
	else return vy/Vy;
}

int Lineprim::project (double &x, double &y)
{	double l=(x-X)*Vx+(y-Y)*Vy;
	x=X+l*Vx; y=Y+l*Vy;
	return 1;
}

template<class Class>
class TwoPointAction : public Action // generate a line
{   protected :
	int State,I1,I2;
	Pointprim *P1,*P2;
	public :
	TwoPointAction (int i1, int i2) : I1(i1),I2(i2) {}
	virtual void perform (long x, long y);
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
	virtual int finished () { return State==1; }
};

TwoPointAction<Line> lineaction(8,9);

template<class Class> void TwoPointAction<Class>::start ()
{	State=1;
}

template<class Class> void TwoPointAction<Class>::inform ()
{	switch (State)
	{	case 1 : tools.inform(String(I1)); break;
		case 2 : tools.inform(String(I2)); break;
	}
}

template<class Class>
	void TwoPointAction<Class>::perform (long c, long r)
{	double x=window.x(c),y=window.y(r);
	switch (State)
	{	case 1 :
			if (!objects.select(x,y,(Object *&)P1,Object::point))
				return;
			P1->highlight.set(); objects.redraw();
			State=2; break;
		case 2 :
			if (!objects.select(x,y,(Object *&)P2,Object::point))
				return;
			if (P1->distance(P2->x(),P2->y())<1e-15) return;
			State=1;
			P1->highlight.clear();
			new Class(*P1,*P2);
			objects.redraw();
			break;
	}
	inform();
}

template<class Class> void TwoPointAction<Class>::abort ()
{   if (State==2)
	{	P1->highlight.clear(); objects.redraw();
	}
	State=0;
}

void Lineprim::edit ()
{   Dialog d(window,IDD_LineEdit,help,2010);
	StringItem nameitem(IDP_Name,d,name());
	PointColor coloritem(IDP_Color,d,1,truecolor()+1);
	CheckItem hideitem(IDP_Hidden,d,ishidden),
		shownameitem(IDP_ShowName,d,showname);
	StringItem lineitem(IDP_Line,d,stripnl(write()),512);
	lineitem.readonly();
	DoubleItem ritem(IDL_Length,d,R);
	ritem.readonly();
	d.carryout();
	if (d.result()!=DID_OK) return;
	setname(nameitem);
	if (strcmp(nameitem,name()))
		Warning(String(48),String(1),window);
	setcolor(coloritem.col()-1);
	ishidden=hideitem;
	showname=shownameitem;
}

// ******* Ray Object ******************

class Ray : public Lineprim
{   public :
	Ray (Pointprim &p1, Pointprim &p2);
	virtual void draw (PS &ps);
	virtual double distance (double x, double y);
	virtual int checkpoint (double x, double y);
};

TwoPointIO<Ray> rayio("ray");

Count raycount;

Ray::Ray (Pointprim &p1, Pointprim &p2) : Lineprim(p1,p2)
{	setname(String(26),raycount);
	setio(rayio);
}

void Ray::draw (PS &ps)
{   double x,y,hx,hy,d,r,l,r1;
	if (!valid || hidden()) return;
	x=(window.x1()+window.x2())/2;
	y=(window.y1()+window.y2())/2;
	d=(x-X)*Vy-(y-Y)*Vx; x-=d*Vy; y+=d*Vx;
	r=max(window.x2()-window.x1(),window.y2()-window.y1())*2;
	l=-lambda(x,y);
	if (l>r) return;
	if (l>-r) r1=l;
	else r1=-r;
	ps.linetrunc(window.col(x+Vx*r1,ps),window.row(y+Vy*r1,ps),
		window.col(x+Vx*r,ps),window.row(y+Vy*r,ps),color());
	if (showname)
	{   double extend=ps.textextend(name(),Vy,-Vx);
		r=extend*window.pixel(ps)/2;
		x=(X+X1)/2; y=(Y+Y1)/2;		x-=Vy*r; y+=Vx*r;
		ps.move(window.col(x,ps),window.row(y,ps));
		ps.text(name(),color(),TA_CENTER,TA_HALF);
	}
}

double Ray::distance (double x, double y)
{	double r=Vx*(x-X)+Vy*(y-Y),vx,vy;
	if (r<0)
	{	vx=x-X; vy=y-Y; return sqrt(vx*vx+vy*vy);
	}
	else return Lineprim::distance(x,y);
}

int Ray::checkpoint (double x, double y)
{	if ((x-X)*Vx+(y-Y)*Vy>=-1e-15) return 1;
	else return 0;
}

TwoPointAction<Ray> rayaction(24,25);

// ******* Segment Object ******************

class Segment : public Lineprim
{	public :
	Segment (Pointprim &p1, Pointprim &p2);
	virtual void draw (PS &ps);
	virtual double distance (double x, double y);
	virtual int checkpoint (double x, double y);
};

TwoPointIO<Segment> segmentio("segment");

Count segmentcount;

Segment::Segment (Pointprim &p1, Pointprim &p2) : Lineprim(p1,p2)
{	setname(String(29),segmentcount);
	setio(segmentio);
}

void Segment::draw (PS &ps)
{   double x,y,hx,hy,d,r,l,r1,r2;
	if (!valid || hidden()) return;
	x=(window.x1()+window.x2())/2;
	y=(window.y1()+window.y2())/2;
	d=(x-X)*Vy-(y-Y)*Vx; x-=d*Vy; y+=d*Vx;
	r=max(window.x2()-window.x1(),window.y2()-window.y1())*2;
	l=-lambda(x,y);
	if (l>r) return;
	if (l>-r) r1=l;
	else r1=-r;
	l+=length();
	if (l>r) r2=r;
	else r2=l;
	ps.linetrunc(window.col(x+Vx*r1,ps),window.row(y+Vy*r1,ps),
		window.col(x+Vx*r2,ps),window.row(y+Vy*r2,ps),color());
	if (showname)
	{   double extend=ps.textextend(name(),Vy,-Vx);
		r=extend*window.pixel(ps)/2;
		x=(X+X1)/2; y=(Y+Y1)/2;
		x-=Vy*r; y+=Vx*r;
		ps.move(window.col(x,ps),window.row(y,ps));
		ps.text(name(),color(),TA_CENTER,TA_HALF);
	}
}

double Segment::distance (double x, double y)
{	double r=Vx*(x-X)+Vy*(y-Y),vx,vy;
	if (r<0)
	{	vx=x-X; vy=y-Y; return sqrt(vx*vx+vy*vy);
	}
	else if (r>R)
	{	vx=x-X1; vy=y-Y1; return sqrt(vx*vx+vy*vy);
	}
	else return Lineprim::distance(x,y);
}

int Segment::checkpoint (double x, double y)
{	if ((x-X)*Vx+(y-Y)*Vy>=-1e-15 &&
		(x-X1)*Vx+(y-Y1)*Vy<=1e-15) return 1;
	else return 0;
}

TwoPointAction<Segment> segmentaction(27,28);

// ******** Circle object **************

class Circle : public Object // a circle through two points
{	double X,Y,X1,Y1,R;
	Pointprim *P1,*P2;
	public :
	Circle (Pointprim &p1, Pointprim &p2);
	double x() { return X; }
	double y() { return Y; }
	double x1() { return X1; }
	double y1() { return Y1; }
	double r() { return R; }
	virtual void draw (PS &ps);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P1; o[1]=P2; return 2;
	}
	virtual double distance (double x, double y);
	void intersect (Object &P, Pointprim &P1, Pointprim &P2);
	virtual int project (double &x, double &y);
	Pointprim *p1 () { return P1; }
	Pointprim *p2 () { return P2; }
	virtual void edit ();
};

TwoPointIO<Circle> circleio("circle");

Count circlecount;

Circle::Circle (Pointprim &p1, Pointprim &p2) : Object()
{	P1=&p1; P2=&p2;
	objectclass(Object::circle);
	setname(String(10),circlecount);
	setio(circleio);
	update();
}

void Circle::update ()
{   double Vx,Vy;
	valid.set();
	if (!P1->valid || !P2->valid)
	{	valid.clear(); return;
	}
	X=P1->x(); Y=P1->y(); X1=P2->x(); Y1=P2->y();
	Vx=X1-X; Vy=Y1-Y;
	R=sqrt(Vx*Vx+Vy*Vy);
	if (R<1e-15) valid.clear();
	else { valid.set(); }
}

void Circle::draw (PS &ps)
{   long c;
	double x,y,h;
	if (!valid || hidden()) return;
	c=window.col(X,ps);
	ps.circle(c,window.row(Y,ps),window.col(X+R,ps)-c,1,color());
	if (showname)
	{   h=1/sqrt(2);
		x=X+R*h+2*ps.markersize()*window.pixel(ps);
		y=Y-R*h-2*ps.markersize()*window.pixel(ps);
		ps.move(window.col(x,ps),window.row(y,ps));
		ps.text(name(),color(),TA_CENTER,TA_HALF);
	}
}

double Circle::distance (double x, double y)
{	double rx=x-X,ry=y-Y;
	return fabs(sqrt(rx*rx+ry*ry)-R);
}

void Circle::intersect (Object &P, Pointprim &P1, Pointprim &P2)
{   double d,vx,vy,hx,hy;
	if (P.objectclass()&Object::line)
	{   Lineprim &L=(Lineprim &)P;
		d=L.vy()*(X-L.x())-L.vx()*(Y-L.y());
		if (fabs(d)>R)
		{	P1.valid.clear(); P2.valid.clear();
			return;
		}
		vx=-L.vy();
		vy=L.vx();
	}
	else
	{   Circle &C=(Circle &)P;
		vx=C.x()-x(); vy=C.y()-y();
		d=sqrt(vx*vx+vy*vy);
		if (d<1e-15 || d>C.r()+r() || d+C.r()<r() || d+r()<C.r())
		{	P1.valid.clear(); P2.valid.clear();
			return;
		}
		vx/=d; vy/=d;
		d=(r()*r()-C.r()*C.r()+d*d)/(2*d);
	}
	hx=X+d*vx; hy=Y+d*vy;
	d=sqrt(r()*r()-d*d);
	P1.set(hx+vy*d,hy-vx*d);
	P2.set(hx-vy*d,hy+vx*d);
	if (P.objectclass()&Object::line)
	{   Lineprim &L=(Lineprim &)P;
		if (L.checkpoint(P1.x(),P1.y())) P1.valid.set();
		else P1.valid.clear();
		if (L.checkpoint(P2.x(),P2.y())) P2.valid.set();
		else P2.valid.clear();
	}
	else
	{	P1.valid.set();
		P2.valid.set();
	}
}

int Circle::project (double &x, double &y)
{	double vx=x-P1->x(),vy=y-P1->y(),r;
	r=sqrt(vx*vx+vy*vy);
	if (r<1e-15) return 0;
	x=P1->x()+vx/r*R; y=P1->y()+vy/r*R;
	return 1;
}

TwoPointAction<Circle> circleaction(11,12);

void Circle::edit ()
{   Dialog d(window,IDD_CircleEdit,help,2010);
	StringItem nameitem(IDP_Name,d,name());
	PointColor coloritem(IDP_Color,d,1,truecolor()+1);
	CheckItem hideitem(IDP_Hidden,d,ishidden),
		shownameitem(IDP_ShowName,d,showname);
	StringItem lineitem(IDP_Line,d,stripnl(write()),512);
	lineitem.readonly();
	DoubleItem ritem(IDL_Length,d,R);
	ritem.readonly();
	d.carryout();
	if (d.result()!=DID_OK) return;
	setname(nameitem);
	if (strcmp(nameitem,name()))
		Warning(String(48),String(1),window);
	setcolor(coloritem.col()-1);
	ishidden=hideitem;
	showname=shownameitem;
}

// ******** Middle Point ***************

class Middle : public Pointprim
{	Pointprim *P1,*P2;
	public :
	Middle (Pointprim &p1, Pointprim &p2);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P1; o[1]=P2; return 2;
	}
	Pointprim *p1 () { return P1; }
	Pointprim *p2 () { return P2; }
};

TwoPointIO<Middle> middleio("middle");

Count middlecount;

Middle::Middle (Pointprim &p1, Pointprim &p2)
{	P1=&p1; P2=&p2;
	setio(middleio);
	setname(String(49),middlecount);
	objectclass(Object::point);
	update();
}

void Middle::update ()
{   valid.set();
	if (!P1->valid || !P2->valid)
	{	valid.clear();
		return;
	}
	set((P1->x()+P2->x())/2,(P1->y()+P2->y())/2);
}

TwoPointAction<Middle> middleaction(45,46);

// ******** Edit Objects ********

class EditAction : public Action
{   public :
	virtual void perform (long c, long r);
	virtual void inform ();
} editaction;

void EditAction::perform (long c, long r)
{	Object *P;
	double x=window.x(c),y=window.y(r);
	if (!objects.select(x,y,P)) return;
	P->highlight.set(); objects.redraw();
	P->edit();
	objects.update(P);
	P->highlight.clear();
	objects.redraw();
}

void EditAction::inform ()
{   tools.inform(String(44));
}

// ******** Move action ****************

class MoveAction : public Action
{   Pointprim *P;
	public :
	MoveAction () { P=0; drag.set(); }
	virtual void dragperform (int type, long x, long y);
	virtual void inform ();
	virtual void start ();
	virtual int finished () { return P==0; }
	virtual void abort ()
	{	if (P) { P->highlight.clear(); objects.redraw(); }
		P=0;
	}
} moveaction;

void MoveAction::start ()
{	P=0;
}

void MoveAction::dragperform (int type, long c, long r)
{   double x=window.x(c),y=window.y(r);
	static int moved=0,pickup=0;
	int count;
	if (!P) pickup=0;
	switch(type)
	{	case ClickType::button1down :
		case ClickType::button2down :
			moved=0;
			if (pickup && P) P->highlight.clear();
			count=objects.select(x,y,(Object * &)P,
					Object::notfixed,pickup?0:1);
			if (count<1) break;
			P->highlight.set(); objects.redraw();
			tools.inform(String(13));
			if (count==1)
			{	window.capture(1);
				pickup=0;
			}
			else
			{	pickup=1;
			}
			break;
		case ClickType::mousemove :
			if (!P || pickup) break;
			P->set(x,y);
			objects.update(P);
			objects.redraw();
			moved=1;
			break;
		case ClickType::button1up :
		case ClickType::button2up :
			if (pickup) break;
			if (P)
			{	window.capture(0);
				P->highlight.clear();
				objects.redraw();
			}
			if (!moved && type==ClickType::button2up)
			{	editaction.perform(c,r);
			}
			inform();
			P=0;
			break;
	}
}

void MoveAction::inform ()
{	tools.inform(String(14));
}

// ******** Move on Object action ****************

class MoveonAction : public Action
{   Pointprim *P;
	Object *O;
	public :
	MoveonAction () { drag.clear(); }
	virtual void dragperform (int type, long c, long r);
	virtual void perform (long c, long r);
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
} moveonaction;

void MoveonAction::start ()
{   P=0;
	drag.clear();
}

void MoveonAction::dragperform (int type, long c, long r)
{   double x=window.x(c),y=window.y(r);
	switch(type)
	{	case ClickType::button1down :
			if (!objects.select(x,y,(Object * &)P,
					Object::notfixed,0))
				break;
			objects.mark(P);
			if (O->mark)
			{	P=0; break;
			}
			P->highlight.set(); objects.redraw();
			tools.inform(String(13));
			break;
		case ClickType::mousemove :
			if (!P) break;
			if (O->project(x,y))
			{	P->set(x,y);
				P->update();
			}
			objects.update(P);
			objects.redraw();
			break;
		case ClickType::button1up :
			if (!P) break;
			P->highlight.clear(); P=0;
			O->highlight.clear();
			drag.clear();
			objects.redraw();
			inform();
			break;
	}
}

void MoveonAction::perform (long c, long r)
{   double x=window.x(c),y=window.y(r);
	if (!objects.select(x,y,O,Object::line|Object::circle))
		return;
	O->highlight.set();
	objects.redraw();
	drag.set();
	inform();
}

void MoveonAction::abort ()
{   if (drag) O->highlight.clear();
	drag.clear();
	objects.redraw();
}

void MoveonAction::inform ()
{	if (!drag) tools.inform(String(37));
	else tools.inform(String(14));
}

// ******* Intersections ***************

class OtherIntersection; // Intersections may generate two points
class Intersection : public Pointprim
// Intersection of line/circle with line/circle
{	Object *P1,*P2;
	OtherIntersection *Other; // NULL or other point
	public :
	Intersection (Object &p1, Object &p2);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P1; o[1]=P2; return 2;
	}
	Object *first () { return P1; }
	Object *second () { return P2; }
	OtherIntersection *other() { return Other; }
};

class OtherIntersection : public Pointprim
// The other intersection point
{	Intersection *Other; // knows its brother
	public :
	OtherIntersection (Intersection *other);
	virtual int dep (Object *o[])
	{	o[0]=Other; return 1;
	}
	Intersection *other () { return Other; }
	virtual Object *original () { return Other; }
};

class NoIO : public IO
	// class does not appear in files
{	public :
	NoIO () : IO("otherintersection") {}
	virtual char *write (Object &o) { return ""; }
} noio;

Count iscount;

OtherIntersection::OtherIntersection (Intersection *other)
{	Other=other;
	objectclass(Object::point);
	setname(String(15),iscount);
	setio(noio);
}

class IntersectionIO : public IO
{	public :
	IntersectionIO () : IO("intersection") {}
	virtual void depwrite (Object &o)
	{	Intersection &l=(Intersection &)o;
		depadd(*l.first()); comma(); depadd(*l.second());
	}
	virtual void reswrite (Object &o)
	{   Intersection &l=(Intersection &)o;
		if (l.other()) { comma(); resadd(*l.other()); }
	}
	virtual int interpret (Input &i);
} intersectionio;

int IntersectionIO::interpret (Input &i)
{   Intersection *l;
	if (i.nright()!=2) return 0;
	if (i.nleft()==1)
	{	Object *o1=i.object(0,Object::line),
				*o2=i.object(1,Object::line);
		if (!o1 || !o2) return 0;
		l=new Intersection(*o1,*o2);
		return i.set(*l,0);
	}
	else if (i.nleft()==2)
	{	Object *o1=i.object(0,Object::line|Object::circle),
				*o2=i.object(1,Object::line|Object::circle);
		if (!o1 || !o2) return 0;
		l=new Intersection(*o1,*o2);
		if (!l->other()) return 0;
		if (!i.set(*l,0)) return 0;
		return i.set(*l->other(),1);
	}
	return 0;
}

Intersection::Intersection (Object &p1, Object &p2) : Pointprim()
{	P1=&p1; P2=&p2;
	if ((P1->objectclass()&Object::circle) ||
		(P2->objectclass()&Object::circle))
			Other=new OtherIntersection(this);
	else Other=0;
	objectclass(Object::point);
	setname(String(15),iscount);
	setio(intersectionio);
	update();
}

void Intersection::update ()
{   valid.set();
	if (!P1->valid || !P2->valid)
	{	valid.clear();
		if (Other) Other->valid.clear();
		return;
	}
	if (!Other) // Means the intersection is between lines
		((Lineprim *)P1)->intersect(*(Lineprim *)P2,
			*(Pointprim *)this);
	else if (P1->objectclass()&Object::circle) // first is a circle
		((Circle *)P1)->intersect(*P2,
			*(Pointprim *)this,*(Pointprim *)Other);
	else // second must be a circle
		((Circle *)P2)->intersect(*P1,
			*(Pointprim *)this,*(Pointprim *)Other);
}

class IntersectionAction : public Action // generate a line
{   int State;
	Object *P1,*P2;
	public :
	virtual void perform (long x, long y);
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
	virtual int finished () { return State==1; }
} intersectionaction;

void IntersectionAction::start ()
{	State=1;
}

void IntersectionAction::inform ()
{	switch (State)
	{	case 1 : tools.inform(String(16)); break;
		case 2 : tools.inform(String(17)); break;
	}
}

void IntersectionAction::perform (long c, long r)
{	double x=window.x(c),y=window.y(r);
	switch (State)
	{	case 1 :
			if (!objects.select(x,y,P1,
				Object::line|Object::circle)) return;
			P1->highlight.set(); objects.redraw();
			State=2; break;
		case 2 :
			if (!objects.select(x,y,P2,
				Object::line|Object::circle)) return;
			if (P1==P2) return;
			State=1;
			P1->highlight.clear();
			new Intersection(*P1,*P2);
			objects.redraw();
			break;
	}
	inform();
}

void IntersectionAction::abort ()
{   if (State==2)
	{	P1->highlight.clear(); objects.redraw();
	}
	State=0;
}

// ******** Hide/Unhide Objects ********

class HideAction : public Action
{   public :
	virtual void perform (long c, long r);
	virtual void inform ();
} hideaction;

void HideAction::perform (long c, long r)
{	Object *P;
	double x=window.x(c),y=window.y(r);
	if (!objects.select(x,y,P)) return;
	P->ishidden.toggle();
	objects.redraw();
}

void HideAction::inform ()
{   tools.inform(String(18));
}

// ******* Delete Action ***************

class DeleteAction : public Action
{	public :
	virtual void perform (long c, long r);
	virtual void inform ();
} deleteaction;

void DeleteAction::perform (long c, long r)
{	Object *P;
	double x=window.x(c),y=window.y(r);
	if (!objects.select(x,y,P)) return;
	objects.clear(P->original());
	objects.redraw();
}

void DeleteAction::inform ()
{	tools.inform(String(23));
}

// ******** Parallels **************

class DepLine : public Line
	// A line with no IO
{   Object *O;
	public :
	DepLine(Pointprim &p1, Pointprim &p2) :
		Line(p1,p2),O(&p2)
	{	setio(noio);
	}
	virtual Object *original () { return O; }
};

class Parallel : public Pointprim
	// a line parallel to another line
	// passing through a given point
	// creates a new point and a line
{   protected :
	Pointprim *P;
	Lineprim *L;
	DepLine *Other;
	public :
	Parallel (Pointprim &p, Lineprim &l);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P; o[1]=L; return 2;
	}
	Pointprim *point () { return P; }
	Lineprim *line () { return L; }
	DepLine *other () { return Other; }
	void other (DepLine *o) { Other=o; }
};

template<class Class>
class PointLineIO : public IO
{	public :
	PointLineIO (char *name) : IO(name) {}
	virtual void depwrite (Object &o)
	{	Class &l=(Class &)o;
		depadd(*l.point()); comma(); depadd(*l.line());
	}
	virtual void reswrite (Object &o)
	{   Class &l=(Class &)o;
		comma(); resadd(*l.other());
	}
	virtual int interpret (Input &i);
};

template<class Class> int PointLineIO<Class>::interpret (Input &i)
{	if (i.nleft()!=2 || i.nright()!=2) return 0;
	Pointprim *p=(Pointprim *)i.object(0,Object::point);
	Lineprim *l=(Lineprim *)i.object(1,Object::line);
	if (!p || !l) return 0;
	Class *pl=new Class(*p,*l);
	pl->other(new DepLine(*p,*pl));
	if (!i.set(*pl,0)) return 0;
	return i.set(*pl->other(),1);
}

PointLineIO<Parallel> parallelio("parallel");

Parallel::Parallel (Pointprim &p, Lineprim &l) : Pointprim()
{	P=&p; L=&l;
	objectclass(Object::point);
	setio(parallelio);
	update();
}

void Parallel::update ()
{   valid.set();
	if (!P->valid || !L->valid)
	{	valid.clear(); return;
	}
	set(P->x()+L->vx(),P->y()+L->vy());
}

template<class Class>
class PointLineAction : public Action // generate a parallel
{   protected :
	int State,I1,I2;
	Pointprim *P;
	Lineprim *L;
	public :
	PointLineAction (int i1, int i2) : I1(i1),I2(i2) {}
	virtual void perform (long x, long y);
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
	virtual int finished () { return State==1; }
};

PointLineAction<Parallel> parallelaction(30,31);

template<class Class> void PointLineAction<Class>::start ()
{	State=1;
}

template<class Class> void PointLineAction<Class>::inform ()
{	switch (State)
	{	case 1 : tools.inform(String(I1)); break;
		case 2 : tools.inform(String(I2)); break;
	}
}

template<class Class>
	void PointLineAction<Class>::perform (long c, long r)
{   Class *R;
	double x=window.x(c),y=window.y(r);
	switch (State)
	{	case 1 :
			if (!objects.select(x,y,(Object *&)L,Object::line))
				return;
			L->highlight.set(); objects.redraw();
			State=2; break;
		case 2 :
			if (!objects.select(x,y,(Object *&)P,Object::point))
				return;
			State=1;
			L->highlight.clear();
			R=new Class(*P,*L);
			R->ishidden.set();
			R->other(new DepLine(*P,*R));
			objects.redraw();
			break;
	}
	inform();
}

template<class Class> void PointLineAction<Class>::abort ()
{   if (State==2)
	{	L->highlight.clear(); objects.redraw();
	}
	State=0;
}

// ******** Rectangular Action **************

class Rectangular : public Pointprim
	// a line parallel to another line
	// passing through a given point
	// creates a new point and a line
{   protected :
	Pointprim *P;
	Lineprim *L;
	DepLine *Other;
	public :
	Rectangular (Pointprim &p, Lineprim &l);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P; o[1]=L; return 2;
	}
	Pointprim *point () { return P; }
	Lineprim *line () { return L; }
	DepLine *other () { return Other; }
	void other (DepLine *o) { Other=o; }
};

PointLineIO<Rectangular> rectangulario("rectangular");

Rectangular::Rectangular (Pointprim &p, Lineprim &l) :
		Pointprim()
{	P=&p; L=&l;
	objectclass(Object::point);
	setio(rectangulario);
	update();
}

void Rectangular::update ()
{   valid.set();
	if (!P->valid || !L->valid)
	{	valid.clear(); return;
	}
	set(P->x()-L->vy(),P->y()+L->vx());
}

PointLineAction<Rectangular> rectangularaction(32,33);

// ******** Point on Object **************

class Pointon : public Pointprim
	// A Point on a Line or Circle
{   protected :
	Object *O;
	public :
	Pointon (Object &O);
	Pointon (Object &O, double x, double y);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=O; return 1;
	}
	Object *object() { return O; }
};

class PointonIO : public IO
{	public :
	PointonIO () : IO("pointon") {}
	virtual void depwrite (Object &o)
	{	Pointon &p=(Pointon &)o;
		depadd(*p.object());
		sprintf(end(),",%g,%g",p.x(),p.y());
	}
	virtual int interpret (Input &i);
} pointonio;

int PointonIO::interpret (Input &i)
{	if (i.nleft()!=1 || i.nright()!=3) return 0;
	Object *o=i.object(0,Object::line|Object::circle);
	if (!o) return 0;
	double x,y;
	if (!i.right()[1].todouble(x) || !i.right()[2].todouble(y))
		return 0;
	Pointon *P=new Pointon(*o,x,y);
	return i.set(*P,0);
}

Count pointoncount;

Pointon::Pointon (Object &o) : Pointprim()
{	O=&o;
	objectclass(Object::point|Object::moveable|Object::notfixed);
	setname(String(50),pointoncount);
	setio(pointonio);
	update();
}

Pointon::Pointon (Object &o, double x, double y)
{	O=&o;
	objectclass(Object::point|Object::moveable|Object::notfixed);
	setname(String(50),pointoncount);
	setio(pointonio);
	set(x,y);
	update();
}

void Pointon::update ()
{   double a=x(),b=y();
	valid.set();
	if (!O->valid)
	{	valid.clear(); return;
	}
	if (O->project(a,b)) set(a,b);
}

class PointonAction : public Action // generate a parallel
{   Object *O;
	protected :
	public :
	virtual void perform (long x, long y);
	virtual void inform ();
} pointonaction;

void PointonAction::inform ()
{	tools.inform(String(38));
}

void PointonAction::perform (long c, long r)
{   double x=window.x(c),y=window.y(r);
	if (!objects.select(x,y,O,Object::line|Object::circle))
		return;
	new Pointon(*O,x,y);
	objects.redraw();
}

// ******** Circles of three points **************

class Circle3Circle : public Circle
{   Object *O;
	public :
	Circle3Circle (Pointprim &p1, Pointprim &p2) :
		Circle(p1,p2),O(&p2)
	{	setio(noio);
	}
	virtual Object *original () { return O; }
};

class Circle3 : public Pointprim
	// Create a circle from midpoint and radius
{   protected :
	Pointprim *P1,*P2,*M;
	Circle3Circle *C;
	public :
	Circle3 (Pointprim &m, Pointprim &p1, Pointprim &p2);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P1; o[1]=P2; o[2]=M; return 3;
	}
	Pointprim *p1 () { return P1; }
	Pointprim *p2 () { return P2; }
	Pointprim *m () { return M; }
	Circle3Circle *c () { return C; }
	void c (Circle3Circle *cc) { C=cc; }
};

class Circle3IO : public IO
{	public :
	Circle3IO () : IO("circle3") {}
	virtual void depwrite (Object &o)
	{	Circle3 &l=(Circle3 &)o;
		depadd(*l.m()); comma();
		depadd(*l.p1()); comma(); depadd(*l.p2());
	}
	virtual void reswrite (Object &o)
	{   Circle3 &l=(Circle3 &)o;
		comma(); resadd(*l.c());
	}
	virtual int interpret (Input &i);
} circle3io;

int Circle3IO::interpret (Input &i)
{	if (i.nleft()!=2 || i.nright()!=3) return 0;
	Pointprim *p1=(Pointprim *)i.object(0,Object::point),
		*p2=(Pointprim *)i.object(1,Object::point),
		*p3=(Pointprim *)i.object(2,Object::point);
	if (!p1 || !p2 || !p3) return 0;
	Circle3 *l=new Circle3(*p1,*p2,*p3);
	l->c(new Circle3Circle(*p1,*l));
	if (!i.set(*l,0)) return 0;
	return i.set(*l->c(),1);
}

Circle3::Circle3 (Pointprim &m, Pointprim &p1, Pointprim &p2) : Pointprim()
{	P1=&p1; P2=&p2; M=&m;
	objectclass(Object::point);
	setio(circle3io);
	update();
}

void Circle3::update ()
{   valid.set();
	if (!P1->valid || !P2->valid || !M->valid)
	{	valid.clear(); return;
	}
	set(M->x()+P2->x()-P1->x(),M->y()+P2->y()-P1->y());
}

class Circle3Action : public Action // generate a parallel
{   protected :
	int State;
	Pointprim *P1,*P2,*M;
	public :
	virtual void perform (long x, long y);
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
	virtual int finished () { return State==1; }
} circle3action;

void Circle3Action::start ()
{	State=1;
}

void Circle3Action::inform ()
{	switch (State)
	{	case 1 : tools.inform(String(34)); break;
		case 2 : tools.inform(String(35)); break;
		case 3 : tools.inform(String(36)); break;
	}
}

void Circle3Action::perform (long c, long r)
{   Circle3 *R;
	double x=window.x(c),y=window.y(r);
	switch (State)
	{	case 1 :
			if (!objects.select(x,y,(Object *&)M,Object::point))
				return;
			M->highlight.set(); objects.redraw();
			State=2; break;
		case 2 :
			if (!objects.select(x,y,(Object *&)P1,Object::point))
				return;
			P1->highlight.set(); objects.redraw();
			State=3; break;
		case 3 :
			if (!objects.select(x,y,(Object *&)P2,Object::point))
				return;
			State=1;
			M->highlight.clear();
			P1->highlight.clear();
			R=new Circle3(*M,*P1,*P2);
			R->ishidden.set();
			R->c(new Circle3Circle(*M,*R));
			objects.redraw();
			break;
	}
	inform();
}

void Circle3Action::abort ()
{   if (State==2)
	{	M->highlight.clear();
		objects.redraw();
	}
	else if (State==3)
	{	M->highlight.clear();
		P1->highlight.clear();
		objects.redraw();
	}
	State=0;
}

// ******** Angles of three points **************

class Angle : public Object
	// Create a circle from midpoint and radius
{   protected :
	Pointprim *P1,*P2,*M;
	double Phi1,Phi2;
	public :
	Angle (Pointprim &m, Pointprim &p1, Pointprim &p2);
	virtual void update ();
	virtual int dep (Object *o[])
	{	o[0]=P1; o[1]=P2; o[2]=M; return 3;
	}
	Pointprim *p1 () { return P1; }
	Pointprim *p2 () { return P2; }
	Pointprim *m () { return M; }
	virtual void draw (PS &ps);
	virtual double distance (double x, double y)
	{	return M->distance(x,y);
	}
	virtual void edit ();
};

class AngleIO : public IO
{	public :
	AngleIO () : IO("angle") {}
	virtual void depwrite (Object &o)
	{	Angle &l=(Angle &)o;
		depadd(*l.p1()); comma();
		depadd(*l.m()); comma(); depadd(*l.p2());
	}
	virtual int interpret (Input &i);
} angleio;

int AngleIO::interpret (Input &i)
{	if (i.nleft()!=1 || i.nright()!=3) return 0;
	Pointprim *p1=(Pointprim *)i.object(0,Object::point),
		*p2=(Pointprim *)i.object(1,Object::point),
		*p3=(Pointprim *)i.object(2,Object::point);
	if (!p1 || !p2 || !p3) return 0;
	Angle *l=new Angle(*p1,*p2,*p3);
	return i.set(*l,0);
}

Count anglecount;

Angle::Angle (Pointprim &p1, Pointprim &m, Pointprim &p2)
{	P1=&p1; P2=&p2; M=&m;
	objectclass(Object::angle);
	setio(angleio);
	setname(String(56),anglecount);
	update();
}

int arg (double vx, double vy, double &phi)
{	if (fabs(vx)<1e-15 && fabs(vy)<1e-15) return 0;
	phi=atan2(vy,vx);
	return 1;
}

const double pideg=180.0/(4*atan(1)),pi2=8*atan(1);

void Angle::update ()
{	if (!P1->valid || !P2->valid || !M->valid)
	{	valid.clear(); return;
	}
	valid.set();
	if (!arg(P1->x()-M->x(),P1->y()-M->y(),Phi1)) valid.clear();
	if (!arg(P2->x()-M->x(),P2->y()-M->y(),Phi2)) valid.clear();
}

void Angle::draw (PS &ps)
{   long c;
	double x,y,h,phi1,phi2;
	if (!valid || hidden()) return;
	ps.arc(window.col(M->x(),ps),window.row(M->y(),ps),20,
		1,Phi1*pideg,Phi2*pideg,color());
}

class AngleAction : public Action // generate a parallel
{   protected :
	int State;
	Pointprim *P1,*P2,*M;
	public :
	virtual void perform (long x, long y);
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
	virtual int finished () { return State==1; }
} angleaction;

void AngleAction::start ()
{	State=1;
}

void AngleAction::inform ()
{	switch (State)
	{	case 1 : tools.inform(String(53)); break;
		case 2 : tools.inform(String(54)); break;
		case 3 : tools.inform(String(55)); break;
	}
}

void AngleAction::perform (long c, long r)
{   double x=window.x(c),y=window.y(r);
	switch (State)
	{	case 1 :
			if (!objects.select(x,y,(Object *&)P1,Object::point))
				return;
			P1->highlight.set(); objects.redraw();
			State=2; break;
		case 2 :
			if (!objects.select(x,y,(Object *&)M,Object::point))
				return;
			M->highlight.set(); objects.redraw();
			State=3; break;
		case 3 :
			if (!objects.select(x,y,(Object *&)P2,Object::point))
				return;
			State=1;
			P1->highlight.clear();
			M->highlight.clear();
			new Angle(*P1,*M,*P2);
			objects.redraw();
			break;
	}
	inform();
}

void AngleAction::abort ()
{   if (State==2)
	{	P1->highlight.clear();
		objects.redraw();
	}
	else if (State==3)
	{	M->highlight.clear();
		P1->highlight.clear();
		objects.redraw();
	}
	State=0;
}

void Angle::edit ()
{   Dialog d(window,IDD_AngleEdit,help,2010);
	StringItem nameitem(IDP_Name,d,name());
	PointColor coloritem(IDP_Color,d,1,truecolor()+1);
	CheckItem hideitem(IDP_Hidden,d,ishidden),
		shownameitem(IDP_ShowName,d,showname);
	StringItem lineitem(IDP_Line,d,stripnl(write()),512);
	lineitem.readonly();
	DoubleItem ritem(IDL_Length,d,
		fabs(fmod((Phi2-Phi1)*pideg+360,360)));
	ritem.readonly();
	d.carryout();
	if (d.result()!=DID_OK) return;
	setname(nameitem);
	if (strcmp(nameitem,name()))
		Warning(String(48),String(1),window);
	setcolor(coloritem.col()-1);
	ishidden=hideitem;
	showname=shownameitem;
}

// ********* Track action **************

class TrackAction : public Action
{   Pointprim *P;
	public :
	virtual void inform ();
	virtual void perform (long c, long r);
} trackaction;

void TrackAction::inform ()
{	tools.inform(String(39));
}

void TrackAction::perform (long c, long r)
{	double x=window.x(c),y=window.y(r);
	if (!objects.select(x,y,(Object * &)P,Object::point))
		return;
	P->starttrack();
	objects.redraw();
}

// *********** Makro things ********

class ParameterList
// List for holding parameter and target objects
{	Object *P;
	ParameterList *Next;
	String Prompt,Name; // Parameter prompt and name in makro
	int Class; // point, line or circle
	public :
	ParameterList (Object &p, char *s="", char *sn="") :
		P(&p),Next(0),Prompt(s),Name(sn) {}
	~ParameterList ()
	{	if (Next) delete Next;
	}
	ParameterList (char *n, char *s, int c) :
		P(0),Next(0),Prompt(s),Name(n),Class(c) {}
	ParameterList * next () { return Next; }
	void next (ParameterList &p) { Next=&p; }
	void append (ParameterList *p);
	Object *object () { return P; }
	void object (Object *o) { P=o; }
	char * prompt () { return Prompt; }
	void prompt (char *s) { Prompt.copy(s); }
	char * name () { return Name; }
	void name (char *s) { Name.copy(s); }
	int objectclass () { return Class; }
	void objectclass (int t) { Class=t; }
};

void ParameterList::append (ParameterList *p)
{	if (!Next) Next=p;
	else Next->append(p);
}

class Makro;
class MakroDefineAction : public Action
// Action to define a makro by choosing parameters first,
// then targets
{   ParameterList *P,*T;
	int State;
	public :
	MakroDefineAction () : P(0),T(0),State(0) {}
	virtual void inform ();
	virtual void start ();
	virtual void abort ();
	virtual void perform (long c, long r);
	void unhighlight ();
	void forget ();
	void tforget ();
	int state () { return State; }
	ParameterList * parameter () { return P; }
	void advance ();
	int invalid () { return P==0 || T==0; }
	int rektest (Object *o);
	int testmakro ();
		// test if everything is constructable
		// and mark construction steps
	int testmakro (Object *o);
		// test if o is constructable
	int isparam (Object *o); // check if o is one of the parameters
	void createmakro (char *name);
	void createmakro (Makro *m);
} makrodefineaction;

void MakroDefineAction::forget ()
{	if (P) delete P;
	if (T) delete T;
	P=T=0;
}

void MakroDefineAction::tforget ()
{	if (T) delete T;
	T=0;
}

void MakroDefineAction::unhighlight ()
{	ParameterList *p;
	p=P;
	while (p)
	{	p->object()->highlight.clear();
		p=p->next();
	}
	p=T;
	while (p)
	{	p->object()->highlight.clear();
		p=p->next();
	}
	objects.redraw();
}

void MakroDefineAction::start ()
{	forget(); State=1;
}

void MakroDefineAction::abort ()
{	unhighlight();
	forget();
	State=0;
}

void MakroDefineAction::inform ()
{	if (State==2) tools.inform(String(61));
	else tools.inform(String(60));
}

void MakroDefineAction::advance ()
{	unhighlight(); State=2; inform();
}

void MakroDefineAction::perform (long c, long r)
{	double x=window.x(c),y=window.y(r);
	Object *o;
	if (!objects.select
		(x,y,o,
		Object::point|Object::line|Object::circle,1,0))
		return;
	if (State==1)
	{   int np;
		Dialog d(window,IDD_Parameter,help,2070);
		if (o->objectclass()&Object::circle) np=65;
		else if (o->objectclass()&Object::line) np=64;
		else np=63;
		StringItem prompt(ID_Parameter,d,String(np));
		d.carryout();
		if (d.result()!=Dialog::ok) return;
		String n(prompt);
		if (!*(char *)n) n.copy(String(69));
		ParameterList *pl=new ParameterList(*o,n);
		if (P) P->append(pl);
		else P=pl;
	}
	else if (State==2)
	{   if (isparam(o))
		{	Warning(String(68),String(1),window);
			return;
		}
		if (!makrodefineaction.testmakro(o)) return;
		ParameterList *tl=new ParameterList(*o,"");
		if (T) T->append(tl);
		else T=tl;
	}
	o->highlight.set(); objects.redraw();
}

int MakroDefineAction::isparam (Object *o)
{	ParameterList *ol=P;
	while (ol)
	{	if (ol->object()==o) return 1;
		ol=ol->next();
	}
	return 0;
}

void domakroparameter ()
// menu: Choose Parameters
{	if (makrodefineaction.state()==2) makrodefineaction.abort();
	window.action(makrodefineaction);
}

void domakroconstructed ()
// menu: Choose Targets
{	if (makrodefineaction.state()==2)
	{   makrodefineaction.unhighlight();
		makrodefineaction.tforget();
	}
	else if (makrodefineaction.state()==0 ||
		makrodefineaction.parameter()==0)
	{	Warning(String(62),String(1),window);
		makrodefineaction.start();
	}
	else makrodefineaction.advance();
}

int MakroDefineAction::rektest (Object *o)
// test if o is constructable from the parameters
{	Object *ol[MAXDEP];
	int i,n;
	if (o->mark) return 1;
	o->mark.set();
	n=o->dep(ol);
	if (n==0) return 0;
	for (i=0; i<n; i++)
		if (!rektest(ol[i])) return 0;
	return 1;
}

int MakroDefineAction::testmakro ()
{   ParameterList *t;
	char s[256];
	objects.unmark();
	t=P;
	while (t) // mark parameters
	{	t->object()->mark.set();
		t=t->next();
	}
	t=T;
	while (t) // check targets and construction steps
	{   if (!rektest(t->object()))
		{	sprintf(s,String(67),t->object()->name());
			Warning(s,String(1),window);
			return 0;
		}
		t=t->next();
	}
	return 1;
}

int MakroDefineAction::testmakro (Object *o)
{   ParameterList *t;
	char s[256];
	objects.unmark();
	t=P;
	while (t) // mark parameters
	{	t->object()->mark.set();
		t=t->next();
	}
	if (!rektest(o))
	{	sprintf(s,String(67),o->name());
		Warning(s,String(1),window);
		return 0;
	}
	return 1;
}

void domakrodefine ()
// menu: Define Macro
{	if (makrodefineaction.state()!=2 ||
			makrodefineaction.invalid())
	{	Warning(String(66),String(1),window);
		return;
	}
	Dialog d(window,IDD_MakroName,help,2080);
	StringItem name(ID_MakroName,d,"");
	allover :
	d.carryout();
	if (d.result()!=Dialog::ok) return;
	if (!makrodefineaction.testmakro()) return;
	if (!*(char *)name)
	{	Warning(String(70),String(1),window);
		goto allover;
	}
	makrodefineaction.createmakro(name);
}

class ConstructionStep
// list to hold the construction steps
{	ConstructionStep *Next;
	String S;
	public :
	ConstructionStep (char *s) : S(s),Next(0) {}
	~ConstructionStep ()
	{	if (Next) delete Next;
	}
	ConstructionStep *next () { return Next; }
	void next (ConstructionStep *s) { Next=s; }
	char *step () { return S; }
	void append (ConstructionStep *s)
	{	if (!Next) Next=s;
		else Next->append(s);
	}
};

Dialog makrosdialog(window,IDD_Makros,help,2060);

Count makrocount(1000); // menu id count

class Makro
// Class to hold a Makro
// Includes name, paramters and construction steps
{	Makro *Next,*Prev;
	ParameterList *P; // List of parameters
	String Name; // The makro name
	ConstructionStep *Step; // List of construction steps
	int C; // Number for menu ID
	public :
	Makro (ParameterList *p, ParameterList *t,
		char *name);
	Makro (char *name) :
		P(0),Next(0),Prev(0),Step(0),Name(name),C(makrocount.count())
	{}
	~Makro ()
	{	if (P) delete P;
		if (Step) delete Step;
	}
	Makro *next () { return Next; }
	void next (Makro *m) { Next=m; }
	Makro *prev () { return Prev; }
	void prev (Makro *m) { Prev=m; }
	char *name () { return Name; }
	ParameterList *params () { return P; }
	ConstructionStep *step () { return Step; }
	Makro * find (char *name)
	{	if (!strcmp(name,Name)) return this;
		if (!Next) return 0;
		else return Next->find(name);
	}
	void remove ();
	int count () { return C; }
	void save (char *filename);
	void addparam (char *name, char *prompt, int type)
	{	ParameterList *pl=new ParameterList(name,prompt,type);
		if (!P) P=pl;
		else P->append(pl);
	}
	void addstep (char *s)
	{	ConstructionStep *cs=new ConstructionStep(s);
		if (!Step) Step=cs;
		else Step->append(cs);
	}
};

Makro *makros=0;

class MakrosItem : public ListItem
// list box handling class for makro choice
{	public :
	MakrosItem() : ListItem(ID_Makros,makrosdialog) {}
	virtual void init ();
	virtual void notify ();
} makrositem;

void MakrosItem::init ()
{   int count=0;
	Makro *m=makros;
	while (m)
	{	insert(m->name());
		m=m->next();
		count++;
	}
	select(count-1);
}

void MakrosItem::notify ()
{	finish();
}

void Makro::remove ()
{	if (Prev) Prev->next(Next);
	else makros=Next;
	if (Next) Next->prev(Prev);
	Next=0; Prev=0;
	menu.removeitem(C,600);
}


Makro::Makro (ParameterList *p, ParameterList *t,
	char *name) :
		Name(name),P(p),Step(0),Next(0),Prev(0),
		C(makrocount.count())
{   ParameterList *ol=p;
	while (ol) // get types of parameters and their names
		// objects are then irrelevant.
	{	ol->objectclass(ol->object()->objectclass());
		ol->name(ol->object()->name());
		ol=ol->next();
	}
	Object *o=objects.first();
	while (o)
	{   if (o->mark && !makrodefineaction.isparam(o))
		{   ConstructionStep *s=new ConstructionStep(o->write());
			if (*(char *)s->step())
			{	if (!Step) Step=s;
				else Step->append(s);
			}
		}
		o=o->next();
	}
}

void domakromenu ();

void MakroDefineAction::createmakro (char *name)
{   Makro *m;
	char s[256];
	if (makros && (m=makros->find(name))!=0)
	{   sprintf(s,String(72),m->name(),window);
		if (Question(s,String(1),window)!=Answers::yes) return;
		m->remove(); delete m;
	}
	m=new Makro(P,T,name);
	P=0; window.abort();
	if (makros)
	{	m->next(makros);
		makros->prev(m);
	}
	makros=m;
	menu.additem(m->count(),domakromenu,m->name(),600,0);
}

void MakroDefineAction::createmakro (Makro *m)
{   char s[256];
	Makro *m1;
	if (makros && (m1=makros->find(m->name()))!=0)
	{   sprintf(s,String(72),m->name(),window);
		if (Question(s,String(1),window)!=Answers::yes) return;
		m1->remove(); delete m1;
	}
	if (makros)
	{	m->next(makros);
		makros->prev(m);
	}
	makros=m;
	menu.additem(m->count(),domakromenu,m->name(),600,0);
}

char * TranslationList::translate (char *ps)
{	if (!strcmp(ps,Pseudo)) return Original;
	else
	{	if (!Next) return 0;
		else return Next->translate(ps);
	}
}

class MakroRunAction : public Action
// action to run a makro
// lets the user choose parameters
{	Makro *Running;
	ParameterList *Param;
	TranslationList *TL;
	public :
	MakroRunAction () : Running(0),Param(0),TL(0) {}
	virtual void inform ();
	virtual void start ();
	int choose ();
	void menuchoose (int id);
	void startover ();
	virtual void abort ();
	virtual void perform (long c, long r);
} makrorunaction;

void MakroRunAction::inform ()
{   static char s[256];
	if (Running) sprintf(s,"%s : ",Running->name());
	else sprintf(s,"???");
	if (Param) strcat(s,Param->prompt());
	tools.inform(s);
}

void MakroRunAction::start ()
{	Param=Running->params();
	if (TL) delete TL;
	TL=0;
}

void MakroRunAction::startover ()
{	Param=Running->params();
	if (TL) delete TL;
	TL=0;
	objects.unhighlight(); objects.redraw();
	inform();
}

void MakroRunAction::abort ()
{	Param=0; objects.unhighlight(); objects.redraw();
	if (TL) delete TL;
	TL=0;
}

void MakroRunAction::perform (long c, long r)
{   if (!Running) return;
	double x=window.x(c),y=window.y(r);
	Object *o;
	if (!objects.select(x,y,o,Param->objectclass())) return;
	o->highlight.set(); objects.redraw();
	TranslationList *tl=new TranslationList(o->name(),Param->name());
	if (!TL) TL=tl;
	else TL->append(tl);
	Param=Param->next();
	if (!Param) // last parameter reached
	{   ConstructionStep *s=Running->step();
		translation=TL;
		while (s)
		{	if (!iolist.interpret(s->step())) break;
			s=s->next();
		}
		translation=0;
		startover();
		window.old();
	}
	else inform();
}

int MakroRunAction::choose ()
{	if (!makros)
	{	Warning(String(71),String(1),window);
		window.old();
		return 0;
	}
	makrosdialog.carryout(); // Choose between avail. Makros
	if (makrosdialog.result()!=Dialog::ok)
	{	window.old(); return 0;
	}
	int n=makrositem;
	Running=makros;
	while (n>0)
	{	Running=Running->next();
		n--;
	}
	return 1;
}

void MakroRunAction::menuchoose (int id)
{	Makro *m=makros;
	while (m)
	{	if (m->count()==id) break;
		m=m->next();
	}
	Running=m;
}

void domakrorun ()
// menu: run makro (F7)
{	if (!makros) return;
	if (!makrorunaction.choose()) return;
	window.action(makrorunaction);
}

Makro *selmakro ()
{	if (!makros) return 0;
	makrosdialog.carryout(); // Choose between avail. Makros
	if (makrosdialog.result()!=Dialog::ok) return 0;
	int n=makrositem;
	Makro *m=makros;
	while (n>0)
	{	m=m->next();
		n--;
	}
	return m;
}

void domakromenu ()
{	makrorunaction.menuchoose(menu.command());
	window.action(makrorunaction);
}

FileSelector makrosout(window,String(73),1,String(74),String(76));
FileSelector makrosin(window,String(73),1,String(75),String(77));

void Makro::save (char *filename)
{	FILE *out=fopen(filename,"w");
	char s[256];
	if (!out)
	{   sprintf(s,String(40),filename);
		Warning(s,String(1),window);
		return;
	}
	fprintf(out,"N:%s\n",name());
	ParameterList *p=P;
	while (p)
	{	fprintf(out,"P:%s\n",p->name());
		fprintf(out,"L:%s\n",p->prompt());
		switch (p->objectclass()&
			(Object::point|Object::line|Object::circle))
		{	case Object::point : fprintf(out,"T:point\n"); break;
			case Object::line : fprintf(out,"T:line\n"); break;
			case Object::circle : fprintf(out,"T:circle\n"); break;
		}
		p=p->next();
	}
	ConstructionStep *st=Step;
	while (st)
	{	fprintf(out,"S:%s",st->step());
		st=st->next();
	}
	fclose(out);
	if (ferror(out))
	{   sprintf(s,String(41),filename);
		Warning(s,String(1),window);
		return;
	}
}

void domakrosave ()
// menu: save makro
{   Makro *m=selmakro();
	if (!m) return;
	String name(makrosout.select());
	if (name.empty()) return;
	if (!*name.extension()) name.extension(String(79));
	m->save(name);
}

void domakroload ()
// menu: load makro
{   Makro *m;
	String name(makrosin.select());
	if (name.empty()) return;
	FILE *in=fopen(name,"r");
	char s[512],sw[612],pprompt[256],pname[256];
	int c;
	if (!in)
	{   sprintf(s,String(40),name);
		Warning(s,String(1),window);
		return;
	}
	if (!fgets(s,500,in)) goto error;
	stripnl(s);
	if (strncmp("N:",s,2)) goto error;
	m=new Makro(s+2);
	while(1)
	{	if (!fgets(s,500,in)) goto error;
		stripnl(s);
		if (!strncmp("S:",s,2)) break;
		if (strncmp("P:",s,2)) goto error;
		strcpy(pname,s+2);
		if (!fgets(s,500,in)) goto error;
		stripnl(s);
		if (strncmp("L:",s,2)) goto error;
		strcpy(pprompt,s+2);
		if (!fgets(s,500,in)) goto error;
		stripnl(s);
		if (strncmp("T:",s,2)) goto error;
		if (!strcmp(s+2,"point")) c=Object::point;
		else if (!strcmp(s+2,"line")) c=Object::line;
		else if (!strcmp(s+2,"circle")) c=Object::circle;
		else goto error;
		m->addparam(pname,pprompt,c);
	}
	while (!feof(in))
	{   if (strncmp("S:",s,2)) goto error;
		m->addstep(s+2);
		if (!fgets(s,500,in)) break;
	}
	fclose(in);
	makrodefineaction.createmakro(m);
	return;
	error :
	fclose(in);
	sprintf(sw,String(78),s);
	Warning(sw,String(1),window);
}

void domakrodelete ()
// menu: Delete Makro
{   Makro *m=selmakro();
	if (!m) return;
	m->remove();
}

// ******** Tools dialog ***************

class Values : public ValuesetItem
{	public :
	Values () : ValuesetItem(ID_Values,tools) {}
	virtual void init ();
	virtual void notify ();
	void call (char key);
	void shift (int n);
} values;

const int idn=18;
char ids[idn]=
{	IDBM_Point,IDBM_Line,IDBM_Segment,IDBM_Ray,IDBM_Circle,
	IDBM_Move,IDBM_Moveon,IDBM_Pointon,
	IDBM_Intersect,IDBM_Track,
	IDBM_Hide,IDBM_Edit,IDBM_Delete,
	IDBM_Lot,IDBM_Parallel,IDBM_Circle3,
	IDBM_Middle,IDBM_Angle
};
Action *actions[idn]=
{	&pointaction,&lineaction,&segmentaction,&rayaction,&circleaction,
	&moveaction,&moveonaction,&pointonaction,
	&intersectionaction,&trackaction,
	&hideaction,&editaction,&deleteaction,
	&rectangularaction,&parallelaction,&circle3action,
	&middleaction,&angleaction
};

class Value : public Bitmap
{   int Id,Col;
	char Key;
	Action *A;
	public :
	Value (int id, int col, char key, Action &a) : Bitmap(id)
	{	Id=id; A=&a; Col=col; Key=key;
		values.setbitmap(1,col,*this);
	}
	Action *action () { return A; }
	int key () { return Key; }
};

Value *value[idn];

void Values::init ()
{	int i;
	String s(22);
	char *keys=s;
	for (i=0; i<idn; i++)
	{	value[i]=new Value(ids[i],i+1,keys[i],*actions[i]);
	}
}

void Values::notify ()
{   window.action(*value[col()-1]->action());
}

void Values::call (char key)
{   int i;
	key=toupper(key);
	for (i=0; i<idn; i++)
	{	if (value[i]->key()==key)
		{	window.action(*value[i]->action());
			select(1,i+1);
			return;
		}
	}
}

void Values::shift (int n)
{	n+=col()-1;
	if (n>=idn) n-=idn;
	if (n<0) n+=idn;
	select(1,n+1);
}

int MainWindow::key (int flags, int code, int scan)
{   if (flags&Keycode::up) return 0;
	if (flags&Keycode::virtualkey)
	{   switch(scan)
		{	case VK_LEFT : values.shift(-1); break;
			case VK_RIGHT : values.shift(1); break;
			case VK_ESC :
				window.abort(); break;
		}
	}
	else values.call(code);
	return 0;
}

int ToolDialog::key (int flags, int code, int scan)
{   if (flags&Keycode::virtualkey && scan==VK_ESC)
	{	window.abort(); return 1;
	}
	values.call(code);
	return 1;
}

// ********* handle clicks ***********

void MainWindow::clicked (long c, long r, int click,
	int state)
{   if (A->drag) // drag action active
	{	if (click==ClickType::button1down ||
				click==ClickType::mousemove ||
				click==ClickType::button1up ||
			click==ClickType::button2down ||
				click==ClickType::button2up)
				A->dragperform(click,c,r);
		if (click==ClickType::button2up && Old) old();
			// right mouse drag finished
	}
	else if (click==ClickType::button1down) A->perform(c,r);
	else if (click==ClickType::button2down)
	{   action(moveaction);
		A->dragperform(click,c,r);
	}
}

// ********* Menu Entries **********

void doexit ()
{	if (window.close()) exit(0);
}

void dohidden ()
{	window.showhidden.toggle();
	objects.redraw();
	menu.check(IDM_Hidden,window.showhidden);
}

void dohelp ()
{   switch (menu.command())
	{	case IDM_General : help.general(); break;
		case IDM_Index : help.index(); break;
		case IDM_Keys : help.display(500); break;
		case IDM_Content : help.content(); break;
	}
}

void doinfo ()
{	String s("",256);
	sprintf(s,String(21),__DATE__);
	Message(s,String(1),window);
}

void doclear ()
{   if (objects.changed && !window.close()) return;
	window.abort();
	objects.clear();
	objectcolors.defcolor(0);
	objects.redraw();
	counts.reset();
	loadname.copy("");
	window.settitle();
	objects.comment().copy("");
}

void dobackspace ()
{   window.abort();
	if (!objects.first()) return;
	objects.clear(objects.last()->original());
	while (1)
	{	if (!objects.first()) break;
		if (!objects.last()->original()->hidden()) break;
		objects.clear(objects.last()->original());
	}
	objects.redraw();
}

// ****** laoding and saving *******

char *IO::write (Object &o)
{   *S=0;
	resadd(o);
	reswrite(o);
	sprintf(end(),"=%s(",Name);
	depwrite(o);
	strcat(end(),");\n");
	return S;
}

int Object::setflags (Strings &s)
{   int i;
	for (i=1; i<s.n(); i++)
	{	if (!strncmp(s[i],"hidden",6)) ishidden.set();
		else if (!strncmp(s[i],"color:",6))
			setcolor(objectcolors.index((char *)s[i]+6));
		else if (!strcmp(s[i],"showname"))
			showname.set();
		else return setspecialflag(s[i]);
	}
	return 1;
}

int Input::set (Object &o, int i)
{   if (translation)
	{   TranslationList *tl=
			new TranslationList(o.name(),left()[i][0]);
		translation->append(tl);
	}
	else
	{	if (objects.find(left()[i][0],0,&o))
		{   Warning(String(48),String(1),window);
			o.setname("???");
			return 0;
		}
		o.setname1(left()[i][0]);
	}
	if (!o.setflags(left()[i])) return 0;
	return 1;
}

Object *Input::object (int i, int flags)
{	return objects.find(right()[i],flags);
}

void IO::resadd (Object &o)
{   sprintf(end(),"\"%s\"(",o.name());
	if (o.ishidden)
	{   comma();
		strcat(end(),"hidden");
	}
	if (o.truecolor()!=0)
	{   comma();
		sprintf(end(),"color:%s",objectcolors.name(o.truecolor()));
	}
	if (o.showname)
	{	comma();
		sprintf(end(),"showname");
	}
	o.writespecialflags(*this);
	strcat(end(),")");
}

void IO::comma ()
{   if (*(end()-1)!='(') strcat(end(),",");
}

void IO::depadd (Object &o)
{   strcat(end(),"\"");
	strcat(end(),o.name());
	strcat(end(),"\"");
}

int IOList::interpret (char *line)
{   if (!input.read(line)) return 0;
	IO *io=First;
	while (io)
	{   if (!strcmp(io->name(),input.name()))
			return io->interpret(input);
		io=io->next();
	}
	return 0;
}

void ObjectList::save (char *filename)
{   char s[512];
	Object *o=First;
	FILE *out=fopen(filename,"w");
	if (!out)
	{   sprintf(s,String(40),filename);
		Warning(s,String(1),window); return;
	}
	// save comment
	char *p=Comment;
	int lc=0;
	fprintf(out,"window(%g,%g,%g,%g);\n",
		window.x1(),window.x2(),window.y1(),window.y2());
	fprintf(out,"/*\n");
	while (*p)
	{	while (*p && *p!=13)
		{	putc(*p++,out); lc++;
			if (*p==' ' && lc>60)
			{   p++;
				putc('\n',out);
				lc=0;
			}
		}
		if (*p)
		{	p+=2;
			if (lc>0) putc('\n',out);
			putc('\n',out);
			lc=0;
		}
	}
	if (lc>0) putc('\n',out);
	fprintf(out,"*/\n");
	// save objects
	while (o)
	{   fprintf(out,"%s",o->write());
		o=o->next();
	}
	if (ferror(out))
	{	sprintf(s,String(41),filename);
		Warning(s,String(1),window);
	}
	fclose(out);
	if (ferror(out))
	{	sprintf(s,String(41),filename);
		Warning(s,String(1),window);
	}
	else changed.clear();
}

void ObjectList::load (char *filename)
{	char s[512];
	char *p=s+2,*q=Comment;
	int nl=1;
	doclear();
	if (objects.first()) return;
	FILE *in=fopen(filename,"r");
	if (!in)
	{   sprintf(s,String(40),filename);
		Warning(s,String(1),window); return;
	}
	if (!fgets(s,510,in)) goto cont;
	stripnl(s);
	if (!strncmp(s,"window(",7))
	{   double x1,x2,y1,y2;
		if (sscanf(s,"window(%lg,%lg,%lg,%lg);",&x1,&x2,&y1,&y2)==4)
			window.setsize(x1-0.001,x2-0.001,y1+0.001,y2+0.001);
		if (!fgets(s,510,in)) goto cont;
		stripnl(s);
	}
	if (strncmp(s,"/*",2)) goto cont;
	while (!feof(in))
	{	while (*p)
		{	*q++=*p++;
			nl=0;
			if (q-(char *)Comment>Comment.size()-4)
			{	*q=0; goto cont;
			}
		}
		if (!fgets(s,510,in)) break;
		p=s;
		stripnl(s);
		if (!strncmp(s,"*/",2)) break;
		if (s[0])
		{	if (!nl) *q++=' ';
		}
		else
		{	*q++=13; *q++=10; nl=1;
		}
	}
	*q=0;
	while (!feof(in))
	{	if (!fgets(s,510,in)) break;
		if (!strncmp(s,"//",2)) continue;
		stripnl(s);
		cont:
		if (!iolist.interpret(s))
		{   Dialog error(window,IDD_Fileerror,help,2020);
			StringItem line(ID_Errorline,error,s,512);
			error.carryout();
			break;
		}
	}
	bitmap->invalid.set();
	window.update();
	fclose(in);
	changed.clear();
}

FileSelector out(window,String(47),1,String(42),String(51));
FileSelector in(window,String(47),0,String(43),String(52));

void ObjectList::copy ()
{	Clipboard clip;
	MetafilePS meta(window);
	draw(meta);
	clip.copy(meta);
}

void docomment ()
{	Dialog d(window,IDD_Comment,help,2040);
	MultilineItem text(IDC_Comment,d,
		objects.comment(),objects.comment().size());
	d.carryout();
	if (d.result()!=Dialog::ok) return;
	if (strcmp(objects.comment(),text)!=0)
	{	objects.changed.set();
		strcpy(objects.comment(),text);
	}
}

void dosaveas ()
{   String name(out.select());
	if (name.empty()) return;
	if (!*name.extension()) name.extension(String(80));
	loadname.copy(name);
	objects.save(loadname);
	loadname.copy(name);
	window.settitle();
}

void dosave ()
{   if (loadname.empty())
	{	dosaveas(); return;
	}
	objects.save(loadname);
}

int MainWindow::close ()
{   profile.writeint("square",window.square);
	if (!objects.changed || objects.empty()) return 1;
	switch (QuestionAbort(String(57),String(1),window))
	{	case Answers::yes :
			dosave();
			break;
		case Answers::abort : return 0;
	}
	return 1;
}

void doload ()
{   String name(in.select());
	if (name.empty()) return;
	objects.load(name);
	loadname.copy(name);
	docomment();
	loadname.copy(name);
	window.settitle();
}

void doload (char *name)
{   objects.load(name);
	loadname.copy(name);
	docomment();	loadname.copy(name);
	window.settitle();
}

void docolor ()
{	Dialog d(window,IDD_Color,help,2030);
	PointColor coloritem(IDP_Color,d,1,objectcolors.defcolor()+1);
	d.carryout();
	if (d.result()!=DID_OK) return;
	objectcolors.defcolor(coloritem.col()-1);
}

void dozoomin ()
{	window.zoomin();
}

void dozoomout ()
{	window.zoomout();
}

void docenter ()
{	window.center();
}

void docopy ()
{	objects.copy();
}

void dosquare ()
{	window.square.toggle();
	menu.check(IDM_Square,window.square);
	if (window.square) window.sized();
}

void dofont ()
{	fontselector.select(window);
	if (fontselector)
	{   Font font(fontselector);
		bitmap->setfont(font);
		objects.redraw();
	}
}

void dodeletetracks ()
{	objects.deletetracks();
	objects.redraw();
}

FileSelector savebmp(window,"*.bmp",1,String(59),String(58));

void dosavebitmap ()
{	char *name=savebmp.select();
	if (!name) return;
	Dialog d(window,IDD_Bitmap,help,2090);
	LongItem w(ID_BitmapWidth,d,bitmap->width());
	LongItem h(ID_BitmapHeight,d,bitmap->height());
	CheckItem a(ID_BitmapAspect,d,1);
	d.carryout();
	if (d.result()!=Dialog::ok) return;
	long w1,h1;
	if ((long)w<10) w1=10;
	else w1=w;
	if ((long)h<10) h1=10;
	else h1=h;
	if (a)
	{	if (w1==bitmap->width())
		{	if (h1!=bitmap->width())
			w1=h1*(double)bitmap->width()/bitmap->height();
		}
		else
		{	h1=w1*(double)bitmap->height()/bitmap->width();
		}
	}
	BitmapPS bm(w1,h1);
	objects.draw(bm);
	bm.save(name);
}

//********* Things for the choose selected objects dialog *****

class ColorCheckItem : public CheckItem
{   int I;
	public :
	ColorCheckItem (int id, Dialog &d, int i, int f) :
		CheckItem(id,d,f),I(i) {}
	virtual void notify ()
	{	objectcolors.choose(I,flag());
		bitmap->invalid.set();
		window.update();
	}
};

class ColorButtonItem : public ButtonItem
{	public :
	ColorButtonItem (int id, Dialog &d) : ButtonItem(id,d)
	{}
};

class ChooseDialog : public Dialog
{	static int Colorid[6];
	static int Buttonid[6];
	ColorCheckItem *Ci[6];
	ColorButtonItem *Bi[6];
	ButtonItem *All,*None;
	public :
	ChooseDialog ();
	~ChooseDialog ();
	void all ();
	void none ();
};

class AllItem : public ButtonItem
{   ChooseDialog *D;
	public :
	AllItem (ChooseDialog &d) : ButtonItem(ID_All,d),D(&d) {}
	virtual void notify();
};

class NoneItem : public ButtonItem
{   ChooseDialog *D;
	public :
	NoneItem (ChooseDialog &d) : ButtonItem(ID_None,d),D(&d) {}
	virtual void notify();
};

ChooseDialog::ChooseDialog () :
	Dialog(window,IDD_ChooseColors,help,2050)
{	int i;
	for (i=0; i<6; i++)
	{	Ci[i]=new ColorCheckItem(Colorid[i],*this,
			i,objectcolors.chosen(i));
		Bi[i]=new ColorButtonItem(Buttonid[i],*this);
	}
	All=new AllItem(*this);
	None=new NoneItem(*this);
	void all ();
	void none ();
}

ChooseDialog::~ChooseDialog ()
{	int i;
	for (i=0; i<6; i++)
	{	delete Ci[i]; delete Bi[i];
	}
	delete All;
	delete None;
}

int ChooseDialog::Colorid[6] =
{	ID_Black,ID_Blue,ID_Pink,ID_Green,ID_Cyan,ID_Brown};
int ChooseDialog::Buttonid[6] =
{	ID_B1,ID_B2,ID_B3,ID_B4,ID_B5,ID_B6};

void AllItem::notify ()
{	D->all();
}

void NoneItem::notify ()
{	D->none();
}

void ChooseDialog::all ()
{	int i;
	for (i=0; i<6; i++)
	{	Ci[i]->reinit(1);
		Ci[i]->notify();
	}
}

void ChooseDialog::none ()
{	int i;
	for (i=0; i<6; i++)
	{	Ci[i]->reinit(0);
		Ci[i]->notify();
	}
}

void dochoosecolors ()
{	ChooseDialog d;
	int note[6],i;
	for (i=0; i<6; i++) note[i]=objectcolors.chosen(i);
	d.carryout();
	if (d.result()==DID_OK) return;
	for (i=0; i<6; i++)
		objectcolors.choose(i,note[i]);
	bitmap->invalid.set();
	window.update();
}

void dosavepos ()
{	long x,y,w,h;
	window.getframe(x,y,w,h);
	profile.writelong("x",x);
	profile.writelong("y",y);
	profile.writelong("w",window.width());
	profile.writelong("h",window.height());
	tools.getframe(x,y,w,h);
	profile.writelong("xd",x);
	profile.writelong("yd",y);
}

void doblackwhite ()
{	blackwhite=!blackwhite;
	menu.check(IDM_BW,blackwhite);
	objects.redraw();
}

void dogrid ()
{	grid=!grid;
	menu.check(IDM_Grid,grid);
	objects.redraw();
}

void dogridset ()
{	Dialog d(window,IDD_Grid,help,2100);
	DoubleItem g1(ID_Grid,d,grid1);
	DoubleItem g2(ID_Gridthick,d,grid2);
	CheckItem a(ID_Axes,d,axes);
	d.carryout();
	if (d.result()!=Dialog::ok) return;
	if (g1<1e-10) grid1=1e-10;
	else grid1=g1;
	if (g2<2*grid1) grid2=2*grid1;
	else grid2=g2;
	axes=a;
	objects.redraw();
}

// ********** Printing *************

int fprint (Parameter p);

class PrintThread : public Thread
{   int Chosen,Size;
	public :
	PrintThread () : Thread(fprint) {}
	void set (int ch, int si)
	{	Chosen=ch; Size=si;
	}
	friend int fprint (Parameter P);
} print;

int fprint (Parameter p)
{   Queues q;
	q.choose(print.Chosen);
	PrinterPS ps(q,String(82));
	if (!ps.ready())
	{   window.sendsemaphore(1);
		return -1;
	}
	long x,y,w,h;
	w=ps.width(); h=w*(double)window.height()/window.width();
	if (h>ps.height())
	{	h=ps.height(); w=h*(double)window.width()*window.height();
	}
	w=w*(double)print.Size/100.0;
	h=h*(double)print.Size/100.0;
	x=ps.width()/2-w/2; y=ps.height()/2-h/2;
	ps.offset(x,y); ps.clip(0,0,w,h);
	ps.markersize(w/200);
	Font font("",h/20);
	ps.setfont(font);
	objects.draw(ps);
	return 0;
}

class PrinterItem : public ListItem
{   Queues *Q;
	public :
	PrinterItem (int id, Dialog &d, Queues &q) :
		ListItem(id,d),Q(&q) {}
	virtual void init ();
};

void PrinterItem::init ()
{   int i,n=0;
	for (i=0; i<Q->number(); i++)
	{	insert(Q->name(i));
		if (!strcmp(Q->name(i),printername)) n=i;
	}
	select(n);
}

class SizeItem : public SliderItem
{   public :
	SizeItem (int id, Dialog &d, int n) :
		SliderItem(id,d,n+1) {}
	virtual void init ();
};

void SizeItem::init ()
{   int i;
	SliderItem::init();
	for (i=0; i<=100; i+=10) tick(i);
	label(100,"100");
	label(0,"0");
}

void doprint ()
{   Dialog d(window,IDD_Print);
	Queues q;
	if (q.number()==0)
	{	Warning(String(81),String(1),window);
		return;
	}
	PrinterItem printer(ID_Printer,d,q);
	SizeItem size(ID_Size,d,printsize);
	d.carryout();
	if (d.result()!=Dialog::ok) return;
	print.set(printer,size);
	printername.copy(q.name(printer));
	print.start();
}

// *********** main ****************

int main (int argc, char *argv[])
{   window.top();
	window.active.set();
	tools.show();
	menu.add(IDM_Exit,doexit);
	menu.add(IDM_Hidden,dohidden);
	menu.add(IDM_General,dohelp);
	menu.add(IDM_Index,dohelp);
	menu.add(IDM_Content,dohelp);
	menu.add(IDM_Keys,dohelp);
	menu.add(IDM_Product,doinfo);
	menu.add(IDM_Clear,doclear);
	menu.add(IDM_Backspace,dobackspace);
	menu.add(IDM_Saveas,dosaveas);
	menu.add(IDM_Save,dosave);
	menu.add(IDM_Load,doload);
	menu.add(IDM_Color,docolor);
	menu.add(IDM_Comment,docomment);
	menu.add(IDM_Zoomin,dozoomin);
	menu.add(IDM_Zoomout,dozoomout);
	menu.add(IDM_Center,docenter);
	menu.add(IDM_Copy,docopy);
	menu.add(IDM_Font,dofont);
	menu.add(IDM_DeleteTracks,dodeletetracks);
	menu.add(IDM_Savebitmap,dosavebitmap);
	menu.add(IDM_Square,dosquare);
	menu.add(IDM_ShowSelected,dochoosecolors);
	menu.add(IDM_SavePosition,dosavepos);
	menu.add(IDM_MakroParameter,domakroparameter);
	menu.add(IDM_MakroConstructed,domakroconstructed);
	menu.add(IDM_MakroDefine,domakrodefine);
	menu.add(IDM_MakroRun,domakrorun);
	menu.add(IDM_MakroLoad,domakroload);
	menu.add(IDM_MakroSave,domakrosave);
	menu.add(IDM_MakroDelete,domakrodelete);
	menu.add(IDM_Print,doprint);
	menu.add(IDM_BW,doblackwhite);
	menu.add(IDM_Grid,dogrid);
	menu.add(IDM_GridSet,dogridset);
	if (argc==2) doload(argv[1]);
	long x,y,w,h,xd,yd,wd,hd;
	w=profile.readlong("w",window.width());
	h=profile.readlong("h",window.height());
	window.size(w,h);
	window.getframe(x,y,w,h);
	x=profile.readlong("x",20);
	y=profile.readlong("y",program.height()-20-h);
	window.framepos(x,y);
	tools.getframe(xd,yd,wd,hd);
	xd=x; yd=program.height()-20-h-hd-5;
	xd=profile.readlong("xd",xd);
	yd=profile.readlong("yd",yd);
	tools.framepos(xd,yd);
	window.action(pointaction);
	window.square=profile.readint("square",0);
	if (window.square)
	{	menu.check(IDM_Square,1);
		window.sized();
	}
	window.sizing.set();
	window.sized();
	program.loop();	return 0;
}

