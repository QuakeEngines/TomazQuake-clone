
/*
======================================
FrikBot version 0.09 REVISION B
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
These installation instructions only apply to Normal Quake (as does this
entire file). For QuakeWorld, please refer to bot_qw.qc

--------------------------------------
To install on a new mod, do all this:
--------------------------------------
Place all included bot*.qc files in the subdirectory "frikbot"
in your source folder, then...

* Add the following lines to progs.src right after the defs.qc line
frikbot/bot.qc
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
.float wallhug, keys, ishuman, tfrags, fClientNo, fShirt, fPants; 
.float chat_subj, chat_time, priority, ai_time, f_sound, missile_speed;
.float portal_time, botskill, switch_wallhug, failure_time, obs_time;
.entity temp_way, last_way;
.entity phys_obj;
.vector b_angle, ldest1, mouse_emu, obs_dir;
.entity target1, target2, target3, target4;
.vector movevect; // movevect: X = forward/back, Y = left/right, Z = up/down

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
float fActiveClients;
float fMaxClients;
float BOT_COUNT;
float WAYPOINT_MODE; 
float WAYPOINTS;
float b_conversation; // used by over-done chat code
entity b_originator, b_target; // ditto
float sv_friction, sv_gravity, sv_accelerate, sv_maxspeed, sv_stopspeed;
entity fixer;

// -------ProtoTypes------
// external
void() ClientConnect;
void() ClientDisconnect;
float(float v) anglemod;
void() SetNewParms;
float (entity targ) visible;
// rankings
float( float fClientNo ) clientBitFlag;
float( float fClientNo ) clientIsActive;
void( float fClientNo ) clientSetUsed;
void( float fClientNo ) clientSetFree;
float() clientNextAvailable;
void(float whichteam) botConnect;
void( entity bot ) botDisconnect;
void( float clientno ) botInvalidClientNo;
void( float clientno, string clientname ) msgUpdateNameToAll;
void( float clientno, float clientshirt, float clientpants ) msgUpdateColorsToAll;
void( float clientno, float clientfrags ) msgUpdateFragsToAll;
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
void(entity client, string s) frik_sprint = #24;
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
	local entity jj;
	if (client.ishuman)
		frik_stuffcmd(client, s);
	jj = find(world, classname, "botcam");
	while(jj != world)
	{
		if ((jj.enemy == client) && jj.ishuman)
			frik_stuffcmd(jj, s);
		jj = find(jj, classname, "botcam");	
	}
};

void(entity	e) setspawnparms =
{
	if (e.ishuman)
		frik_setspawnparms(e);
	else
		SetNewParms();
};
void(entity	client, string s) sprint =
{
	local entity jj;
	if (client.ishuman)
		frik_sprint(client, s);
	jj = find(world, classname, "botcam");
	while(jj != world)
	{
		if ((jj.enemy == client) && jj.ishuman)
			frik_sprint(jj, s);
		jj = find(jj, classname, "botcam");	
	}

};
void(entity	client, string s) centerprint =
{
	local entity jj;
	if (client.ishuman)
		frik_centerprint(client, s);
	jj = find(world, classname, "botcam");
	while(jj != world)
	{
		if ((jj.enemy == client) && jj.ishuman)
			frik_centerprint(jj, s);
		jj = find(jj, classname, "botcam");	
	}
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

Bot Cam, see what the bot sees (or any other player)

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

float() botcam =
{

	if (self.classname != "botcam")
		return FALSE;
	setorigin(self, self.enemy.origin);
	self.items = self.enemy.items;
	self.weapon = self.enemy.weapon;
	self.weaponmodel = self.enemy.weaponmodel;
	self.currentammo = self.enemy.currentammo;
	self.weaponframe = self.enemy.weaponframe;
	self.ammo_shells = self.enemy.ammo_shells;
	self.ammo_nails = self.enemy.ammo_nails;
	self.ammo_rockets= self.enemy.ammo_rockets;
	self.ammo_cells = self.enemy.ammo_cells;
	self.view_ofs = self.enemy.view_ofs;
	self.health = self.enemy.health;
	self.armorvalue = self.enemy.armorvalue;
	self.dmg_take = self.enemy.dmg_take;
	self.dmg_save = self.enemy.dmg_save;
	self.dmg_inflictor = self.enemy.dmg_inflictor;
	if (self.enemy.ishuman == FALSE)
	{
		self.enemy.dmg_take = 0;
		self.enemy.dmg_save = 0;
        }
	self.punchangle = self.enemy.punchangle;
	self.deadflag = self.enemy.deadflag;
	msg_entity = self;
      WriteByte (MSG_ONE,5);
      WriteEntity (MSG_ONE, self.enemy); //uhh maybe?
	WriteByte (MSG_ONE, 10);
     	WriteAngle (MSG_ONE,self.enemy.v_angle_x);
     	WriteAngle (MSG_ONE,self.enemy.v_angle_y);
     	WriteAngle (MSG_ONE,self.enemy.v_angle_z);
	self.modelindex = 0;
	
	self.impulse = 0;	
	return TRUE;
	
};

void() botcam_u = 
{
	local float loopcount;
	loopcount = 0;
	if (self.classname != "botcam")
		self.enemy = world;
	self.classname = "botcam";
	self.solid = SOLID_NOT;
	self.movetype = MOVETYPE_NONE;
	self.takedamage = DAMAGE_NO;
	
	self.enemy = find(self.enemy, classname, "player");
	if (self.enemy == world)
	{
		sprint(self, "No one left to track! \n");
		msg_entity = self;
		WriteByte (MSG_ONE,5);
		WriteEntity (MSG_ONE, self);
		PutClientInServer();
		return;
	}
	if (self.enemy.ishuman == FALSE)
	{
		self.enemy.dmg_take = 0;
		self.enemy.dmg_save = 0;
	}
	sprint(self, "Now tracking ");
	sprint(self, self.enemy.netname);
	sprint(self, "\n");
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
	if (self.switch_wallhug > time)
		return;
	self.switch_wallhug = 0;

	bott = nextent(world);
	cno = 0;

	while (cno < fMaxClients)
	{
		
		if ((!bott.ishuman) && clientIsActive(cno)) // maybe?
		{
			msgUpdateColorsToAll( bott.fClientNo, bott.fShirt, bott.fPants );
			msgUpdateNameToAll( bott.fClientNo, bott.netname );
			msgUpdateFragsToAll( bott.fClientNo, bott.frags );
		}
		cno = cno + 1;
		bott = nextent(bott);
	}
};
		
void() ClientInRankings =
{
	local float cno;
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
	if (self.ishuman)
		if (self.switch_wallhug)
			ClientFixRankings();
	if (self.classname == "botcam")
		return TRUE;
	else
	{
		if (self.ishuman)
			DynamicWaypoint();
		return FALSE;
	}
};
float () BotPostFrame =
{
	botImpulses();
	if (botcam())
		return TRUE;
	else
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
	WriteByte(MSG_ALL, 1);
	WriteString(MSG_ALL, self.netname);
	WriteByte(MSG_ALL, 8);
	WriteByte(MSG_ALL, 2);
	WriteString(MSG_ALL, h);
};
void() BotSayInit =
{
	WriteByte(MSG_ALL, 8);
	WriteByte(MSG_ALL, 1);
	WriteString(MSG_ALL, self.netname);
};
void(string h) BotSay2 =
{
	WriteByte(MSG_ALL, 8);
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

	ent = nextent(world);

	while(ent != world)
	{
		fMaxClients = fMaxClients + 1;
		ent = nextent( ent );
	}
	// spawn entities for the physics
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


void(float clientno, string clientname) msgUpdateNameToAll =
{
   WriteByte( MSG_ALL, MSG_UPDATENAME );
   WriteByte( MSG_ALL, clientno );
   WriteString( MSG_ALL, clientname );
};

void(float clientno, float clientshirt, float clientpants) msgUpdateColorsToAll =
{
   WriteByte( MSG_ALL, MSG_UPDATECOLORS );
   WriteByte( MSG_ALL, clientno );
   WriteByte( MSG_ALL, clientshirt * 16 + clientpants );
};

void(float clientno, float clientfrags) msgUpdateFragsToAll =
{
   WriteByte( MSG_ALL, MSG_UPDATEFRAGS );
   WriteByte( MSG_ALL, clientno );
   WriteShort( MSG_ALL, clientfrags );
};

float(float clientno) clientBitFlag =
{
   local float bitflag;
   bitflag = 1;
   while( clientno > 0 )
   {
      bitflag = bitflag * 2;
      clientno = clientno - 1;
   }
   return bitflag;
};

float(float clientno) clientIsActive =
{
   if(fActiveClients & clientBitFlag(clientno))
      return TRUE;
   return FALSE;
};

void(float clientno) clientSetUsed =
{
   fActiveClients = fActiveClients | clientBitFlag(clientno);
};

void(float clientno) clientSetFree =
{
   fActiveClients = fActiveClients - 
      (fActiveClients & clientBitFlag(clientno));
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
	local entity bot;
	local entity uself;
	
	clientno = clientNextAvailable();
	uself = self;
	if(clientno == -1)
	{
		bprint("Unable to connect a bot, server is full.\n");
		return;
	}

	clientSetUsed(clientno);

	BOT_COUNT = BOT_COUNT + 1;
	bot = GetClientEntity(clientno);
	bot.fClientNo = clientno;
	bot.colormap = clientno + 1;
	self = bot;
	bot.netname = PickARandomName();
	bot.botskill = cvar("skill");
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

	msgUpdateColorsToAll( bot.fClientNo, bot.fShirt, bot.fPants );
	msgUpdateNameToAll( bot.fClientNo, bot.netname );
	msgUpdateFragsToAll( bot.fClientNo, bot.frags );
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
		self.ammo_shells = self.ammo_nails = self.ammo_rockets = self.ammo_cells = 0;

	      msgUpdateNameToAll( bot.fClientNo, string_null );
      	msgUpdateColorsToAll( bot.fClientNo, 0, 0 );
      	msgUpdateFragsToAll( bot.fClientNo, 0 );

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
	// it's neccesary to kick the bot completely as they
	// cannot simply be re-assigned a client number as before

	while(bot)
	{
		if(( bot.fClientNo == clientno ) && (!(bot.ishuman)))
		{
			bot.fClientNo = -1;
			botDisconnect( bot );
			return;
		}

		bot = find( bot, classname, "player" );
	}
};

//----------------------------------------------------------------------------

void(entity e1, entity e2) DeveloperLightning =
{
	// used to show waypoint links for debugging
	WriteByte (MSG_BROADCAST, 23);
	WriteByte (MSG_BROADCAST, 5);
	WriteEntity (MSG_BROADCAST, e2);
	WriteCoord (MSG_BROADCAST, e1.origin_x);
	WriteCoord (MSG_BROADCAST, e1.origin_y);
	WriteCoord (MSG_BROADCAST, e1.origin_z);
	WriteCoord (MSG_BROADCAST, e2.origin_x);
	WriteCoord (MSG_BROADCAST, e2.origin_y);
	WriteCoord (MSG_BROADCAST, e2.origin_z); 
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Waypoint Loading from file

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void() LoadWaypoint = 
{
	local vector org;
	local entity tep;
	local float r;
	org_x = cvar("saved1");
	org_y = cvar("saved2");
	org_z = cvar("saved3");
	
	tep = make_waypoint(org);
	// I know this looks weirder than hell, but
	// the deal is, I can't restore links until all the ways are spawned
	r = cvar("saved4"); // telelink flags
	r = floor(r / 4);
	tep.chat_time = (r & 15);
	tep.wallhug = ((r & 16) > 0);
	tep.missile_speed = cvar("scratch1");
	tep.chat_subj = cvar("scratch2");
	tep.keys = cvar("scratch3");
	tep.tfrags = cvar("scratch4");
};


void() WaypointWatch =
{
	if (fMaxClients < 2) // don't waypoint in SP
		return;
	local float g;
	g = cvar("saved4");
	if (g != 0)
	{
		if ((g & 3) == 1)
			ClearAllWays();
		else if ((g & 3) == 3)
		{
			FixWaypoints();
			cvar_set("saved4", "0");
			return;
		}
		LoadWaypoint();
		WAYPOINT_MODE = 2;
		cvar_set("saved4", "0");

	}
	else if (WAYPOINT_MODE == 0)
	{
		localcmd("exec maps/");
		localcmd(mapname);
		localcmd(".way\n");
		WAYPOINT_MODE = 1;
	}

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
				CL_KeyMove();
				SV_ClientThink();
				SV_Physics_Client();
			}
		}
		self = nextent(self);
		num = num + 1;
	}
	WaypointWatch();

};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Waypoint Saving to file. 

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void() PrintWaypoint =
{
	local entity t;
	t = find(self.enemy, classname, "waypoint");
	if (t == world)
	{
		remove(self);
		fixer = world;
		bprint("saved4 3\n// end waypoint dump\n");
		return;
	}
	bprint("saved1 ");
	bprint(ftos(t.origin_x));
	bprint("; saved2 ");
	bprint(ftos(t.origin_y));
	bprint("; saved3 ");
	bprint(ftos(t.origin_z));

	bprint("\nscratch1 ");
	bprint(ftos(t.target1.count));
	bprint("; scratch2 ");
	bprint(ftos(t.target2.count));
	bprint("; scratch3 ");
	bprint(ftos(t.target3.count));
	bprint("; scratch4 ");
	bprint(ftos(t.target4.count));
	bprint("\nsaved4 ");
	if (t.count != 1)
		bprint(ftos((t.chat_time * 4) + ((t.wallhug > 0) * 64) + 2));
	else 
		bprint(ftos((t.chat_time * 4) + ((t.wallhug > 0) * 64) + 1));
	bprint ("; wait\n");
	self.nextthink = time + 0.01;
	self.enemy = t;

};

// to allow for 100+ waypoints, we need to trick the runaway loop counter
void() DumpWaypoints =
{
	bprint("// begin waypoint dump - maps/");
	bprint(mapname);
	bprint(".way\n");
	FixNumbers();
	if (!fixer)
	{
		fixer = spawn();
		fixer.nextthink = time + 0.01;
		fixer.think = PrintWaypoint;
		fixer.enemy = world;
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
	else if (self.impulse == (BOT_IMPULSE + 3))
	{
		botcam_u();
		self.impulse = 0;
	}
	else if (self.impulse == (BOT_IMPULSE + 4))
	{
		WAYPOINT_MODE = 2; // don't allow modification when dumping...bad
		DumpWaypoints();
		self.impulse = 0;
	}


};


