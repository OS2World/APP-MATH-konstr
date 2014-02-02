.********************************************************
:userdoc.
:docprof toc=123.
:title.Compasses and Ruler for OS/2
.******************************************
:h1 res=100. Overview
:p.
C.a.R. is able to do constructions with compasses and ruler on a
computer. Clearly, advantages are that one is able to
:ul.
:li. move construction points and observe the result,
:li. generate tracks of points,
:li. hide or display construction details,
:li. use macros to simplify constructions.
:eul.:p.
The program can be started by dragging a text file containing a
construction on its program object. Refer to the OS/2 handbook
on how to create a program object for CAR.
:p.
This program uses OS/2 and its help facilities. The interface consists
of a construction window and the toolbox. For further information read
about
:ul compact.
:li. :link reftype=hd res=200.The construction window.:elink.
:li. :link reftype=hd res=300.The toolbox.:elink.
:li. :link reftype=hd res=400.The menus.:elink.
:li. :link reftype=hd res=450.Using Macros.:elink.
:li. :link reftype=hd res=500.The keyboard.:elink.
:li. :link reftype=hd res=1000.The author.:elink.
:eul.
.******************************************************
:h1 res=200. The construction window.
:i1. Construction Window
:i1. Mouse Buttons
:p.
This window displays your construction. Hidden objects can be made
visible by the corresponding menu option. The window can be scrolled.
You can zoom in and out.
:p.
After choosing a tool, the left mouse button is used to choose a point
inside the construction window. The right mouse button can be used to
edit an object or to move it.
:p.
Tools from the toolbox can also be chosen with the keyboard.
(:link reftype=hd res=500.Using the keyboard:elink.)
.******************************************************
:h1 res=300. The Toolbox.
:i1. Tool Box
:p.
This window shows the available tools as icons. You can choose a tool
with the mouse or the keyboard (left and right arrow key and return).
:p.
A text explains the action, which is expteced from the user.
This text may change during the work with the tool.
:p.
Here is a description of the tools.
:dl compact.
:dt.:artwork name='carbmp\point.bmp'.
:dd.Generates a point.
:dt.:artwork name='carbmp\line.bmp'.
:dd.Generates a straight line through two points.
:dt.:artwork name='carbmp\ray.bmp'.
:dd.Generates a ray.
:dt.:artwork name='carbmp\segment.bmp'.
:dd.Generates a line segment.
:dt.:artwork name='carbmp\circle.bmp'.
:dd.Generates a circle from center and radius point.
:dt.:artwork name='carbmp\move.bmp'.
:dd.Moves a point. This point may not be constructed from other
objects (e.g., by a cut). This can also be done by draging an
object with the right mouse button.
:dt.:artwork name='carbmp\moveon.bmp'.
:dd.Moves a point on a line, ray, segment or circle. This can be used
to study what happens to another point, if a given point is moved
along an object. Track the other point first. The line or circle
must not depend on the point.
:dt.:artwork name='carbmp\pointon.bmp'.
:dd.Generates an arbitrary point on an object.
:dt.:artwork name='carbmp\intersect.bmp'.
:dd.Intersects two objects. This generates one or two points.
Intersections may become invalid. Then all objects depending on it are
also invalid.
:dt.:artwork name='carbmp\track.bmp'.
:dd.Track points. Choose the point and then move another point. The
tracks may be deleted with the button on the right side of the tool
box.
:dt.:artwork name='carbmp\hide.bmp'.
:dd.Hide or unhide an object. Hidden objects can be made visible by
a menu option.
:dt.:artwork name='carbmp\delete.bmp'.
:dd.Delete an object and all objects depending on it.
:dt.:artwork name='carbmp\edit.bmp'.
:dd.Edit an object. This can also be chosen by clicking an object
with the right mouse button.
:dt.:artwork name='carbmp\angle.bmp'.
:dd.Constructs an angle from three points. The angle is always
oriented in the mathematical positive sense. You cannot use
angle to construct anything.
:edl.:p.
:hp2.Note &colon.:ehp2. The following tools can be replaced by pure
construction steps. They just help to solve a bigger construction
problem more rapidly.
:dl compact.
:dt.:artwork name='carbmp\lot.bmp'.
:dd.Generate a plumb line from a point to a line object.
:dt.:artwork name='carbmp\parallel.bmp'.
:dd.Generate a parallel line through a point to a line object.
:dt.:artwork name='carbmp\circle3.bmp'.
:dd.Generate a circle from center, and two points, whose distance
gives the radius.
:dt.:artwork name='carbmp\middle.bmp'.
:dd.Generates the middle point between two points.
:edl.:p.
Additionally there are buttons in this dialog.
:dl compact.
:dt.:artwork name='carbmp\deltrack.bmp'.
:dd.Delete all tracks of points.
:dt.:artwork name='carbmp\delhide.bmp'.
:dd.Toggle show of all hidden objects.
:dt.:artwork name='carbmp\colors.bmp'.
:dd.Set default color.
:dt.:artwork name='carbmp\undo.bmp'.
:dd.Delete the last construction step.
:edl.
.******************************************************
:h1 res=400. The menus.
:i1. Menus
:p.
All menu points should be self explaining (I hope). Some can be chosen
with keyboard shortcuts as displayed besides the menu text.
.******************************************************
:h1 res=450. Macros
:i1. Macros
:p.
Macros collect several construction steps into a single macro step.
Each macro stores its name, the types of parameters it needs and the
construction details. Once the user calls the macro, he will be
prompted for the parameters. If he has finishes choosing all the 
parameters the macro will go through its construction details.
:p.
Writing a macro is easy. First, the parameters must be chosen from the
construction window. Then the target objects must be chosen. The 
target objects will determine, which construction details are stored 
in the macro definition. Finally, the macro definition can be 
finished by giving it a name.
:p.
Macros may be saved and loaded using *.mac files. The name of this
file may be different from the macro name.
:p.
Note that colors, hide flags and other details are stored in a macro
definition. You may want to hide intermediate steps of the macro
before you define it.
.******************************************************
:h1 res=500. The Keyboard.
:i1. Keys
:p.
The tools can be chosen with the keyboard. Press letters "qwertyuiop"
or "asdfghj". Or scroll through the tools with cursor right and left.
:dl compact.
:dthd.:hp2.Key:ehp2.
:ddhd.:hp2.Purpose:ehp2.
:dt.Esc
:dd.Reset the active tool.
:dt.F1
:dd.Help (also in dialog windows)
:dt.F2
:dd.Index for help.
:dt.F3
:dd.Help for keys.
:dt.F4
:dd.Content of help.
:dt.F5
:dd.Zoom in.
:dt.F6
:dd.Zoom out.
:dt.F7
:dd.Run a macro.
:dt.F8
:dd.Choose default object color.
:dt.F9
:dd.Edit comment of construction.
:dt.F10
:dd.Toggle showing all objects.
:dt.<--
:dd.(Backspace key) Delete the lastly constructed object.
:dt.DEL
:dd.(Delete key) Delete all tracks of points.
:dt.Shift-F7
:dd.Center screen to (0,0).
:dt.Shift-F10
:dd.Select object colors, which should be visible.
:edl.
.******************************************************
:h1 res=600. Die Object Choice
:p.
This dialog appears, if the choice with the mouse was not unique.
Choose the object with a double click or click OK, after your
choice is highlighted.
.******************************************************
:h1 res=1000. About the Author.
:i1. Autor
:artwork align=left name='rene.bmp'.
:p.
This program was written using a self made C++ library. You use it
at your own risk. Questions and comments are wellcome.
:xmp.
Dr. R. Grothmann
Ahornweg 5a
85117 Eitensheim

EMail&colon.
grothm@ku-eichstaett.de
:exmp.:p.
.****
:h1 res=2000 hide. Object choice
:p.
Double click on the object you want, or select and press OK.
.****
:h1 res=2010 hide. Edit object
:p.
Not all fields in this dialog are changable. A point can only be
moved, if it is not a constructed point.
.****
:h1 res=2020 hide. Error
:p.
This should only happen, if you change the construction file manually.
.****
:h1 res=2030 hide. Default Color
:p.
Choose the default color for further constructions.
.****
:h1 res=2040 hide. Comment
:p.
You may want to comment your construction.
.****
:h1 res=2050 hide. Choose Objects for Display
:p.
Using this dialog, you may completely hide groups of objects with the same
color.
.****
:h1 res=2060 hide. Choose Makros
:p.
This dialog lets you choose one of the defined makros.
.****
:h1 res=2070 hide. Parameter Prompt
:p.
This prompt will appear, whenever the user is asked to choose this
macro parameter.
.****
:h1 res=2080 hide. Makro Name
:p.
Determine a unique name for the parameter. If there is already a macro
with this name, you will be asked to delete this macro.
.****
:h1 res=2090 hide. Save Bitmap
:p.
You can specify the size of the saved bitmap. If you keep the aspect ratio,
you need only change one of the coordinates.
.****
:h1 res=2100 hide. Grid Settings
:p.
You can specify the distance of the grid lines for the fine and the normal
grid. Moreover, you can specify, if the axes should be hilighted.
.******************************************************
:euserdoc.
