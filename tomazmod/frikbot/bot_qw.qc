
/*
======================================
FrikBot version 0.09 for QW REVISION B
======================================

FrikBot, and all functions and code writen by Ryan "Frika C" Smith are
copyrighted (C) 1999-2000. You are allowed to use and distribute this mod as
long as this comment remains intact and the mod is provided free of charge. 
You *are* allowed to modify and incorporate this bot into your own mods, as
long as adequate credit is given and the end result of your modifications is
also provided free of charge. If you have questions or comments regarding the
bot, the code, or just the universe in general, please write me at
frika-c@earthling.net. Have a nice day!

======================================
These installation instructions only apply to QuakeWorld (as does this entire
file). For Normal Quake, please refer to bot.qc

--------------------------------------
To install on a new mod, do all this:
--------------------------------------
Place all included bot*.qc files in the subdirectory "frikbot"
in your source folder, then...

* Add the following lines to progs.src right after the defs.qc line
frikbot/bot_qw.qc
frikbot/bot_way.qc
frikbot/bot_ai.qc
frikbot/bot_misc.qc
frikbot/bot_phys.qc
frikbot/bot_move.qc
--------------------------------------
* Comment out the following functions in defs.qc
sound, stuffcmd, sprint, aim, centerprint, setspawnparms
WriteByte, WriteChar, WriteShort, WriteLong, WriteCoord
WriteAngle, WriteString, WriteEntity
--------------------------------------
* Add this to worldspawn() in world.qc, right at the very top, before InitBodyQue();
BotInit();  // FrikBot
--------------------------------------
* add this line to StartFrame() in world.qc, at the very top
BotFrame(); // FrikBot
--------------------------------------
* Add these two lines to PlayerPreThink in client.qc at the very top
if (BotPreFrame()) // FrikBot
	return;
--------------------------------------
* Add this line to PlayerPostThink in client.qc at the very top
if (BotPostFrame()) // FrikBot
	return;
--------------------------------------
* Add the following line to the very top of Client Connect in client.qc
ClientInRankings(); // FrikBot
--------------------------------------
* Add these lines to the very top of ClientDisconnect in client.qc
if (self.ishuman)
	clientSetFree( self.fClientNo ); // FrikBot
--------------------------------------
* Add these lines to the very top of SpectatorConnect in spectate.qc
ClientFixRankings(); // FrikBot
--------------------------------------
*/
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Stuff you can customize.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

float BOT_IMPULSE = 100; 
// first impulse used for the bots, the other impulses will be consecutive 
//+1, +2 from it. If the mod you are incorportaing into uses 100 - 104,
// change this to whatever suits you 

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

And other stuff you might not want to touch...

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
// ----- entity fields ---
.float wallhug, keys, ishuman, tfrags, fClientNo, fShirt, fPants, fEnterTime, fUserID; 
.float chat_subj, chat_time, priority, ai_time, f_sound, missile_speed;
.float portal_time, botskill, switch_wallhug, failure_time, obs_time;
.entity temp_way, last_way;
.entity phys_obj;
.vector b_angle, ldest1, mouse_emu, obs_dir;
.entity target1, target2, target3, target4;
.vector movevect; // movevect: X = forward/back, Y = left/right, Z = up/down
.vector punchangle; // HACK - Don't want to screw with bot_phys

// --------defines-----
float MSG_UPDATENAME   = 13; // message id for update name
float MSG_UPDATEFRAGS  = 14; // message id for update frags
float MSG_UPDATECOLORS = 17; // message id for update colors

// used for the physics & movement AI
float KEY_MOVEUP 		= 1;
float KEY_MOVEDOWN 	= 2;
float KEY_MOVELEFT 	= 4;
float KEY_MOVERIGHT 	= 8;
float KEY_MOVEFORWARD	= 16;
float KEY_MOVEBACK	= 32;
float KEY_LOOKUP		= 64;
float KEY_LOOKDOWN	= 128;
float KEY_LOOKLEFT	= 256;
float KEY_LOOKRIGHT	= 512;

// -------globals-----
float fActiveClients1;
float fActiveClients2;
float fMaxClients;
float BOT_COUNT;
float WAYPOINT_MODE; 
float WAYPOINTS;
float b_conversation; // used by over-done chat code
entity b_originator, b_target; // ditto
float sv_friction, sv_gravity, sv_accelerate, sv_maxspeed, sv_stopspeed;
entity fixer;
float userid; // give each player/bot a unique uid
float coop; // needed for bot_ai.qc
// -------ProtoTypes------
// external
void() ClientConnect;
void() ClientDisconnect;
float(float v) anglemod;
void() SetNewParms;
float (entity targ) visible;
// rankings
float(float fClientNo) clientBitFlag;
float(float fClientNo) clientIsActive;
void(float fClientNo) clientSetUsed;
void(float fClientNo) clientSetFree;
float() clientNextAvailable;
void(float whichteam) botConnect;
void(entity bot) botDisconnect;
void(float clientno) botInvalidClientNo;
void(float clientno, float clientfrags) svcUpdateFrags;
void(float clientno, float clientping) svcUpdatePing;
void(float clientno, float cliententertime) svcUpdateEnterTime;
void(float clientno, float clientid, float clientshirt, float clientpants, string clientname) svcUpdateUserInfo;
// waypointing
void() DynamicWaypoint;
entity(vector org) make_waypoint;
void() ClearAllWays, FixNumbers, FixWaypoints;
// physics & movement
float(vector weird) frik_walkmove;
void() frik_movetogoal;
float(float flag) frik_recognize_plat;
void(vector whichway, float danger) frik_obstructed;
void() CL_KeyMove, SV_Physics_Client, SV_ClientThink;
// ai & misc
string () PickARandomName;
void(entity t, float y1) BotStartTopic;
float (entity targ) fov;
float (float y1, float y2) angcomp;
float (entity targ) wisible;
float (entity targ) fisible;
vector(entity ent) realorigin;
void(entity ent) target_drop;
void(entity ent) target_add;
void() BotChat;
void() KickABot;
void() botImpulses;


// ----------Commands---------
void(entity e, float chan, string samp, float vol, float atten) frik_sound = #8;
void(entity client, string s) frik_stuffcmd = #21;
void(entity client, float level, string s) frik_sprint = #24;
vector(entity e, float sped) frik_aim = #44;	
void(entity client, string s) frik_centerprint = #73;
void(entity e) frik_setspawnparms = #78;
void(float to, float f) frik_WriteByte = #52;
void(float to, float f) frik_WriteChar = #53;
void(float to, float f) frik_WriteShort = #54;
void(float to, float f) frik_WriteLong = #55;
void(float to, float f) frik_WriteCoord = #56;
void(float to, float f) frik_WriteAngle = #57;
void(float to, string s) frik_WriteString	= #58;
void(float to, entity s) frik_WriteEntity	= #59;

//----------------------------------------------------------------------------

		
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Function redclarations. These allow function 
designed to work for clients (sprint, so forth)
to mainly not complain when working with a bot

Although these shouldn't be needed anymore,
as the bots truly are clients now, if you don't
stop the SZ_ buffer from filling up by disabling
direct messages to the bots, it crashes quake :-(

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void(entity	client, string s) stuffcmd =
{
	if (client.ishuman)
		frik_stuffcmd(client, s);
};

void(entity	e) setspawnparms =
{
	if (e.ishuman)
		frik_setspawnparms(e);
	else
		SetNewParms();
};
void(entity	client, float level, string s) sprint =
{
	if (client.ishuman)
		frik_sprint(client, level, s);
};
void(entity	client, string s) centerprint =
{
	if (client.ishuman)
		frik_centerprint(client, s);
};

vector(entity e, float sped) aim =
{
	e.missile_speed = sped;
	return frik_aim(e, sped);
};

void(entity e, float chan, string samp, float vol, float atten) sound = 
{
	if (e.classname == "player")
		e.f_sound = time + 0.5;
	frik_sound(e, chan, samp, vol, atten);
};
void(float to, float f) WriteByte =
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteByte(to, f);
};
void(float to, float f) WriteChar =		
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteChar(to, f);
};
void(float to, float f) WriteShort =	
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteShort(to, f);
};
void(float to, float f) WriteLong = 
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteLong(to, f);
};
void(float to, float f) WriteCoord = 
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteCoord(to, f);
};
void(float to, float f) WriteAngle = 
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteAngle(to, f);
};
void(float to, string s) WriteString = 
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteString(to, s);
};
void(float to, entity s) WriteEntity = 
{
	if ((to == MSG_ONE) && (msg_entity.ishuman != TRUE))
		return;
	frik_WriteEntity(to, s);
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Normal Quake functions, here for the bot ai... 
if they conflict with your mod, delete them

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

float (entity targ) visible =
{
	local vector	spot1, spot2;
	
	spot1 = self.origin + self.view_ofs;
	spot2 = targ.origin + targ.view_ofs;
	traceline (spot1, spot2, TRUE, self);	// see through other monsters
	
	if (trace_inopen && trace_inwater)
		return FALSE;			// sight line crossed contents

	if (trace_fraction == 1)
		return TRUE;
	return FALSE;
};
float(float v) anglemod =
{
	while (v >= 360)
		v = v - 360;
	while (v < 0)
		v = v + 360;
	return v;
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Stuff mentioned up top
it just links the bot into the mod

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
// use switch_wallhug -- that's safe right?

void() ClientFixRankings =
{
	local float cno;
	local entity bott;
	self.ishuman = TRUE; // fix for spectators
	if (self.switch_wallhug > time)
		return;

	self.switch_wallhug = 0;

	bott = nextent(world);
	cno = 0;
	while (cno < fMaxClients)
	{
		
		if ((!bott.ishuman) && clientIsActive(cno)) // maybe?
		{
			svcUpdatePing( bott.fClientNo, 100 * (4 - bott.botskill));
			svcUpdateEnterTime( bott.fClientNo, time - bott.fEnterTime );
			svcUpdateUserInfo( bott.fClientNo, bott.fUserID, bott.fShirt, bott.fPants, bott.netname);
			svcUpdateFrags( bott.fClientNo, bott.frags );
		}
		cno = cno + 1;
		bott = nextent(bott);
	}
};
		
void() ClientInRankings =
{
	local float cno;
	userid = userid + 1;
	self.fUserID = userid;
	if (self.ishuman == 2)
	{
		self.ishuman = FALSE;
		return;
	}
	if (BOT_COUNT)
	{
		b_conversation = 5;
		b_originator = self;
	}
   	cno = self.colormap - 1;
	botInvalidClientNo (cno);	
	clientSetUsed (cno);
	self.fClientNo = cno;
	self.ishuman = TRUE;
	self.switch_wallhug = time + 1;

};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BotPreFrame & BotPostFrame, used to make the
bot easier to install

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
float () BotPreFrame =
{
	if (self.ishuman == TRUE)
		if (self.switch_wallhug)
			ClientFixRankings();
	if (self.ishuman)
		DynamicWaypoint();
	return FALSE;
};
float () BotPostFrame =
{
	botImpulses();
	return FALSE;
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bot Chat code

The rest of this code is in bot_rank.qc

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void(string h) BotSay = // simulate talking by composing a 'chat' message
{
	WriteByte(MSG_ALL, 8);
	WriteByte(MSG_ALL, 3);
	WriteByte(MSG_ALL, 1);
	WriteString(MSG_ALL, self.netname);
	WriteByte(MSG_ALL, 8);
	WriteByte(MSG_ALL, 3);
	WriteByte(MSG_ALL, 2);
	WriteString(MSG_ALL, h);
};
void() BotSayInit =
{
	WriteByte(MSG_ALL, 8);
	WriteByte(MSG_ALL, 3);
	WriteByte(MSG_ALL, 1);
	WriteString(MSG_ALL, self.netname);
};
void(string h) BotSay2 =
{
	WriteByte(MSG_ALL, 8);
	WriteByte(MSG_ALL, 3);
	WriteByte(MSG_ALL, 2);
	WriteString(MSG_ALL, h);
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BotInit

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void() BotInit =
{
	local entity ent, fisent;
	local float numents;
	// this works by counting the number of entities created after world
	// so you must call it right at the beginning of worldspawn in WORLD.QC

	ent = nextent( world );

	while(ent != world)
	{
		fMaxClients = fMaxClients + 1;
		ent = nextent( ent );
	}
	ent = nextent( world );
	while (numents < fMaxClients)
	{
		fisent = spawn();
		ent.phys_obj = fisent;
		fisent.owner = ent;
		numents = numents + 1;
		ent = nextent(ent);
	}
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Rankings 'utilities'. Written by Alan Kivlin,
this code just fools clients by sending precisely
the same network messages as when a real player
signs on to the server.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void(float clientno, float clientfrags) svcUpdateFrags =
{
	WriteByte( 2, 14 );
	WriteByte( 2, clientno );
	WriteShort( 2, clientfrags );
};
void( float clientno, float clientping ) svcUpdatePing =
{
	WriteByte( 2, 36 );
	WriteByte( 2, clientno );
	WriteShort( 2, clientping );
};
void( float clientno, float cliententertime ) svcUpdateEnterTime =
{
	WriteByte( 2, 37 );                // update entertime
	WriteByte( 2, clientno );          // client number
	WriteLong( 2, cliententertime );   // client entertime
};
void(float clientno, float clientid, float clientshirt, float clientpants, string clientname) svcUpdateUserInfo =
{
	WriteByte( 2, 40 );            // update userinfo
	WriteByte( 2, clientno );      // client number
	WriteLong( 2, clientid );      // client userid
	// the following is a breakdown of the client userinfo string	
	// thank you Requiem!

	WriteByte( 2, 92 ); // \
	WriteByte( 2, 98 ); // b
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 116 ); // t
	WriteByte( 2, 116 ); // t
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 109 ); // m
	WriteByte( 2, 99 ); // c
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 108 ); // l
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 114 ); // r
	WriteByte( 2, 92 ); // \
	if( clientpants > 9 )
	{
		WriteByte( 2, 49 );
      	WriteByte( 2, 38 + clientpants );
	}
	else
		WriteByte( 2, 48 + clientpants );
	WriteByte( 2, 92 );  // \
	WriteByte( 2, 116 ); // t
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 112 ); // p
	WriteByte( 2, 99 );  // c
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 108 ); // l
	WriteByte( 2, 111 ); // o
	WriteByte( 2, 114 ); // r
	WriteByte( 2, 92 );  // \
	if( clientshirt > 9 )
	{
		WriteByte( 2, 49 );
		WriteByte( 2, 38 + clientshirt );
	}
	else
      	WriteByte( 2, 48 + clientshirt );
	WriteByte( 2, 92 );  // \
	WriteByte( 2, 116 ); // t
	WriteByte( 2, 101 ); // e
	WriteByte( 2, 97 );  // a
	WriteByte( 2, 109 ); // m
	WriteByte( 2, 92 );  // \
	// FIXME: write team
	WriteByte( 2, 92 );  // \
	WriteByte( 2, 115 ); // s
	WriteByte( 2, 107 ); // k
	WriteByte( 2, 105 ); // i
	WriteByte( 2, 110 ); // n
	WriteByte( 2, 92 );  // \
	WriteByte( 2, 98 );  // b
	WriteByte( 2, 97 );  // a
	WriteByte( 2, 115 ); // s
	WriteByte( 2, 101 ); // e
	WriteByte( 2, 92 );  // \
	WriteByte( 2, 110 ); // n
	WriteByte( 2, 97 );  // a
	WriteByte( 2, 109 ); // m
	WriteByte( 2, 101 ); // e
	WriteByte( 2, 92 );  // \
	WriteString( 2, clientname );
};

float(float clientno) clientBitFlag =
{
   local float bitflag;
   bitflag = 1;
   while(clientno > 0)
   {
      bitflag = bitflag * 2;
      clientno = clientno - 1;
   }
   return bitflag;
};

float(float clientno) clientIsActive =
{
   if( clientno > 16 )
   {
      if( fActiveClients2 & clientBitFlag(clientno - 16))
         return 1;
   }
   else if(fActiveClients1 & clientBitFlag(clientno))
      return 1;
   return 0;
};

void(float clientno) clientSetUsed =
{
   if( clientno > 16 )
      fActiveClients2 = fActiveClients2 | clientBitFlag(clientno - 16);
   else
      fActiveClients1 = fActiveClients1 | clientBitFlag(clientno);
};

void( float clientno ) clientSetFree =
{
   if( clientno > 16 )
      fActiveClients2 = fActiveClients2 - (fActiveClients2 & clientBitFlag(clientno - 16));
   else
      fActiveClients1 = fActiveClients1 - (fActiveClients1 & clientBitFlag(clientno));
};

float() clientNextAvailable =
{
   local float clientno;
   clientno = 0;
   while(clientno < fMaxClients)
   {
      if(!clientIsActive(clientno))
         return clientno;
      clientno = clientno + 1;
   }
   return -1;
};

//----------------------------------------------------------------------------


/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Find Another Color

Team finding code

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

float(float tcolor) FindAnotherColor =
{
	local entity t;
	local float bestbet, scolor, pcount, bestp;
	bestbet = -1;
	bestp = 16;
	while(scolor < 14)
	{
		if (scolor != tcolor)
		{
			t = find(world, classname, "player");
			pcount = 0;
			while(t != world)
			{
				if (t.team == scolor + 1)
					pcount = pcount + 1;
				t = find(t, classname, "player");
			}
			if ((pcount < bestp) && (pcount != 0))
			{
				bestbet = scolor;
				bestp = pcount;
			}
		}
		scolor = scolor + 1;
		
	}
	if (bestbet < 0)
	{
		bestbet = tcolor;
		while (bestbet == tcolor)
		{
			bestbet = floor(random() * 13);
		}
	}
	return bestbet;
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BotConnect and related functions.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/				
entity(float num) GetClientEntity =
{
	local entity upsy;
	upsy = world;
	num = num + 1;
	while (num > 0)
	{
		num = num - 1;
		upsy = nextent(upsy);
	} 
	return upsy;
};

void(float whichteam) botConnect =
{
	local float clientno;
	local entity bot, uself;
	local string g;

	clientno = clientNextAvailable();
	uself = self;
	if(clientno == -1)
	{
		bprint(2, "Unable to connect a bot, server is full.\n");
		return;
	}

	clientSetUsed(clientno);

	BOT_COUNT = BOT_COUNT + 1;
	bot = GetClientEntity(clientno);
	bot.fClientNo = clientno;
	bot.colormap = clientno + 1;
	self = bot;
	bot.netname = PickARandomName();
	g = infokey(world, "skill");
	bot.botskill = stof(g);
	bot.botskill = rint(bot.botskill);
	if (bot.botskill > 3)
		bot.botskill = 3;
	else if (bot.botskill < 0)
		bot.botskill = 0;

	if (teamplay)
	{
		if (whichteam)
			self.fPants = FindAnotherColor(uself.team - 1);
		else
			self.fPants = uself.team - 1;
		self.fShirt = self.fPants;
	}
	bot.team = bot.fPants + 1;
	bot.fEnterTime = time;
	svcUpdateFrags( bot.fClientNo, bot.frags );
      svcUpdatePing( bot.fClientNo, 100 * (4 - bot.botskill));
      svcUpdateEnterTime( bot.fClientNo, time - bot.fEnterTime );
      svcUpdateUserInfo( bot.fClientNo, bot.fUserID, bot.fShirt, bot.fPants, bot.netname);
	SetNewParms();
	self.ishuman = 2; // testing?
	ClientConnect();
	self.ishuman = FALSE;
	PutClientInServer();
	BotStartTopic(self, 1);
	self = uself;
};

void( entity bot ) botDisconnect =
{
	local entity whoself;
	whoself = self;
	self = bot;

	BOT_COUNT = BOT_COUNT - 1;
	ClientDisconnect();

	if( bot.fClientNo != -1 )
	{
      	// the bot's client number is not in use by a real player so we
		// must remove it's entry in the rankings
		// Quake engine sets all fields to 0, can only do the most imprtant here
		self.tfrags = self.frags = 0;
		self.netname = "";
		self.classname = "";
		self.health = 0;
		self.items = 0;
		self.armorvalue = 0;
		self.weaponmodel = "";
		self.phys_obj.modelindex = 0;
		self.ammo_shells = self.ammo_nails = self.ammo_rockets = self.ammo_cells = 0;
		svcUpdateFrags( bot.fClientNo, 0 );
		svcUpdatePing( bot.fClientNo, 0 );
		svcUpdateEnterTime( bot.fClientNo, 0 );
		svcUpdateUserInfo( bot.fClientNo, 0, 0, 0, string_null);
	      clientSetFree( bot.fClientNo );
		bot.fClientNo = -1;
   	}
	self = whoself;
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

botInvalidClientNo
kicks a bot if a player connects and takes the bot's space
FIXME: try to reconnect?

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void( float clientno ) botInvalidClientNo =
{
	local entity bot;

	bot = find( world, classname, "player" );
	while(bot)
	{
		if ((bot.fClientNo == clientno) && (!(bot.ishuman)))
		{
			bot.fClientNo = -1;
			botDisconnect(bot);
			return;
      	}
		bot = find( bot, classname, "player" );
	}
};

void(entity e1, entity e2) DeveloperLightning =
{
	WriteByte (MSG_MULTICAST, 23);
	WriteByte (MSG_MULTICAST, 5);
	WriteEntity (MSG_MULTICAST, e2);
	WriteCoord (MSG_MULTICAST, e1.origin_x);
	WriteCoord (MSG_MULTICAST, e1.origin_y);
	WriteCoord (MSG_MULTICAST, e1.origin_z);
	WriteCoord (MSG_MULTICAST, e2.origin_x);
	WriteCoord (MSG_MULTICAST, e2.origin_y);
	WriteCoord (MSG_MULTICAST, e2.origin_z);
	multicast (e2.origin, MULTICAST_PHS);
};

void() BotFrame =
{
	local float num;

	// for the sake of speed
	sv_maxspeed = cvar("sv_maxspeed");
	sv_gravity = cvar("sv_gravity");
	sv_friction = cvar("sv_friction");
	sv_accelerate = cvar("sv_accelerate");
	sv_stopspeed = cvar("sv_stopspeed");

	self = nextent(world);
	num = 0;
	while (num < fMaxClients)
	{
		if (self.ishuman == FALSE)
		{
			if (clientIsActive(num))
			{
				if (self.tfrags != self.frags)
					svcUpdateFrags (self.fClientNo, self.frags);
				CL_KeyMove();
				SV_ClientThink();
				SV_Physics_Client();
				// this is sickening	
				if (self.phys_obj)
				{
					if (self.phys_obj.modelindex != self.modelindex)
					{
						setmodel(self.phys_obj, "progs/player.mdl");
						self.phys_obj.modelindex = self.modelindex;
					}
					self.phys_obj.frame = self.frame;
					self.phys_obj.angles = self.angles;
					self.phys_obj.avelocity = self.velocity;
					self.phys_obj.colormap = self.colormap;
				}
			}
		}
		self = nextent(self);
		num = num + 1;
	}

};


/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bot Impulses. Allows the player to perform bot
related functions.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


void() botImpulses =
{
	if (self.impulse == BOT_IMPULSE)
	{
		botConnect(0);
		self.impulse = 0;
	}
	else if (self.impulse == (BOT_IMPULSE + 1))
	{
		botConnect(1);
		self.impulse = 0;
	}
	else if (self.impulse == (BOT_IMPULSE + 2))
	{
		KickABot();
		self.impulse = 0;
	}
	// bot cam not available in QW
	// waypoint dump not available in Qw
};
