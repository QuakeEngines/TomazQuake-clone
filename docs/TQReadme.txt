TomazQuake 1.481

8 June 2010

Author: 	Tomas "Tomaz" Jakobsson and Bram "BramBo" Stein and Victor "Vic" Luchits
Email: 		tompsson@hotmail.com
Webpage: 	http://tomaz.snowcold.net

Instructions on how to use this engine in your own modifications can be found in Moding.txt and Mapping.txt

IMPORTANT NOTE

TQScript no longer use id1/textures.rs to read the script... it now uses id1/scripts/textures.txt

////////////////
//NEW in 1.481//
////////////////

...Made it run on a modern computer / modern graphics card

///////////////
//NEW in 1.48//
///////////////

...Removed FMOD
...Added "maps" command that lists all maps... also added that list to the multiplayer menu
...fixed a MAJOR memory bug that might have caused a big slowdown on most computers
...use -vid_gamma 0-1 when u start TQ and use the brightness control to get the brightness u want... the 2 commands changes the brightness differently... so its up to u to experiment until u get the brightness u want...
...fixed so it now can load gfx/wad/conchars.tga or pcx as the font texture
...all sound cvars begins with s_ (i think)... this is due to the removeal of FMOD
...the "puffing" fire is fixed in TomazMod

///////////////
//NEW in 1.47//
///////////////

Most of this is done by Victor "Vic" Luchits apart from a few misc fixes by me...

light lerping on mdl- and md2- models
fullbright skins on mdl's
fixed memory leaks in scripts
sped up drawing of flares on models
sped up drawing of strings
glu32 library is not used anymore
new method of dynamic lighting (faster)
fixed MOVETYPE_PUSH error
improved md2 shadows
a bit faster demo recording\reading
fixed showlmp bug which didn't not allow to view pics on high-values coordinates
misc fixes and improvements

//////////////////
//How to install//
//////////////////

Just unzip to your quake dir.

If the engine wont run or if something doesnt do what it should... then try to run the engine WITHOUT using my mod... If that doesnt help then let me know and ill fix it.

///////////////
//NEW in 1.46//
///////////////
saving of screenresolution 	- see below for more info
mirrors 			- see below
added brightness, now in video menu
mapshots are now loaded fullscreen (looks way better)
centerprints will now also be printed to the console (on request)
max/min viewangles is now a cvar (see "new console variables")
gl_wireframe and gl_wireonly options, see new console variables
new fire effect
fixed fullbrights
toggleable underwater and slowmo sound pitch
total redone particle renderer						(thanks to illusion)
new envmap code that uses no extensions (should work on any 3D card)	(thanks to Vic)

/////////////////////////
//NEW console variables//
/////////////////////////

vid_savecurrentmode	- saves the current videomode to startup.txt, 	will be read next time quake starts
cl_viewangles_up	- the upward max angle a player can look *
cl_viewangles_down	- the downward max angle a player can look *
gl_showpolys		- shows the polygons used in bsp models
gl_wireframe		- wireframe bsp model (make sure gl_clear is set to 1)
print_center_to_console - enables centerprint messages to console
s_nowatereffect	 	- with this enabled the sound wont slow down while underwater
s_noslowmoeffect	- with this enabled the sound wont slow down while in slowmo mode
most sound cvars can be viewed by typing SND_ (nig letters) and pressing tab

NOTE: 
*for the sake of gameplay (and your own health, don't set viewangles above 90)

////////
//BUGS//
////////

skybox isn't properly reflected in mirror
ambient sounds is fux0red
timerefresh not correctly working near mirrors

//////////////
//BUGS FIXED//
//////////////

particles now draw correctly in front of water, and also reflect properly in
mirrors
mirrors work (see mirrors below)
sbar drawing on map change removed (not really a bug)
crosshairs
tqscript animation, envmapping
sound samplerate is now saved and used when u restart a game
typing a console command and pressing tab sometimes crashed the engine... not now anymore
models dissapear on edges is fixed
brightness slider should work now

###########
# mirrors #
###########
Mirrors work in all video modes - support for the old & mirror texture name trick.

###########################
# screenresolution saving #
###########################
i got tired of using batch files to start glquake with -width 800 -bpp 32, so i hacked
this together. Start quake with the screenresolution you want, go to the console and 
type "vid_savecurrentmode" followed by enter. Quake will then generate a file called
startup.txt in your gamedir/scripts/ directory. Next time you start quake (without extra
command line parameters) quake will start at the same resolution/bpp as when you typed
"vid_savecurrentmode". Of course, using the -width, -height and -bpp parameters will
override these (now) default resolution. Try not to edit the startup.txt file, if it get's
corrupted, remove it and make a new one by calling the "vid_savecurrentmode" parameter.
(also expect a tutorial to be released on this soon)

////////////////
//Improvements//
////////////////

Code with tutorials:

Md2 Support			LordHavoc
Lit support			LordHavoc
Motion & Anim Interpolation	Phoenix
QuakeC File Access		FrikaC
Better ChaseCam			FrikaC
Copy & Paste			FrikaC
QC Exec				FrikaC
Toggelable Mouselook		Heffo
Fmod Sound Base			Heffo
Enhanced Console Editing	Radix
Bobbing Items			Muff
Centered Scorboard		Muff
FPS Counter			Muff
New Shadow Code			Muff
Enhanced Tab Complettion	Fett
Scroll Enhance			Fett
Rotating BModels		MrG
Any Pak Files			MrG
Alt-Tab Fix			Tim



Code from others sources and heavily modified by me:

RailTrail			TerribleOne
Bouncing Particles		LordHavoc
QC Alpha Scale Glow On Models	LordHavoc
TGA On Almost Everything	LordHavoc
Hl BSP Support			LordHavoc
Pr_checkextension		LordHavoc
Trans Object Sorter		FrikaC
Frame Exporter			Heffo
ARB MultiTexturing		Heffo
Underwater Caustics		Heffo		(very heavy on fps)
Colored Characters		Topaz		(not fully working)
Map Snapshots			Muff
Glowing Models			Muff
TGA Crosshairs			Muff
Texture Scripts			MrG

Totally own code:

Md2's Working as Players
Translucent Hud
Mp3 Player
Fullbright Map Textures
Improved Aiming
Q2 Skyboxes
Removed Waterwarp
Extended Option Menu
Higher Packet Overflow Limit
More Trans Menu Background
New Particle System
Particle rain and snow
No More id1/glquake
Bigger Model Support
Water Wave
Slowmo Cvar
Fog
Show BSP Triangles
Draw World as Wireframe

//////////////
//How To Use//
//////////////

####################
Md2 Support:

Read Moding.txt
####################
Lit support:

Read Mapping.txt
####################
Motion & Anim Interpolation:

Makes Animations and Movements of all models ALOT smoother. Can be turned on/off in the "Extra Options" menu.
####################
Better ChaseCam:

Doesnt poke into walls as much as the original one did.
####################
QuakeC File Access:

Additional QuakeC stuff to read and write to files from QuakeC.
####################
Copy & Paste:

Now u can Copy from windows and Paste into the console... Handy for IP's ;)
####################
QC Exec:
####################
Toggelable Mouselook:

The original +mlook is removed and this was added instead. Can be turned on/off in the "Extra Options" menu.
####################
Enhanced Console Editing:
####################
Bobbing Items:

Items not only rotates but bounces to. Can be turned on/off in the "Extra Options" menu.
####################
Centered Scorboard:

The scoreboard that pops up when u complete a map no longer is in the up-left corner.
####################
FPS Counter:

Shows your current "Frames Per Second". Can be turned on/off in the "Extra Options" menu.
####################
New Shadow Code:

Shadows on rocket no longer flies all around the room.
####################
Enhanced Tab Complettion:
####################
Scroll Enhance:

Console doesnt jump back to bottom if something gets written to it if u have scrolled up to read.
####################
Rotating BModels:

U can now make rotating Brushes... More info when i know myself how to do em... ;)
####################
Any Pak Files:

Pak files no longer need to be named pakX.pak. But can now be anything.
####################
Alt-Tab Fix:

Alt-Tabbing out and in again on nVidia cards now should work. (I only have a Voodoo2 so it has not been tested).
####################
RailGun:
####################
QC Alpha Scale Glow On Models:

Read Moding.txt
####################
TGA On Almost Everything:

Read Mapping and Moding.txt
####################
Hl BSP Support:

U can now run Half-Life maps. But since alot of em uses stuff that Quake1 doesnt have (like func_ladders) they're not really playable unless u have a good mod that has support for all thos things. Some maps have their textures builtin into the map. Those maps will probably crash the engine. Maps can be put in maps/cs and maps/hl now to make the maps folder cleaner. And the WAD's that are used by the maps goes into the textures folder. Consult the "TEsted HL Maps.txt" to see what maps that work... This is still in beta stages.
####################
Pr_checkextension:

Read Pr_checkextension.txt.
####################
Trans Object Sorter:

Translucent objects are now sorted correctly so other models and stuff actually are seen thro the translucent objects.
####################
ARB MultiTexturing:

MultiTexturing thats faster on nVidia cards.
####################
Underwater Caustics:

A weird underwater effect. The images goes into id1/textures/caustics
####################
Colored Characters:

Not Working.
####################
Map Snapshots:

Loads an image of the map thats loading.
####################
Glowing Models:

The glow around the flames on the walls.
####################
TGA Crosshairs:

You can now change between 10 different crosshairs. Change is made in the "Extra Options" menu.
####################
Texture Scripts:

Read TQScripts.txt
####################
Translucent Hud

Now the status bar translucency can be set to a desire value. Change is made in the "Extra Options" menu.
####################
Mp3 Player

instead of audio tracks 02-11 it uses Track02-11.mp3 when loading music for maps. mp3's goes into id1/music.
(mp3 play "scooter - fire") in console would load that mp3 if it is in the music folder. (mp3 stop), (mp3 pause), (mp3 resume) and (mp3 restart) is also supported.
####################
Fullbright Map Textures:

Those fullbright colors on software that went away in GL are now back. Can be turned on/off in the "Extra Options" menu.
####################
Improved Aiming:

A more accurate aiming.  Can be turned on/off in the "Extra Options" menu. Has to be off on a lan game if someone doesnt use TQ.
####################
Q2 Skyboxes:

Read Mapping.txt
####################
Removed Waterwarp:

That b0rked waterwarp is now totally gone.
####################
Extended Option Menu:

My "Extra Options" menu can be found at the bottom of the original options menu.
####################
Higher Packet Overflow Limit:

Mods that tend to cause a "packet overflow" message should now work better.
####################
More Trans Menu Background:

Just the menu being more translucent.
####################
New Particle System:

Im sure everyone knows what this is ;)
####################
Particle rain and snow:

Read Mapping.txt
####################
No More id1/glquake:

Doesnt put out those ms2 files that used to be in the id1/glquake folder.
####################
Bigger Model Support:

Now supports bigger models and skins.
####################
Water Wave:

Makes waves on the water surface. Can be turned on/off in the "Extra Options" menu.
####################
Slowmo Cvar:

Slow down or speed up the game. use slowmo in the console. 1 is normal speed.
####################
Fog:

Can be turned on/off in the "Extra Options" menu. Density and color is also changed there.
####################
Draw WireFrame Over The World:

use gl_wireframe 1 to activate this mode. It draws a white line around all the world polygons.
####################
Draw World as Wireframe Only:

use gl_wireonly 1 to activete this mode. It works pretty much as the gl_showpolys except that it doesnt draw anything but the polygon lines. So it can be used both to view the splitting of the BSP polygons and to check how bad or good the vising of the map is.
####################

Additional credit goes to.

Akuma			Moral Support and Beta Testing
scar3crow		Moral Support and Beta Testing
Krust			Moral Support and Beta Testing
Horn			Moral Support and Beta Testing
CocoT			Moral Support and Beta Testing
Harb			Moral Support and Beta Testing
MrPeanut		Moral Support and Beta Testing
CtM			Moral Support and Beta Testing
The Lieutenant		Moral Support and Beta Testing
Plumb			Moral Support and Beta Testing
Midgar			Moral Support and Beta Testing
Deetee			Moral Support and Beta Testing
Paddy			Moral Support and Beta Testing
KyleMac			Moral Support and Beta Testing
jAGO			Moral Support and Beta Testing
ArchAngel		Coding Help
P5YCHO			Coding Help
BramBo			Coding Help
KrimZon			Coding Help
Ender			Coding Help
Atomizer		Coding Help
dakilla			Coding Help
Illusion		Coding Help

And of course i also wanna thank all you people out there that use this engine. Without you this wouldnt exist.