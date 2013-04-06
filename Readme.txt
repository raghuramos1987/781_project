COMPILATION AND RUNNING

$ make clean
$ make
$ ./main <ply_file>

FEATURES

If the ply file does not have normals then the program generates the 
normals.

It can read ply files containing models with triangles or quadrilaterals.
(All quadrilaterals need to have same number of sides).

Tested with:
Bunny, Dragon, Blade (from ply site).

KEYBOARD USAGE:

'w': enables/disables wire frame mode
't': enables/disables trackball (in global/local mode)
'g': enables/disables global mode
'r': Resets all views

MOUSE USAGE:

Have modified a few control sequences to accomodate extra credit features.

Left click and drag (irrespective of whether trackball is enabled or not) 
rotates either model or world (according to whether global mode is enabled
or not).

Ctrl+right click scales down (model/world).

Shift+right click scales up (model/world).

You can control the amount of scaling per click by changing the line 

#define SCALE_FACT

in opengl_hook.cpp

EXTRA CREDIT:

Right click and drag positions model at new point. The model can be 
rotated at any point where it is positioned.

The model might appear to be blocked from rotation if the mouse 
is dragged in the same direction for a long time. This is because the 
mouse co-ordinates move out of the view volume. 

GENERAL OBSERVATION:

It is easier to do the global rotation if you scale down the global 
size.
