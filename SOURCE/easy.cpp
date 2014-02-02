#include "easy.h"

#include <math.h>

static int window_invalid=0;
static StandardWindow *actwindow;

// ******************* Debug tools **********************

void dumplong (long n)
{	char s[256];
	sprintf(s,"%ld, %lx %c",n,n,(char)n);
	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,"Dump",0,
		MB_OK);
}

void dump (char *s)
{	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,"Dump",0,
		MB_OK);
}

// *********** Messages etc. ***************************

void Message (char *s, char *title)
{	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,title,0,
		MB_OK|MB_INFORMATION);
}

void Warning (char *s, char *title)
{	WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,title,0,
		MB_OK|MB_WARNING);
}

int Question (char *s, char *title)
{	return WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,title,0,
		MB_YESNO|MB_QUERY);
}

int QuestionAbort (char *s, char *title)
{	return WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,s,title,0,
		MB_YESNOCANCEL|MB_QUERY);
}

void Message (char *s, char *title, Window &w)
{	WinMessageBox(HWND_DESKTOP,w.handle(),s,title,0,
		MB_OK|MB_INFORMATION);
}

void Warning (char *s, char *title, Window &w)
{	WinMessageBox(HWND_DESKTOP,w.handle(),s,title,0,
		MB_OK|MB_WARNING);
}

int Question (char *s, char *title, Window &w)
{	return WinMessageBox(HWND_DESKTOP,w.handle(),s,title,0,
		MB_YESNO|MB_QUERY);
}

int QuestionAbort (char *s, char *title, Window &w)
{	return WinMessageBox(HWND_DESKTOP,w.handle(),s,title,0,
		MB_YESNOCANCEL|MB_QUERY);
}

void Beep (int f, double sec)
{	DosBeep(f,sec*1000);
}

// **************** Threads **********************

void _syscall ThreadProc (Thread *s)
{	s->call();
	s->finished();
	_endthread();
}

void Thread::start (Parameter p)
{   stop();
	P=p;
	Started=1; Expected=0;
	Tid=_beginthread(
		(void (_USERENTRY *) (void *))ThreadProc,
		Stacksize,Parameter(this));
	if (Tid==(TID)-1) Started=0;
}

void Thread::stop ()
{	if (Started) DosKillThread(Tid);
	Started=0; Expected=0;
}

void Thread::suspend ()
{	if (Started) DosSuspendThread(Tid);
}

void Thread::resume ()
{	if (Started) DosResumeThread(Tid);
}

void Thread::wait ()
{   TID tid=Tid;
	Expected=1;
	if (Started) DosWaitThread(&tid,DCWW_WAIT);
}

// **************** Time *************************

void Time::set ()
{   long sec;
	DosQuerySysInfo(QSV_TIME_LOW,QSV_TIME_LOW,
		&sec,sizeof(unsigned long));
	Seconds=sec;
}

// ********************* Strings ************************

long String::defaultsize=32;

String::String (char *text, long size)
{   if (!text) text="";
	Size=strlen(text);
	if (Size<size) Size=size;
	P=new char[Size+1];
	strcpy(P,text);
}

String::String (String &s)
{	Size=s.Size;
	P=new char[Size+1];
	strcpy(P,s.P);
}

String::String (int id)
{   char s[1024];
	WinLoadString(program.hab(),NULLHANDLE,id,1022,s);
	Size=strlen(s);
	P=new char[Size+1];
	strcpy(P,s);
}


String::String ()
{   Size=defaultsize;
	P=new char[Size+1];
	*P=0;
}

String::~String ()
{	delete P;
}

void String::copy (char *text, long size)
{	long textsize=strlen(text);
	if (textsize>size) size=textsize;
	if (Size<size)
	{   char *p=P;
		Size=size;
		P=new char[Size+1];
		strcpy(P,text);
		delete p;
	}
	else strcpy(P,text);
}

void String::copy (char *text)
{	long size=strlen(text);
	if (Size<size)
	{   char *p=P;
		Size=size;
		P=new char[Size+1];
		strcpy(P,text);
		delete p;
	}
	else strcpy(P,text);
}

void String::cat (char *text)
{   long total;
	if ((total=strlen(P)+strlen(text))>Size)
	{	copy(P,total);
	}
	strcat(P,text);
}

char *String::filename ()
{	char *p=P+strlen(P);
	while (p>P)
	{	if (*p=='\\' || *p==':') return p+1;
		p--;
	}
	return p;
}

char *String::extension ()
{	char *p=P+strlen(P);
	while (p>P)
	{	if (*p=='\\' || *p=='.') break;
		p--;
	}
	if (p>=P && *p=='.') return p;
	return P+strlen(P);
}

void String::extension (char *ext)
{   *extension()=0;
	cat(ext);
}

int String::testextension (char *ext)
{	char *p=extension();
	while (*p && *ext)
	{   if (::toupper(*p)!=::toupper(*ext)) return 0;
		p++; ext++;
	}
	if (!*p && !*ext) return 1;
	return 0;
}

void String::stripfilename ()
{	*filename()=0;
}

void String::setname (char *name)
{	stripfilename();
	cat(name);
}

int String::todouble (double &x)
{	int n;
	if (sscanf(P,"%lg%n",&x,&n)==0) return 0;
	char *p=P+n;
	if (*p) return 0;
	else return 1;
}

int String::tolong (long &x)
{	int n;
	if (sscanf(P,"%ld%n",&x,&n)==0) return 0;
	char *p=P+n;
	if (*p) return 0;
	else return 1;
}

void String::toupper ()
{	char *p=P;
	while (*p)
	{	*p=::toupper(*p);
		p++;
	}
}

// ****************** Rectangle *************************

void Rectangle::minsize (LONG wmin, LONG hmin)
{	if (W<0 && -W<wmin) W=-wmin;
	else if (W>=0 && W<wmin) W=wmin;
	if (H<0 && -H<hmin) H=-hmin;
	else if (H>=0 && H<hmin) H=hmin;
}

void Rectangle::hrescale (double scale)
{	H=(LONG)(scale*abs(W))*(H<0?-1:1);
}

void Rectangle::wrescale (double scale)
{	W=(LONG)(scale*abs(H))*(W<0?-1:1);
}

// ******************** Program *************************

MRESULT EXPENTRY MainWindowProc (HWND hwnd, unsigned long msg,
	MPARAM mp1, MPARAM mp2)
{	StandardWindow *window;
	int click;
	if (window_invalid) window=actwindow;
	else window=(StandardWindow *)WinQueryWindowPtr(hwnd,QWL_USER);
	POINTS *points;
	switch (msg)
	{   case WM_CREATE :
			window->Handle=hwnd;
			break;
		case WM_PRESPARAMCHANGED :
			window->pres_changed();
			goto def;
		case WM_PAINT :
			{	RedrawPS ps(hwnd,*window);
				window->redraw(ps);
			}
			break;
		case WM_SIZE :
			if (SHORT1FROMMP(mp2)==0) goto def;
			if (window->Width==SHORT1FROMMP(mp2) &&
				window->Height==SHORT2FROMMP(mp2)) goto def;
			window->Width=SHORT1FROMMP(mp2);
			window->Height=SHORT2FROMMP(mp2);
			window->sized();
			break;
		case WM_CHAR :
			if (window->key(SHORT1FROMMP(mp1),SHORT1FROMMP(mp2),
				SHORT2FROMMP(mp2))) break;
			goto def;
		case WM_HSCROLL :
			if (window->hscrolled(SHORT2FROMMP(mp2),SHORT1FROMMP(mp2)))
				break;
			goto def;
		case WM_VSCROLL :
			if (window->vscrolled(SHORT2FROMMP(mp2),SHORT1FROMMP(mp2)))
				break;
			goto def;
		case WM_BUTTON1CLICK :
			click=ClickType::button1; goto button;
		case WM_BUTTON2CLICK :
			click=ClickType::button2; goto button;
		case WM_BUTTON3CLICK :
			click=ClickType::button3; goto button;
		case WM_BUTTON1UP :
			click=ClickType::button1up; goto button;
		case WM_BUTTON2UP :
			click=ClickType::button2up; goto button;
		case WM_BUTTON3UP :
			click=ClickType::button3up; goto button;
		case WM_BUTTON1DOWN :
			click=ClickType::button1down; goto button;
		case WM_BUTTON2DOWN :
			click=ClickType::button2down; goto button;
		case WM_BUTTON3DOWN :
			click=ClickType::button3down; goto button;
		case WM_BUTTON1DBLCLK :
			click=ClickType::button1double; goto button;
		case WM_BUTTON2DBLCLK :
			click=ClickType::button2double; goto button;
		case WM_MOUSEMOVE :
			WinSetPointer(HWND_DESKTOP,window->New);
			click=ClickType::mousemove; goto button;
		case WM_BUTTON3DBLCLK :
			click=ClickType::button3double;
			button: points=(POINTS *)&mp1;
			window->clicked(points->x,points->y,click,
				SHORT2FROMMP(mp2));
			break;
		case WM_COMMAND :
			if (window->Dlg)
				if (window->Dlg->command(
					(Parameter)mp1,(Parameter)mp2)) break;
			if (window->Windowmenu)
				if (window->Windowmenu->call(SHORT1FROMMP(mp1)))
					break;
			goto def;
		case WM_SHOW :
			window->Visible=SHORT1FROMMP(mp1);
			goto def;
		case WM_CLOSE :
			if (window->close()) goto def;
			break;
		case WM_TIMER :
			window->timer(SHORT1FROMMP(mp1));
			break;
		case WM_SEM1 :
			window->semaphore(1,(long)mp1); break;
		case WM_SEM2 :
			window->semaphore(2,(long)mp1); break;
		case WM_SEM3 :
			window->semaphore(3,(long)mp1); break;
		case WM_SEM4 :
			window->semaphore(4,(long)mp1); break;
		default :
			def: return WinDefWindowProc(hwnd,msg,mp1,mp2);
	}
	return (MRESULT)FALSE;
}

Program::Program (int style, int size)
{	Hab=WinInitialize(0);
	Hmq=WinCreateMsgQueue(Hab,size);
	WinRegisterClass(Hab,"EasyMainWindow",MainWindowProc,
		style,sizeof(StandardWindow *));
	HPS hps=WinGetScreenPS(HWND_DESKTOP);
	GpiQueryFontMetrics(hps,sizeof(FONTMETRICS),
		&F);
	WinReleasePS(hps);
}

Program::~Program ()
{	WinDestroyMsgQueue(Hmq);
	WinTerminate(Hab);
}

inline int Program::getmessage ()
{	return WinGetMsg(Hab,&Qmsg,0L,0,0);
}

inline void Program::dispatch ()
{	WinDispatchMsg(Hab,&Qmsg);
}

void Program::loop ()
{	while(getmessage())
		dispatch();
}

void Program::clear_messages ()
{	while (WinPeekMsg(Hab,&Qmsg,0L,0,0,PM_REMOVE))
		dispatch();
}

// ************** Window ******************************

Window::Window ()
{	New=Old=WinQuerySysPointer(HWND_DESKTOP,pointer_arrow,TRUE);
}

void Window::size (long w, long h)
{   WinSetWindowPos(Handle,NULLHANDLE,0,0,
		w,h,SWP_SIZE);}

void Window::pos (long x, long y)
{	WinSetWindowPos(Handle,NULLHANDLE,x,y,
		0,0,SWP_MOVE);
}

void Window::pos (long &x, long &y, long &w, long &h)
{   SWP p;
	WinQueryWindowPos(Handle,&p);
	x=p.x; y=p.y; w=p.cx; h=p.cy;
}

void Window::setpointer (pointertype p)
{	New=WinQuerySysPointer(HWND_DESKTOP,p,FALSE);
	WinSetPointer(HWND_DESKTOP,New);
}

void Window::setpointer (Pointer &p)
{	New=p.handle();
	WinSetPointer(HWND_DESKTOP,New);
}

void Window::resetpointer ()
{	WinSetPointer(HWND_DESKTOP,Old);
	New=Old;
}

void Window::showpointer ()
{	WinShowPointer(HWND_DESKTOP,TRUE);
}

void Window::hidepointer ()
{	WinShowPointer(HWND_DESKTOP,FALSE);
}

// ************* Standard Window ***********************

int StandardWindow::msg_semaphore[4]=
	{ WM_SEM1,WM_SEM2,WM_SEM3,WM_SEM4 };

void StandardWindow::size (LONG w, LONG h)
{   RECTL r;
	r.xLeft=r.yBottom=0; r.xRight=w; r.yTop=h;
	WinCalcFrameRect(FrameHandle,&r,FALSE);
	SWP s;
	WinQueryWindowPos(FrameHandle,&s);
	h=r.yTop-r.yBottom;
	WinSetWindowPos(FrameHandle,NULLHANDLE,
		s.x,(s.y+s.cy)-h,
		r.xRight-r.xLeft,h,
		SWP_SIZE|SWP_MOVE);
}

void StandardWindow::framepos (long x, long y)
{	WinSetWindowPos(FrameHandle,NULLHANDLE,x,y,
		0,0,SWP_MOVE);
}

void StandardWindow::getframe (long &x, long &y,
	long &w, long &h)
{	SWP s;
	WinQueryWindowPos(FrameHandle,&s);
	x=s.x; y=s.y; w=s.cx; h=s.cy;
}

void StandardWindow::center ()
{	long x,y,w,h;
	getframe(x,y,w,h);
	framepos(program.width()/2-w/2,program.height()/2-h/2);
}

void StandardWindow::init (int style)
{   window_invalid=1;
	actwindow=this;
	FrameHandle=WinCreateStdWindow(HWND_DESKTOP,
		style,&Flags,"EasyMainWindow",
		(char *)Name,0,
		(HMODULE)0,Id,&Handle);
	WinSetWindowPtr(Handle,QWL_USER,this);
	window_invalid=0;
	Windowmenu=NULL;
	Visible=1;
}

StandardWindow::~StandardWindow ()
{	WinDestroyWindow(FrameHandle);
}

StandardWindow::StandardWindow (int id, char *name,
	unsigned long flags)
	: Name(name),Dlg(0)
{	Id=id; Flags=flags;
}

void StandardWindow::top ()
{	WinSetWindowPos(FrameHandle,
		HWND_TOP,0,0,0,0,SWP_ZORDER);
	WinSetFocus(HWND_DESKTOP,Handle);
}

int StandardWindow::topped ()
{	return WinQueryWindow(HWND_DESKTOP,QW_TOP)==FrameHandle;
}

int StandardWindow::rubberbox (LONG x, LONG y, int click,
	Rectangle &R, LONG wmin, LONG hmin, double wscale, double hscale)
{   WindowPS ps(*this);
	ps.mix(FM_XOR);
	switch (click)
	{	case ClickType::button1down :
			capture();
			Rubber=1;
			R=Rectangle(x,y,wmin,hmin);
			ps.frame(R,0,CLR_GREEN);
			return RUBBER_START;
		case ClickType::mousemove :
			if (!Rubber) break;
			ps.frame(R,0,CLR_GREEN);
			R.resize(x-R.x(),y-R.y());
			R.minsize(hmin,wmin);
			if (wscale>0) R.wrescale(wscale);
			if (hscale>0) R.hrescale(hscale);
			ps.frame(R,0,CLR_GREEN);
			Rubber=2;
			break;
		case ClickType::button1up :
			capture(0);
			ps.frame(R,0,CLR_GREEN);
			if (!Rubber || Rubber==1)
			{	Rubber=0;
				return RUBBER_CANCEL;
			}
			Rubber=0;
			if (abs(y-R.y())>=hmin) return RUBBER_DONE;
			else return RUBBER_CANCEL;
	}
	return RUBBER_ZERO;
}

void StandardWindow::vscroll (int pos, int min, int max)
{	HWND h=WinWindowFromID(FrameHandle,FID_VERTSCROLL);
	WinSendMsg(h,SBM_SETSCROLLBAR,MPFROMSHORT(pos),
		MPFROM2SHORT(min,max));
}

void StandardWindow::hscroll (int pos, int min, int max)
{	HWND h=WinWindowFromID(FrameHandle,FID_HORZSCROLL);
	WinSendMsg(h,SBM_SETSCROLLBAR,MPFROMSHORT(pos),
		MPFROM2SHORT(min,max));
}

void StandardWindow::title (char *s)
{	HWND h=WinWindowFromID(FrameHandle,FID_TITLEBAR);
	Name.copy(s);
	WinSetWindowText(h,Name);
}

// ************* Menus ************************************

Menu::~Menu ()
{   Menuentry *Mp1;
	while (Mp)
	{   Mp1=Mp;
		Mp=Mp->next();
		delete Mp1;
	}
}

int Menu::call (int command)
{	Menuentry *m=Mp;
	Command=command;
	while (m)
		if (m->command()==command)
		{	m->call(command);
			return 1;
		}
		else m=m->next();
	return 0;
}

void Menu::enable (int id, int flag)
{	HWND handle=WinWindowFromID(W->framehandle(),FID_MENU);
	WinSendMsg(handle,MM_SETITEMATTR,
		MPFROM2SHORT(id,TRUE),MPFROM2SHORT(MIA_DISABLED,
		flag?0:MIA_DISABLED));
}

void Menu::check (int id, int flag)
{	HWND handle=WinWindowFromID(W->framehandle(),FID_MENU);
	WinSendMsg(handle,MM_SETITEMATTR,
		MPFROM2SHORT(id,TRUE),MPFROM2SHORT(MIA_CHECKED,
		flag?MIA_CHECKED:0));
}

HWND Menu::submenu (int id)
{   MENUITEM mi;
	HWND handle=WinWindowFromID(W->framehandle(),FID_MENU);
	WinSendMsg(handle,MM_QUERYITEM,
		MPFROM2SHORT(id,TRUE),(MPARAM)&mi);
	return mi.hwndSubMenu;
}

void Menu::additem (int id,
	Menuproc *proc, char *entry, int menuid, int pos)
{	HWND handle=submenu(menuid);
	MENUITEM mi;
	mi.iPosition=pos;
	mi.afStyle=MIS_TEXT;
	mi.afAttribute=0;
	mi.id=id;
	mi.hwndSubMenu=NULLHANDLE;
	mi.hItem=0;
	WinSendMsg(handle,MM_INSERTITEM,(MPARAM)&mi,
		(MPARAM)entry);
	add(id,proc);
}

void Menu::removeitem (int id, int menuid)
{	HWND handle=submenu(menuid);
	WinSendMsg(handle,MM_DELETEITEM,
		MPFROM2SHORT(id,TRUE),NULL);
	remove(id);
}

void Menu::remove (int id)
{	Menuentry *mp=Mp;
	if (!mp) return;
	if (mp && mp->command()==id)
	{	mp=mp->next();
		delete mp;
		return;
	}
	while (mp)
	{	if (mp->next()->command()==id)
		{	mp->next(mp->next()->next());
			delete mp->next();
			break;
		}
	}
}

//******************** Palettes ***************************

Palette::Palette (unsigned long a[], int n, int opt)
{   long option=0;
	if (opt&pure) option|=LCOL_PURECOLOR;
	if (opt&own) option|=LCOL_OVERRIDE_DEFAULT_COLORS;
	Handle=GpiCreatePalette(program.hab(),option,LCOLF_CONSECRGB,n,a);
}

Palette::~Palette ()
{	GpiDeletePalette(Handle);
}

// ************* Presentation Space class (PS) *************

void PS::color (Color color)
{   if (!supports(Support::color))
	{	if (color!=CLR_WHITE && color!=CLR_NEUTRAL)
			color=CLR_BLACK;
	}
	if (C!=color)
	{   if (!color.direct() && C.direct())
		{	defaultcolors();
		}
		else if (color.direct() && !C.direct())
		{	directcolor(Pure);
		}
		GpiSetColor(Handle,color);
		C=color;
	}
}

void PS::clip (long x, long y, long w, long h)
{	RECTL r;
	r.xLeft=x+X; r.xRight=x+X+w-1; r.yBottom=y+Y; r.yTop=y+Y+h-1;
	GpiIntersectClipRectangle(Handle,&r);
	S.cx=w; S.cy=h;
}

void PS::directcolor (int pure)
{	GpiCreateLogColorTable(Handle,pure?LCOL_PURECOLOR:0,
				LCOLF_RGB,0,0,NULL);
}

void PS::defaultcolors ()
{	GpiCreateLogColorTable(Handle,LCOL_RESET,0,0,0,NULL);
}

void PS::move (LONG c, LONG r, Color col)
{   color(col);
	P.x=c+X; P.y=r+Y; GpiMove(Handle,&P);
}

void PS::linerel (LONG w, LONG h, Color col)
{   color(col);
	P.x+=w; P.y+=h; GpiLine(Handle,&P);
}

void PS::lineto (LONG c, LONG r, Color col)
{   color(col);
	P.x=c+X; P.y=r+Y; GpiLine(Handle,&P);
}

void PS::linetrunc (LONG c1, LONG r1, LONG c2, LONG r2, Color col)
{   LONG h;
	P.x=X+c2; P.y=Y+r2;
	if (c2<c1)
	{	h=c2; c2=c1; c1=h;
		h=r2; r2=r1; r1=h;
	}
	if (c1<0)
	{	if (c2<0) return;
		r1+=(r2-r1)*(double)(-c1)/(c2-c1);
		c1=0;
	}
	if (c2>width()-1)
	{	if (c1>width()-1) return;
		r2-=(r2-r1)*(double)(c2-(width()-1))/(c2-c1);
		c2=width()-1;
	}
	if (r2<r1)
	{	h=c2; c2=c1; c1=h;
		h=r2; r2=r1; r1=h;
	}
	if (r1<0)
	{	if (r2<0) return;
		c1+=(c2-c1)*(double)(-r1)/(r2-r1);
		r1=0;
	}
	if (r2>height()-1)
	{	if (r1>height()-1) return;
		c2-=(c2-c1)*(double)(r2-(height()-1))/(r2-r1);
		r2=height()-1;
	}
	color(col);
	c1+=X; r1+=Y; c2+=X; r2+=Y;
	POINTL p;
	p.x=c1; p.y=r1;
	GpiMove(Handle,&p);
	p.x=c2; p.y=r2;
	GpiLine(Handle,&p);
}

void PS::point (LONG c, LONG r, Color col)
{   color(col);
	P.x=c+X; P.y=r+Y; GpiSetPel(Handle,&P);
}

void PS::text (char *s, Color col, unsigned long al, unsigned long val)
{	color(col);
	GpiSetTextAlignment(Handle,al,val);
	GpiCharString(Handle,strlen(s),s);
}

void PS::clippedtext (char *s, long x, long y,
	long x1, long y1, long x2, long y2,
	Color col, unsigned long al, unsigned long val)
{   RECTL r={x1,y1,x2,y2};
	POINTL p={x,y};
	color(col);
	GpiSetTextAlignment(Handle,al,val);
	GpiCharStringPosAt(Handle,&p,&r,CHS_CLIP,strlen(s),s,NULL);
}

void PS::textbox (char *text, long &width, long &height)
{   POINTL pts[4];
	GpiQueryTextBox(Handle,strlen(text),text,4,pts);
	width=pts[2].x-pts[0].x;
	height=pts[0].y-pts[3].y;
}

double PS::textextend (char *text, double vx, double vy)
{   POINTL pts[4];
	int i;
	double emin,emax,r,h;
	GpiQueryTextBox(Handle,strlen(text),text,4,pts);
	r=sqrt(vx*vx+vy*vy); vx/=r; vy/=r;
	pts[0].x--; pts[1].x--;
	pts[2].x++; pts[2].x++;
	emin=emax=vx*pts[0].x+vy*pts[0].y;
	for (i=1; i<4; i++)
	{	h=vx*pts[i].x+vy*pts[i].y;
		if (h<emin) emin=h;
		else if (h>emax) emax=h;
	}
	return emax-emin;
}

void PS::frame (LONG w, LONG h, int r, Color col)
{	color(col);
	P.x+=w-1; P.y+=h-1; GpiBox(Handle,DRO_OUTLINE,&P,r,r);
}

void PS::area (LONG w, LONG h, int r, Color col)
{	color(col);
	GpiSetBackMix(Handle,BM_OVERPAINT);
	GpiSetPattern(Handle,PATSYM_SOLID);
	P.x+=w-1; P.y+=h-1; GpiBox(Handle,DRO_FILL,&P,r,r);
	GpiSetBackMix(Handle,BM_LEAVEALONE);
}

void PS::backarea (LONG w, LONG h)
{	GpiSetBackMix(Handle,BM_OVERPAINT);
	GpiSetPattern(Handle,PATSYM_BLANK);
	P.x+=w-1; P.y+=h-1; GpiBox(Handle,DRO_FILL,&P,0,0);
	GpiSetBackMix(Handle,BM_LEAVEALONE);
}

void PS::framedarea (LONG w, LONG h, int r, Color col)
{	color(col);
	GpiSetBackMix(Handle,BM_OVERPAINT);
	GpiSetPattern(Handle,PATSYM_BLANK);
	P.x+=w-1; P.y+=h-1; GpiBox(Handle,DRO_OUTLINEFILL,&P,r,r);
	GpiSetBackMix(Handle,BM_LEAVEALONE);
}

void PS::frame (Rectangle &R, int r, Color col)
{	move(R.x1(),R.y1());
	frame(abs(R.w()),abs(R.h()),r,col);
}

void PS::area (Rectangle &R, int r, Color col)
{	move(R.x1(),R.y1());
	area(abs(R.w()),abs(R.h()),r,col);
}

void PS::framedarea (Rectangle &R, int r, Color col)
{	move(R.x1(),R.y1());
	framedarea(abs(R.w()),abs(R.h()),r,col);
}

void PS::gpimark (LONG c, LONG r, unsigned long type, Color col)
{   color(col);
	GpiSetMarker(Handle,type);
	P.x=c+X; P.y=r+Y; GpiMarker(Handle,&P);
}

void PS::mark (LONG c, LONG r, unsigned long type, Color col)
{	switch (type)
	{   case Markers::plus :
			move(c+Ms,r); lineto(c-Ms,r,col);
			move(c,r-Ms); lineto(c,r+Ms,col);
			break;
		case Markers::star :
			move(c+Ms,r); lineto(c-Ms,r,col);
			move(c,r-Ms); lineto(c,r+Ms,col);
			move(c+Ms,r+Ms); lineto(c-Ms,r-Ms,col);
			move(c+Ms,r-Ms); lineto(c-Ms,r+Ms,col);
			break;
		case Markers::eightpointstar :
			move(c+Ms,r); lineto(c-Ms,r,col);
			move(c,r-Ms); lineto(c,r+Ms,col);
			move(c+Ms/2,r+Ms/2); lineto(c-Ms/2,r-Ms/2,col);
			move(c+Ms/2,r-Ms); lineto(c-Ms/2,r+Ms,col);
			break;
		case Markers::cross :
			move(c+Ms,r+Ms); lineto(c-Ms,r-Ms,col);
			move(c+Ms,r-Ms); lineto(c-Ms,r+Ms,col);
			break;
		case Markers::soliddiamond :
		case Markers::diamond : move(c+Ms,r);
			lineto(c,r+Ms,col); lineto(c-Ms,r,col);
			lineto(c,r-Ms,col); lineto(c+Ms,r,col);
			break;
		case Markers::dot : point(c,r,col); break;
		case Markers::circle :
			circle(c,r,Ms/2,1,col); break;
		case Markers::blank : break;
		case Markers::solidsquare :
			move(c-Ms,r-Ms); area(c+Ms,r+Ms,0,col);
		default : move(c+Ms,r+Ms);
			lineto(c-Ms,r+Ms,col); lineto(c-Ms,r-Ms,col);
			lineto(c+Ms,r-Ms,col); lineto(c+Ms,r+Ms,col);
	}}

void PS::circle (LONG c, LONG r, LONG rad, double factor,
	Color col)
{   ARCPARAMS p;
	p.lP=rad; p.lS=0;
	p.lQ=rad*factor; p.lR=0;
	GpiSetArcParams(Handle,&p);
	move(c,r);
	color(col);
	GpiFullArc(Handle,DRO_OUTLINE,65536l);
}

void PS::filledcircle (LONG c, LONG r, LONG rad, double factor,
	Color col, Color fillcol)
{   ARCPARAMS p;
	p.lP=rad; p.lS=0;
	p.lQ=rad*factor; p.lR=0;
	GpiSetArcParams(Handle,&p);
	move(c,r);
	color(fillcol);
	if ((long)col==CLR_DEFAULT) GpiFullArc(Handle,DRO_OUTLINEFILL,65536l);
	else GpiFullArc(Handle,DRO_FILL,65536l);
	if ((long)col!=CLR_DEFAULT)
	{	move(c,r);
		color(col);
		GpiFullArc(Handle,DRO_OUTLINE,65536l);
	}
}

void PS::arc (LONG c, LONG r, LONG rad, double factor,
	double phi1, double phi2, Color col)
{   ARCPARAMS p;
	p.lP=rad; p.lS=0;
	p.lQ=rad*factor; p.lR=0;
	GpiSetArcParams(Handle,&p);
	move(c,r);
	color(col);
	double delta;
	if (phi1<0) phi1+=360;
	if (phi2<0) phi2+=360;
	if (phi2>=phi1) delta=phi2-phi1;
	else delta=phi2+360-phi1;
	GpiSetLineType(Handle,LINETYPE_INVISIBLE);
	GpiPartialArc(Handle,&P,65536l,phi1*65536l,0);
	GpiSetLineType(Handle,LINETYPE_SOLID);
	GpiPartialArc(Handle,&P,65536l,
		phi1*65536l,delta*65536l);
}

void PS::filledpie (LONG c, LONG r, LONG rad, double factor,
	double phi1, double phi2, Color col, Color fillcol)
{   ARCPARAMS p;
	double delta;
	p.lP=rad; p.lS=0;
	p.lQ=rad*factor; p.lR=0;
	GpiSetArcParams(Handle,&p);
	if (phi1<0) phi1+=360;
	if (phi2<0) phi2+=360;
	if (phi2>=phi1) delta=phi2-phi1;
	else delta=phi2+360-phi1;
	color(fillcol);
	if ((long)col==CLR_DEFAULT) GpiBeginArea(Handle,BA_BOUNDARY);
	else GpiBeginArea(Handle,BA_NOBOUNDARY);
	move(c,r);
	GpiSetLineType(Handle,LINETYPE_SOLID);
	GpiPartialArc(Handle,&P,65536l,phi1*65536l,delta*65536l);
	GpiLine(Handle,&P);
	GpiEndArea(Handle);
	if ((long)col!=CLR_DEFAULT)
	{   color(col);
		GpiPartialArc(Handle,&P,65536l,
			phi1*65536l,delta*65536l);
		GpiLine(Handle,&P);
	}
}

void PS::pie (LONG c, LONG r, LONG rad, double factor,
	double phi1, double phi2, Color col)
{   ARCPARAMS p;
	double delta;
	p.lP=rad; p.lS=0;
	p.lQ=rad*factor; p.lR=0;
	GpiSetArcParams(Handle,&p);
	if (phi1<0) phi1+=360;
	if (phi2<0) phi2+=360;
	if (phi2>=phi1) delta=phi2-phi1;
	else delta=phi2+360-phi1;
	move(c,r);
	color(col);
	GpiSetLineType(Handle,LINETYPE_SOLID);
	GpiPartialArc(Handle,&P,65536l,phi1*65536l,delta*65536l);
	GpiLine(Handle,&P);
}

void PS::setfont (Font &font, int id)
{   GpiCreateLogFont(Handle,NULL,id,font.fat());
	GpiSetCharSet(Handle,id);
}

void PS::image (long w, long h, unsigned char *data)
{   SIZEL s;
	s.cx=w; s.cy=h;
	GpiImage(Handle,0,&s,((w-1)/8+1)*h,data);
}

void PS::rectcopy (PS &ps, long x, long y, long w, long h,
		long x1, long y1, int mode)
{	POINTL a[4];
	RECTL r;
	r.xLeft=a[0].x=x; r.yBottom=a[0].y=y;
	r.xRight=a[1].x=x+w-1; r.yTop=a[1].y=y+h-1;
	a[2].x=x1; a[2].y=y1;
	if (!GpiRectVisible(ps.handle(),&r)) return;
	GpiBitBlt(ps.handle(),Handle,3,a,mode,BBO_IGNORE);
}

void PS::setcharbox (long x, long y)
{	SIZEF s;
	s.cx=x*65536l; s.cy=y*65536l;
	GpiSetCharBox(Handle,&s);
}

//************ Bitmaps Presentation Spaces *****************

BitmapPS::BitmapPS (Window &window)
{	PSZ pszData[4]={"Display",NULL,NULL,NULL};
	BITMAPINFOHEADER2 bmp;
	LONG alData[2],size;
	DeviceHandle=DevOpenDC(program.hab(),OD_MEMORY,"*",4,
		(PDEVOPENDATA)pszData,NULLHANDLE);
	S.cx=window.width(); S.cy=window.height();
	Handle=GpiCreatePS(program.hab(),DeviceHandle,&S,
		PU_PELS|GPIA_ASSOC|GPIT_MICRO);
	GpiQueryDeviceBitmapFormats(Handle,2,(PLONG)alData);
	bmp.cbFix=(unsigned long)sizeof(BITMAPINFOHEADER2);
	bmp.cx=S.cx; bmp.cy=S.cy;
	Planes=bmp.cPlanes=alData[0];
	Colorbits=bmp.cBitCount=alData[1];
	bmp.ulCompression=BCA_UNCOMP;
	bmp.cbImage=(((S.cx*(1<<bmp.cPlanes)*(1<<bmp.cBitCount))+31)
		/32)*S.cy;
	bmp.cxResolution=70; bmp.cyResolution=70;
	bmp.cclrUsed=2; bmp.cclrImportant=0;
	bmp.usUnits=BRU_METRIC; bmp.usReserved=0;
	bmp.usRecording=BRA_BOTTOMUP; bmp.usRendering=BRH_NOTHALFTONED;
	bmp.cSize1=0; bmp.cSize2=0;
	bmp.ulColorEncoding=BCE_RGB; bmp.ulIdentifier=0;
	size=sizeof(BITMAPINFO2)+
		(sizeof(RGB2)*(1<<bmp.cPlanes)*(1<<bmp.cBitCount));
	if (DosAllocMem((PVOID *)&Info,size,PAG_COMMIT|PAG_READ|PAG_WRITE))
	{	Valid=0;
		GpiDestroyPS(Handle);
		DevCloseDC(DeviceHandle);
	}
	else Valid=1;
	Info->cbFix=bmp.cbFix;
	Info->cx=bmp.cx; Info->cy=bmp.cy;
	Info->cPlanes=bmp.cPlanes; Info->cBitCount=bmp.cBitCount;
	Info->ulCompression=BCA_UNCOMP;
	Info->cbImage=((S.cx+31)/32)*S.cy;
	Info->cxResolution=70; Info->cyResolution=70;
	Info->cclrUsed=2; Info->cclrImportant=0;
	Info->usUnits=BRU_METRIC;
	Info->usReserved=0;
	Info->usRecording=BRA_BOTTOMUP;
	Info->usRendering=BRH_NOTHALFTONED;
	Info->cSize1=0; Info->cSize2=0;
	Info->ulColorEncoding=BCE_RGB; Info->ulIdentifier=0;
	BitmapHandle=GpiCreateBitmap(Handle,&bmp,FALSE,NULL,Info);
	GpiSetBitmap(Handle,BitmapHandle);
	GpiErase(Handle);
}

BitmapPS::BitmapPS (long w, long h)
{	PSZ pszData[4]={"Display",NULL,NULL,NULL};
	BITMAPINFOHEADER2 bmp;
	LONG alData[2],size;
	DeviceHandle=DevOpenDC(program.hab(),OD_MEMORY,"*",4,
		(PDEVOPENDATA)pszData,NULLHANDLE);
	S.cx=w; S.cy=h;
	Handle=GpiCreatePS(program.hab(),DeviceHandle,&S,
		PU_PELS|GPIA_ASSOC|GPIT_MICRO);
	GpiQueryDeviceBitmapFormats(Handle,2,(PLONG)alData);
	bmp.cbFix=(unsigned long)sizeof(BITMAPINFOHEADER2);
	bmp.cx=S.cx; bmp.cy=S.cy;
	Planes=bmp.cPlanes=alData[0];
	Colorbits=bmp.cBitCount=alData[1];
	bmp.ulCompression=BCA_UNCOMP;
	bmp.cbImage=(((S.cx*(1<<bmp.cPlanes)*(1<<bmp.cBitCount))+31)
		/32)*S.cy;
	bmp.cxResolution=70; bmp.cyResolution=70;
	bmp.cclrUsed=2; bmp.cclrImportant=0;
	bmp.usUnits=BRU_METRIC; bmp.usReserved=0;
	bmp.usRecording=BRA_BOTTOMUP; bmp.usRendering=BRH_NOTHALFTONED;
	bmp.cSize1=0; bmp.cSize2=0;
	bmp.ulColorEncoding=BCE_RGB; bmp.ulIdentifier=0;
	size=sizeof(BITMAPINFO2)+
		(sizeof(RGB2)*(1<<bmp.cPlanes)*(1<<bmp.cBitCount));
	if (DosAllocMem((PVOID *)&Info,size,PAG_COMMIT|PAG_READ|PAG_WRITE))
	{	Valid=0;
		GpiDestroyPS(Handle);
		DevCloseDC(DeviceHandle);
	}
	else Valid=1;
	Info->cbFix=bmp.cbFix;
	Info->cx=bmp.cx; Info->cy=bmp.cy;
	Info->cPlanes=bmp.cPlanes; Info->cBitCount=bmp.cBitCount;
	Info->ulCompression=BCA_UNCOMP;
	Info->cbImage=((S.cx+31)/32)*S.cy;
	Info->cxResolution=70; Info->cyResolution=70;
	Info->cclrUsed=2; Info->cclrImportant=0;
	Info->usUnits=BRU_METRIC;
	Info->usReserved=0;
	Info->usRecording=BRA_BOTTOMUP;
	Info->usRendering=BRH_NOTHALFTONED;
	Info->cSize1=0; Info->cSize2=0;
	Info->ulColorEncoding=BCE_RGB; Info->ulIdentifier=0;
	BitmapHandle=GpiCreateBitmap(Handle,&bmp,FALSE,NULL,Info);
	GpiSetBitmap(Handle,BitmapHandle);
	GpiErase(Handle);
}

BitmapPS::~BitmapPS ()
{   if (!Valid) return;
	GpiDeleteBitmap(BitmapHandle);
	DosFreeMem(Info);
	GpiDestroyPS(Handle);
	DevCloseDC(DeviceHandle);
}

void BitmapPS::copy (PS &ps, long x, long y, int mode)
{	POINTL a[4];
	RECTL r;
	r.xLeft=a[0].x=x; r.yBottom=a[0].y=y;
	r.xRight=a[1].x=x+S.cx; r.yTop=a[1].y=y+S.cy;
	a[2].x=0; a[2].y=0;
	if (!GpiRectVisible(ps.handle(),&r)) return;
	GpiBitBlt(ps.handle(),Handle,3,a,mode,BBO_IGNORE);
}

void BitmapPS::save (char *filename)
{	FILE *f;
	BITMAPFILEHEADER2 bfh;
	char *buffer;
	PBITMAPINFO2 pbmi;
	long size,colorsize,hdsize=20;
	unsigned long compression=BCA_UNCOMP;
	int bits;
	f=fopen(filename,"wb");
	if (!f)
	{   Warning("Could not open\nthat file.","Save Error");
		return;
	}
	bfh.usType=BFT_BMAP;
	if (Colorbits>8) bits=24;
	else bits=Colorbits;
	colorsize=((bits<=8)?(1<<bits):256)*sizeof(RGB2);
	size=((width()*bits+31)/32)*4*height();
	bfh.offBits=14+hdsize+colorsize;
	bfh.cbSize=14+hdsize+colorsize+size;
	buffer=(char *)malloc(size);
	pbmi=(PBITMAPINFO2)malloc(16+colorsize);
	memset(pbmi,0,sizeof(BITMAPINFOHEADER2));
	pbmi->cbFix=hdsize;
	pbmi->cPlanes=1;
	pbmi->cBitCount=bits;
	pbmi->ulCompression=compression;
	if (GpiQueryBitmapBits(handle(),0,height(),buffer,pbmi)!=height())
	{   Warning("Bitmap Save Error!","Error");
		goto end;
	}
	fwrite((char *)&bfh,14,1,f);
	fwrite((char *)pbmi,hdsize+colorsize,1,f);
	fwrite(buffer,1,size,f);
	end: free(pbmi);
	free(buffer);
	fclose(f);
}

// ************ Printer ***********************

Queues::Queues (char *name) : Queues(0),NQueues(0),Default(0)
{   int i;
	getall();
	if (NQueues==0) return;
	if (name[0])
	{	for (i=0; i<NQueues; i++)
			if (!strcmp(name,Queues[i].pszName))
			{	Default=i;
				break;
			}
	}
}

void Queues::getall ()
{   unsigned long ret,size;
	SplEnumQueue(NULL,3,Queues,0,
		&ret,&NQueues,&size,NULL);
	if (NQueues==0) return;
	Queues=(PRQINFO3 *)(new char[size]);
	SplEnumQueue(NULL,3,Queues,size,
		&ret,&NQueues,&size,NULL);
}

PrinterPS::PrinterPS (Queues &q, char *name, int op)
	: Valid(0),Myname(name),Open(0)
{   if (q.number()==0) return;
	memmove(&Queue,q.chosen(),sizeof(PRQINFO3));
	Valid.set();
	if (op) open();
	notsupport(Support::erase);
}

void PrinterPS::open ()
{   if (Open) return;
	Dos.pszLogAddress=Queue.pszName;
	Dos.pszDriverName=Queue.pszDriverName;
	*strchr(Dos.pszDriverName,'.')=0;
	Dos.pdriv=Queue.pDriverData;
	Dos.pszDataType="PM_Q_STD";
	Dos.pszComment=Myname;
	Dos.pszQueueProcName=Queue.pszPrProc;
	Dos.pszQueueProcParams=NULL;
	Dos.pszSpoolerParams=NULL;
	Dos.pszNetworkParams=NULL;
	HandlePrinter=DevOpenDC(program.hab(),OD_QUEUED,"*",9,
		(PDEVOPENDATA)&Dos,(HDC)0);
	if (HandlePrinter==DEV_ERROR)
	{   Valid.clear(); return;
	}
	Handle=GpiCreatePS(program.hab(),HandlePrinter,&S,
		PU_PELS|GPIA_ASSOC);
	if (DevEscape(HandlePrinter,DEVESC_STARTDOC,
		strlen(Myname),(char *)Myname,0,0)!=DEV_OK)
	{	Valid.clear(); return;
	}
	LONG alArray [CAPS_WIDTH_IN_CHARS];
	DevQueryCaps(HandlePrinter,CAPS_FAMILY,CAPS_WIDTH_IN_CHARS,
		(PLONG)alArray);
	S.cx=alArray[CAPS_WIDTH];
	S.cy=alArray[CAPS_HEIGHT];
	Open.set();
}

void PrinterPS::close ()
{   if (!Open) return;
	GpiDestroyPS(Handle);
	DevEscape(HandlePrinter,DEVESC_ENDDOC,0,0,0,0);
	DevCloseDC(HandlePrinter);
	Open.clear();
}

void PrinterPS::newpage ()
{	DevEscape(HandlePrinter,DEVESC_NEWFRAME,0,0,0,0);
}

// ************* Metafiles **********************

MetafilePS::MetafilePS (Window &window)
{   DEVOPENSTRUC dos;
	DEVOPENSTRUC dop;
	char *p;
	dop.pszLogAddress=(PSZ)NULL;
	dop.pszDriverName="DISPLAY";
	Hdc=DevOpenDC(program.hab(),
		OD_METAFILE,"*",2L,(PDEVOPENDATA)&dop,0);
	S.cx=window.width(); S.cy=window.height();
	Handle=GpiCreatePS(program.hab(),Hdc,&S,PU_PELS|GPIA_ASSOC);
}

MetafilePS::MetafilePS (Window &window, long w, long h)
{   DEVOPENSTRUC dos;
	DEVOPENSTRUC dop;
	char *p;
	dop.pszLogAddress=(PSZ)NULL;
	dop.pszDriverName="DISPLAY";
	Hdc=DevOpenDC(program.hab(),
		OD_METAFILE,"*",2L,(PDEVOPENDATA)&dop,0);
	S.cx=w; S.cy=h;
	Handle=GpiCreatePS(program.hab(),Hdc,&S,PU_PELS|GPIA_ASSOC);
}

MetafilePS::~MetafilePS ()
{	close();
}

void MetafilePS::close ()
{   if (Handle==NULLHANDLE) return;
	GpiAssociate(Handle,NULLHANDLE);
	GpiDestroyPS(Handle);
	Metafilehandle=DevCloseDC(Hdc);
	Handle=NULLHANDLE;
}

//***************** Bitmap Resources ***********************

Bitmap::Bitmap (int id, int width, int height)
{	PSZ pszData[4]={"Display",NULL,NULL,NULL};
	DeviceHandle=DevOpenDC(program.hab(),OD_MEMORY,"*",4,
		(PDEVOPENDATA)pszData,NULLHANDLE);
	S.cx=width; S.cy=height;
	PsHandle=GpiCreatePS(program.hab(),DeviceHandle,&S,
		PU_PELS|GPIA_ASSOC|GPIT_MICRO);
	Handle=GpiLoadBitmap(PsHandle,0,id,0,0);
}

Bitmap::~Bitmap ()
{   GpiDestroyPS(PsHandle);
	DevCloseDC(DeviceHandle);
	GpiDeleteBitmap(Handle);
}

//******************* Help ******************************

Help::Help (StandardWindow &window,
	int id, char *filename, char *title)
{	HELPINIT hini;
	hini.cb=sizeof(HELPINIT);
	hini.ulReturnCode=0L;
	hini.pszTutorialName=(PSZ)NULL;
	hini.phtHelpTable=
		(PHELPTABLE)MAKELONG(id,0xFFFF);
	hini.hmodHelpTableModule=(HMODULE)0;
	hini.hmodAccelActionBarModule=(HMODULE)0;
	hini.idAccelTable=0;
	hini.idActionBar=0;
	hini.pszHelpWindowTitle=title;
	hini.fShowPanelId = CMIC_HIDE_PANEL_ID;
	hini.pszHelpLibraryName=filename;
	Handle=WinCreateHelpInstance(program.hab(),&hini);
	if (!Handle) { Valid=0; return; }
	Valid=1;
	WinAssociateHelpInstance(Handle,window.framehandle());
}

// ****************** Dialogs *************************

static Dialog *activedlg,*dlgs=0;

static MRESULT EXPENTRY dialogproc (HWND hwnd, unsigned long msg,
	MPARAM mp1, MPARAM mp2)
{   Dialogitem *item;
	Dialog *dlg,*dlg1;
	int result;
	if (!activedlg) // search dialog using window handle
	{	dlg=dlgs;
		while (dlg)
		{	if (dlg->Handle==hwnd) break;
			dlg=dlg->Next;
		}
		if (!dlg) // should not happen
			return WinDefDlgProc(hwnd,msg,mp1,mp2);
	}
	else dlg=activedlg;
	switch (msg)
	{   case WM_INITDLG : // initialize activedlg
			activedlg->Handle=hwnd;
			item=activedlg->Items;
			while (item)
			{	item->init();
				item=item->next();
			}
			activedlg->start();
			// put dialog into dialog chain
			activedlg->Next=dlgs;
			dlgs=activedlg;
			activedlg=0; // activedlg unusable
			break;
		case WM_CHAR :
			if (dlg->key(SHORT1FROMMP(mp1),SHORT1FROMMP(mp2),
				SHORT2FROMMP(mp2))) break;
			goto def;
		case WM_HELP :
			if (dlg->H) dlg->H->display(dlg->Hid);
			break;
		case WM_CONTROL :
			result=dlg->Result=(SHORT1FROMMP(mp1));
			item=dlg->Items;
			while (item)
			{	if (item->id()==result)
				{	item->command(mp1,mp2);
					return 0;
				}
				item=item->next();
			}
			break;
		case WM_CLOSE :
			if (dlg->close())
			{	result=-1;
				goto finish;
			}
			return 0;
		case WM_COMMAND :
			result=dlg->Result=(SHORT1FROMMP(mp1));
			item=dlg->Items;
			while (item)
			{	if (item->id()==result)
				{	item->command(mp1,mp2);
					return 0;
				}
				item=item->next();
			}
			if (!dlg->handler(result))
			{   finish :
				item=dlg->Items;
				while (item)
				{	item->exit();
					item=item->next();
				}
				dlg->stop();
				// remove dialog from dialog chain
				if (dlgs==dlg) dlgs=dlg->Next;
				else
				{	dlg1=dlgs;
					while (dlg1)
					{	if (dlg1->Next==dlg) break;
						dlg1=dlg1->Next;
					}
					if (dlg1) dlg1->Next=dlg->Next;
				}
				// close dialog
				WinDismissDlg(hwnd,SHORT1FROMMP(mp1));
			}
			break;
		default :
			def: return WinDefDlgProc(hwnd,msg,mp1,mp2);
	}
	return 0;
}

void Dialog::init (Window &window, int id)
{	W=&window;
	Id=id;
	Items=0;
}

Dialog::Dialog (Window &window, int id, Help &h, int hid) :
	S("",256)
{	init(window,id);
	H=&h; Hid=hid; Next=0;
}

Dialog::Dialog (Window &window, int id) :
	S("",256)
{	init(window,id);
	H=0; Next=0;
}

Dialogitem *Dialog::entry (Dialogitem *item)
{	Dialogitem *note=Items;
	Items=item;
	return note;
}

void Dialog::carryout ()
{   activedlg=this;
	Handle=WinDlgBox(HWND_DESKTOP,W->handle(),dialogproc,
		(HMODULE)0,Id,NULL);
}

void DialogPanel::carryout ()
{   activedlg=this;
	Handle=WinLoadDlg(HWND_DESKTOP,W->handle(),dialogproc,
		(HMODULE)0,Id,NULL);
}

char *Dialog::gettext (int id, char *text, long size)
{   HWND handle=WinWindowFromID(Handle,id);
	WinQueryWindowText(handle,size,text);
	return text;
}

char *Dialog::gettext (int id)
{   HWND handle=WinWindowFromID(Handle,id);
	WinQueryWindowText(handle,S.size(),(PSZ)S.text());
	return S;
}

void Dialog::settext (int id, char *text)
{	HWND handle=WinWindowFromID(Handle,id);
	WinSetWindowText(handle,text);
}

MRESULT Dialog::message (int id, int msg,
	Parameter mp1, Parameter mp2)
{	HWND h;
	h=WinWindowFromID(Handle,id);
	return WinSendMsg(h,msg,mp1,mp2);
}

void Dialog::move (LONG x, LONG y)
{	WinSetWindowPos(Handle,NULLHANDLE,x,y,0,0,SWP_MOVE);
}

void Dialog::getframe (LONG &x, LONG &y, LONG &w, LONG &h)
{	SWP s;
	WinQueryWindowPos(Handle,&s);
	x=s.x; y=s.y; w=s.cx; h=s.cy;
}

//*********** Window Dialogs ********************

WindowDialog::WindowDialog (StandardWindow &w) : Dialog(w,0)
{   Handle=w.handle();
	w.setdialog(*this);
}

int WindowDialog::command (Parameter p1, Parameter p2)
{   Result=p1.lower();
	Dialogitem *item=Items;
	while (item)
	{	if (item->id()==Result)
		{	item->command(p1,p2);
			return 1;
		}
		item=item->next();
	}
	return 0;
}

//*************** Dialog Items ******************

Dialogitem::Dialogitem (int id, Dialog &dialog)
{	Id=id;
	D=&dialog;
	Next=dialog.entry(this);
}

void Dialogitem::size (long w, long h)
{   WinSetWindowPos(handle(),NULLHANDLE,0,0,
		w,h,SWP_SIZE);
}

void Dialogitem::pos (long x, long y)
{   WinSetWindowPos(handle(),NULLHANDLE,x,y,
		0,0,SWP_MOVE);
}

void Dialogitem::sizepos (long x, long y, long w, long h)
{   WinSetWindowPos(handle(),NULLHANDLE,x,y,
		w,h,SWP_SIZE|SWP_MOVE);
}

void Dialogitem::enable (int flag)
{	WinEnableWindow(handle(),flag);
}

ButtonItem::ButtonItem (WindowDialog &d,
	long x, long y, long w, long h, char *text,
	int style) :
		Dialogitem(d.newid(),d),Pressed(0)
{	WinCreateWindow(d.handle(),WC_BUTTON,
		text,WS_VISIBLE|BS_PUSHBUTTON|style,
		x,y,w,h,
		d.handle(),HWND_TOP,id(),
		0,0);
}

ButtonItem::ButtonItem (WindowDialog &d,
	long x, long y, long w, long h, int iconid,
	int style) :
		Dialogitem(d.newid(),d),Pressed(0)
{   char s[16];
	sprintf(s,"#%d",iconid);
	WinCreateWindow(d.handle(),WC_BUTTON,
		s,WS_VISIBLE|BS_PUSHBUTTON|style,
		x,y,w,h,
		d.handle(),HWND_TOP,id(),
		0,0);
}

StringItem::StringItem (WindowDialog &d,
		long x, long y, long w, long h,
		char *string, int length, int style)
		: Dialogitem(d.newid(),d),Length(length),S(string),
			Changed(0)
{	WinCreateWindow(d.handle(),WC_ENTRYFIELD,
		string,WS_VISIBLE|style,
		x,y,w,h,
		d.handle(),HWND_TOP,id(),
		0,0);
}

void StringItem::init ()
{   int size=strlen(S)+1;
	if (Length>size) size=Length;
	limit(size);
	D->settext(Id,S);
	D->message(Id,EM_SETREADONLY,Readonly);
	D->message(Id,EM_QUERYCHANGED);
	Changed.clear();
}

int StringItem::changed ()
{   if (Changed) return 1;
	if (D->message(Id,EM_QUERYCHANGED))
	{	S.copy(D->gettext(Id));
		Changed.set();
		return 1;
	}
	return 0;
}

void StringItem::exit ()
{   if (D->message(Id,EM_QUERYCHANGED))
	{	S.copy(D->gettext(Id));
	}
	Changed.clear();
}

void StringItem::limit (int length)
{	D->message(Id,EM_SETTEXTLIMIT,Parameter(length));
}

void CheckItem::init ()
{   D->message(Id,BM_SETCHECK,(int)F);
}

void CheckItem::exit ()
{	F=(long)D->message(Id,BM_QUERYCHECK,1);
}

void CheckItem::command (Parameter mp1, Parameter mp2)
{   if (mp1.higher()==BN_CLICKED || mp1.higher()==BN_DBLCLICKED)
	{	F=(long)D->message(Id,BM_QUERYCHECK);
		notify();
	}
}

void RadioButton::init ()
{   if (F) D->message(Id,BM_CLICK,1);
}

void RadioButton::command (Parameter mp1, Parameter mp2)
{	R->notify();
	R->sel(N);
}

RadioItem::RadioItem (int *ids, int n, Dialog &dialog, int sel)
	: N(n),Ids(ids),I(sel),D(&dialog)
{	int i;
	Rb=new RadioButton *[N];
	for (i=0; i<N; i++)
		Rb[i]=new RadioButton(Ids[i],dialog,i+1==I,*this,i+1);
}

RadioItem::~RadioItem ()
{	int i;
	for (i=0; i<N; i++) delete Rb[i];
	delete Rb;
}

void RadioItem::sel (int s)
{	I=s;
}

void DoubleItem::init ()
{	sprintf(S,"%-0.10g",X);
	D->settext(Id,S);
	D->message(Id,EM_SETREADONLY,Readonly);
}

void DoubleItem::exit ()
{ 	S.copy(D->gettext(Id),64);
	sscanf(S,"%lg",&X);
}

void LongItem::init ()
{	sprintf(S,"%-ld",N);
	D->settext(Id,S);
	D->message(Id,EM_SETREADONLY,Readonly);
}

void LongItem::exit ()
{ 	S.copy(D->gettext(Id),64);
	sscanf(S,"%ld",&N);
}

void SpinItem::init ()
{	D->message(Id,SPBM_SETLIMITS,Upper,Lower);
	D->message(Id,SPBM_SETCURRENTVALUE,N);
}

void SpinItem::exit ()
{	D->message(Id,SPBM_QUERYVALUE,&N,Parameter(0,SPBQ_ALWAYSUPDATE));
}

void SliderItem::init ()
{	D->message(Id,SLM_SETSLIDERINFO,
		Parameter(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),
		Parameter(N));
}

void SliderItem::exit ()
{	N=(long)D->message(Id,SLM_QUERYSLIDERINFO,
		Parameter(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE));
}

void SliderItem::tick (int n, int size)
{	D->message(Id,SLM_SETTICKSIZE,Parameter(n,size));
}

void SliderItem::label (int n, char *text)
{	D->message(Id,SLM_SETSCALETEXT,Parameter(n),Parameter(text));
}

void SliderItem::command (Parameter mp1, Parameter mp2)
{   switch (mp1.higher())
	{	case SLN_CHANGE : N=mp2; notify(); break;
		case SLN_SLIDERTRACK : N=mp2; changing(N); break;
	}
}

void ValuesetItem::select (int row, int col)
{	D->message(Id,VM_SELECTITEM,Parameter(row,col));
}

void ValuesetItem::setbitmap (int row, int col, Bitmap &b)
{	D->message(Id,VM_SETITEMATTR,Parameter(row,col),
		Parameter(VIA_BITMAP,1));
	D->message(Id,VM_SETITEM,Parameter(row,col),
		Parameter((long)b.handle()));
}

void ValuesetItem::setcolor (int row, int col, unsigned long color)
{	D->message(Id,VM_SETITEMATTR,Parameter(row,col),
		Parameter(VIA_RGB,1));
	WindowPS ps(*D->w());
	color=GpiQueryRGBColor(ps.handle(),0,color);
	D->message(Id,VM_SETITEM,Parameter(row,col),
		Parameter((long)color));
}

void ValuesetItem::exit ()
{	Parameter p(D->message(Id,VM_QUERYSELECTEDITEM,0));
	Col=p.higher(); Row=p.lower();
}

void ValuesetItem::command (Parameter mp1, Parameter mp2)
{	if (mp1.higher()==VN_SELECT)
	{	Row=mp2.lower(); Col=mp2.higher();
		notify();
	}
}

void ListItem::insert (char *text)
{	D->message(Id,LM_INSERTITEM,Parameter(LIT_END),Parameter(text));
}

void ListItem::select (int sel)
{	D->message(Id,LM_SELECTITEM,Parameter(sel),Parameter(1));
	Selection=sel;
}

void ListItem::exit ()
{	Selection=Parameter(D->message(Id,LM_QUERYSELECTION)).lower();
}

void ListItem::command (Parameter mp1, Parameter mp2)
{	if (mp1.higher()==LN_ENTER)
	{   Selection=Parameter(D->message(Id,LM_QUERYSELECTION)).lower();
		notify();
	}
}

MultilineItem::MultilineItem (WindowDialog &d,
	long x, long y, long w, long h,
	char *text, long length, int style)
		: Dialogitem(d.newid(),d),S(text,length),
			Changed(0)
{	WinCreateWindow(d.handle(),WC_MLE,
		text,WS_VISIBLE|style,
		x,y,w,h,
		d.handle(),HWND_TOP,id(),0,0);
}

void MultilineItem::init ()
{	limit(S.size());
	D->settext(Id,S);
	D->message(Id,MLM_SETSEL,0,0);
	D->message(Id,MLM_SETREADONLY,Readonly);
	D->message(Id,MLM_SETCHANGED,0);
	Changed.clear();
}

void MultilineItem::limit (int length)
{	D->message(Id,MLM_SETTEXTLIMIT,Parameter(length));
}

int MultilineItem::changed ()
{   if (Changed) return 1;
	if (D->message(Id,MLM_QUERYCHANGED))
	{   D->message(Id,MLM_SETCHANGED,0);
		S.copy(D->gettext(Id,S,S.size()));
		Changed.set();
		return 1;
	}
	return 0;
}

void MultilineItem::exit ()
{   if (D->message(Id,MLM_QUERYCHANGED))
	{	D->message(Id,MLM_SETCHANGED,0);
		S.copy(D->gettext(Id,S,S.size()));
	}
	Changed.clear();
}

//******************* File Selector ********************

FileSelector::FileSelector (Window &window,
		char *filter, int saving,
		char *title, char *ok) : Filter(filter),Title(title)
{	memset(&Fd,0,sizeof(FILEDLG));
	Fd.cbSize=sizeof(FILEDLG);
	Fd.fl=FDS_CENTER|FDS_ENABLEFILELB
		|(saving?FDS_SAVEAS_DIALOG:FDS_OPEN_DIALOG);
	Fd.pszTitle=(char *)Title;
	if (!ok)
	{	if (saving) Ok.copy("Save");
		else Ok.copy("Load");
	}
	else Ok.copy(ok);
	Fd.pszOKButton=(char *)Ok;
	strcpy(Fd.szFullFile,"");
	W=&window;
}

char *FileSelector::select ()
{   char *p=Filter.filename();
	String s(Fd.szFullFile,CCHMAXPATH);
	s.stripfilename();
	strncpy(Fd.szFullFile,s,CCHMAXPATH-16);
	Fd.szFullFile[CCHMAXPATH-16]=0;
	strcat(Fd.szFullFile,p);
	Freturn=WinFileDlg(HWND_DESKTOP,W->handle(),&Fd);
	if (!Freturn || Fd.lReturn!=DID_OK) return 0;
	return Fd.szFullFile;
}

char *FileSelector::select (char *name)
{   strncpy(Fd.szFullFile,name,CCHMAXPATH-1);
	Fd.szFullFile[CCHMAXPATH-1]=0;
	Freturn=WinFileDlg(HWND_DESKTOP,W->handle(),&Fd);
	if (!Freturn || Fd.lReturn!=DID_OK) return 0;
	return Fd.szFullFile;
}

// ************ Font Selector *********

int FontSelector::select (Window &window)
{   WindowPS *sps=0;
	PrinterPS *pps=0;
	Queues *q=0;
	if (Type&screen) sps=new WindowPS(window);	if (Type&printer)
	{   q=new Queues();
		pps=new PrinterPS(*q);
	}
	memset(&Fd,0,sizeof(FONTDLG));
	Fd.cbSize=sizeof(FONTDLG);
	if (sps) Fd.hpsScreen=sps->handle();
	if (pps) Fd.hpsPrinter=pps->handle();
	Fd.pszFamilyname=Facename;
	Fd.usFamilyBufLen=Facename.size();
	Fd.fxPointSize=long(Pointsize*65536);
	Fd.fl=FNTS_CENTER;
	Fd.clrFore=CLR_BLACK;
	Fd.clrBack=CLR_WHITE;
	Fd.fAttrs.usCodePage=Codepage;
	Result=0;
	WinFontDlg(HWND_DESKTOP,window.handle(),&Fd);
	Result=(Fd.lReturn==DID_OK);
	if (sps) delete sps;
	if (pps) { delete pps; delete q; }
	return Result;
}

// *************** handling fonts **************************

Fonts::Fonts (PS &ps)
{	Count=0;
	Count=GpiQueryFonts(ps.handle(),
		QF_PUBLIC,0,&Count,sizeof(FONTMETRICS),0);
	AllFonts=new FONTMETRICS[Count];
	GpiQueryFonts(ps.handle(),
		QF_PUBLIC,0,&Count,sizeof(FONTMETRICS),AllFonts);
}

Fonts::~Fonts ()
{	delete AllFonts;
}

Font::Font (FontSelector &fs)
{	memcpy(&Fat,fs.fat(),sizeof(FATTRS));
}

Font::Font (char *name, long h, long w, long a, long t, int cp)
{	Fat.usRecordLength=sizeof(FATTRS);
	Fat.fsSelection=0;
	Fat.lMatch=0;
	strncpy(Fat.szFacename,name,FACESIZE);
	Fat.idRegistry=0;
	Fat.usCodePage=cp;
	Fat.lMaxBaselineExt=h;
	Fat.lAveCharWidth=w;
	Fat.fsType=t;
	Fat.fsFontUse=a;
}

Font::Font (long id, char *name, long fontparams)
{	Fat.usRecordLength=sizeof(FATTRS);
	Fat.fsSelection=fontparams;
	Fat.lMatch=id;
	strncpy(Fat.szFacename,name,FACESIZE);
	Fat.idRegistry=0;
	Fat.usCodePage=0;
	Fat.lMaxBaselineExt=0;
	Fat.lAveCharWidth=0;
	Fat.fsType=0;
	Fat.fsFontUse=0;
}

// ************ Profile ***************

void Profile::writestring (char *k, char *i)
{	PrfWriteProfileString(P,S,k,i);
}

void Profile::writedouble (char *k, double x)
{	PrfWriteProfileData(P,S,k,&x,sizeof(double));
}

void Profile::writelong (char *k, long x)
{	PrfWriteProfileData(P,S,k,&x,sizeof(long));
}

void Profile::writeint (char *k, int x)
{	PrfWriteProfileData(P,S,k,&x,sizeof(int));
}

char *Profile::readstring (char *k, char *d, long size)
{	H.copy(d,size);
	PrfQueryProfileString(P,S,k,(char *)H,(char *)H,size);
	return H;
}

double Profile::readdouble (char *k, double x)
{   double res;
	unsigned long size=sizeof(double);
	if (PrfQueryProfileData(P,S,k,&res,&size)) return res;
	else return x;
}

long Profile::readlong (char *k, long x)
{   long res;
	unsigned long size=sizeof(long);
	if (PrfQueryProfileData(P,S,k,&res,&size)) return res;
	else return x;
}

int Profile::readint (char *k, int x)
{   int res;
	unsigned long size=sizeof(int);
	if (PrfQueryProfileData(P,S,k,&res,&size)) return res;
	else return x;
}

// ********** Clipboard ***************

void Clipboard::copy (MetafilePS &meta)
{   meta.close();
	if (!WinOpenClipbrd(program.hab())) return;
	WinEmptyClipbrd(program.hab());
	WinSetClipbrdData(program.hab(),
		meta.metafilehandle(),CF_METAFILE,CFI_HANDLE);
	WinCloseClipbrd(program.hab());
}

