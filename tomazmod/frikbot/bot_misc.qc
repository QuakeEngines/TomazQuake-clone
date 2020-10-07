/***********************************************
*                                              *
*              FrikBot Misc Code               *
*   "Because you can't name it anything else"  *
*                                              *
***********************************************/

string(float r) BotName =
{
	if (r == 1)
	{	
		self.fPants = 11;
		self.fShirt = 0; 
		return "Vincent";
	}
	else if (r == 2)
	{
		self.fPants = 1;
		self.fShirt = 3;
		return "Bishop";
	}
	else if (r == 3)
	{	
		self.fPants = 13;
		self.fShirt = 2;
		return "Nomad";
	}
	else if (r == 4)
	{
		self.fPants = 7;
		self.fShirt = 6;
		return "Hudson";
	} 
	else if (r == 5)
	{
		self.fPants = 12;
		self.fShirt = 6;
		return "Lore";
	}
	else if (r == 6)
	{
		self.fPants = 4;
		self.fShirt = 4;
		return "Servo";
	}
	else if (r == 7)
	{
		self.fPants = 2;
		self.fShirt = 5;
		return "Gort";
	} 
	else if (r == 8)
	{	
		self.fPants = 10;
		self.fShirt = 3;
		return "Kryten";
	}
	else if (r == 9)
	{
		self.fPants = 9;
		self.fShirt = 4;
		return "Pimp Bot";
	}
	else if (r == 10)
	{
		self.fPants = 4;
		self.fShirt = 7;
		return "Max";
	}
	else if (r == 11)
	{
		self.fPants = 3;
		self.fShirt = 11;
		return "Marvin";
	} 
	else if (r == 12)
	{
		self.fPants = 13;
		self.fShirt = 12;
		return "Erwin";
	}
	else if (r == 13)
	{
		self.fPants = 11;
		self.fShirt = 2;
		return "FrikBot";
	}
	else if (r == 14)
	{
		self.fPants = 0;
		self.fShirt = 2;
		return "Krosis";
	}
	else if (r == 15)
	{
		self.fPants = 8;
		self.fShirt = 9;
		return "Gypsy";
	}
	else 
	{
		self.fPants = 5;
		self.fShirt = 10;
		return "Hal";
	}
};
string () PickARandomName =
{
	if (BOT_COUNT > 16)
		return "player";

	local float y, test;
	local string h;
	local entity t;
	y = TRUE;
	while(y)
	{
		test = ceil(random() * 16);
		h = BotName(test);
		t = find(world, netname, h);
		if (t == world)
			y = FALSE;
	}
	return h;
};


/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bot Chat code

This code is totally ugly. Overt your eyes. You
can delete this if you're having trouble with pr_globals
or you hate it. It's just for fun.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void(entity t, float y1) BotStartTopic =
{
	if (random() < 0.5)
	{
		// set the bot's chat_subj and chat_time variables
		t.chat_subj = y1;
		t.chat_time = time + 2 + random() * 4;
		b_conversation = 0;
	}
};
// topics:
// 1 - sign on
// 2 - frag more on update
// 3 - frag less on update
// 4 - no frag difference, but died [not used]
// 5 - welcoming someone onto server
// 6 - being annoying
// 7 - look at the score board, pal
// 8 - go easy on me
// 9 - lag
// 10 - "nope"
// 11 - you suck
// 12 - gameover
void() BotChat =
{
	local float r;
	if (self.tfrags != self.frags)
	{
		if (self.tfrags < self.frags)
			BotStartTopic(self, 2);
		else
			BotStartTopic(self, 3);
		self.tfrags = self.frags;
		return;
	}
	if (self.view_ofs == '0 0 0') // we'll assume this is an intermission
	{
		if (!self.chat_subj)
			BotStartTopic(self, 12);
	}
/* not going to use this yet
	if (self.health <= 0)
	{
		if (!self.chat_subj)
			BotStartTopic(self, 4);
	}
*/
	if ((b_conversation != 0) && (b_originator != self))
	{
		if (b_conversation == 7)
		{
			if (self.frags <= b_originator.frags)
			{
				if (random() < 0.2)
					b_conversation = 0;
				return;
			}
		}
		BotStartTopic(self, b_conversation);
		if (random() < 0.2)
			b_conversation = 0;
		return;
	}
	if (!self.chat_subj)
		return;
	else if (self.chat_time > time)
		return;

	r = ceil(random() * 10);
	if (self.chat_subj == 1)
	{
		if (r == 1)
		{
			BotSay(": hey folks!\n");
			b_conversation = 5;
			b_originator = self;
		}
		else if (r == 2)
		{
			if (BOT_COUNT > 1)
				BotSay(": wow, a whole bunch of losers on this server!\n");
			else
				BotSay(": what a lame server this is\n");
			b_conversation = 6;
			b_originator = self;
		}
		else if (r == 3)
		{
			BotSay(": hey everyone, go easy on me I'm still pretty new at this\n");
			b_conversation = 8;
			b_originator = self;
		}
		else if (r == 4)
		{
			BotSay (": hmmm, this is an awful laggy server\n");
			b_conversation = 9;
			b_originator = self;
		}
		else if (r == 5)
		{
			BotSay (": hey all, hows it going?\n");
			b_conversation = 5;
			b_originator = self;
		}
		else if (r == 6)
		{
			// cute idea I had...
			local entity oself;
			BotSayInit ();
			BotSay2(": anyone seen "); 
			oself = self;
			self = self.phys_obj;
			local string s;
			s = PickARandomName();
			self = oself;
			BotSay2(s);
			BotSay2("?\n");
		}
		else if (r == 7)
		{
			BotSay (": i'm here finally!\n");
			b_conversation = 5;
			b_originator = self;	
		}
		else if (r == 8)
		{
			BotSay (": man my connection is acting up today\n");
			b_conversation = 9;
			b_originator = self;	
		}
		else if (r == 9)
		{
			BotSay (": now the fun begins!\n");
			b_conversation = 6;
			b_originator = self;	
		}
		else
		{
			BotSay (": hey\n");
			b_conversation = 5;
			b_originator = self;
		}
	}	
	else if (self.chat_subj == 2)
	{
		if (r == 1)
		{
			BotSay(": yeeehaw!\n");
			b_conversation = 6;
			b_originator = self;
		}
		else if (r == 2)
			BotSay(": take that\n");
		else if (r == 3)
			BotSay(": heh heh\n");
		else if (r == 4)
		{
			BotSay(": eat that\n");
			b_conversation = 6;
			b_originator = self;
		}
		else if (r == 5)
		{
			BotSay(": i rule\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 6)
		{
			BotSay(": you suck\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 7)
		{
			BotSay(": i can kick your butt any day\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 8)
		{
			BotSay(": that's gotta hurt\n");
			b_conversation = 6;
			b_originator = self;
		}
		else if (r == 9)
		{
			BotSay(": no chance against me, pal\n");
			b_conversation = 7;
			b_originator = self;
		}
		else
			BotSay(": and stay down!\n");
	} else if (self.chat_subj == 3)
	{
		if (r == 1)
		{
			BotSay(": oh man!\n");
			b_conversation = 11;
			b_originator = self;
		}
		else if (r == 2)
			BotSay(": grrr\n");
		else if (r == 3)
		{
			BotSay(": heh, oops\n");
			b_conversation = 11;
			b_originator = self;
		}
		else if (r == 4)
		{
			BotSay(": that was lag!\n");
			b_conversation = 9;
			b_originator = self;
		}
		else if (r == 5)
		{
			BotSay(": i need practice\n");
			b_conversation = 8;
			b_originator = self;
		}
		else if (r == 6)
		{
			BotSay(": damnit\n");
			b_conversation = 11;
			b_originator = self;
		}
		else if (r == 7)
		{
			BotSay(": i suck\n");
			b_conversation = 8;
			b_originator = self;
		}
		else if (r == 8)
			BotSay(": doh!\n");
		else if (r == 9)
		{
			BotSay(": whoops\n");
			b_conversation = 11;
			b_originator = self;
		}
		else
		{
			BotSay(": my mouse needs cleaning\n");
			b_conversation = 11;
			b_originator = self;
		}
	}
	else if (self.chat_subj == 5)
	{
		if (r == 1)
		{
			BotSay(": hey, welcome!\n");
			b_conversation = 0;
		}
		else if (r == 2)
		{
			BotSay(": hey there, I pretty much have these idiots under control\n");
			b_conversation = 7;
			b_originator = self;
		}

		else if (r == 3)
		{
			BotSayInit();
			BotSay2(": hiya ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 4)
		{
			BotSay(": get out of here, we don't need more people\n");
			b_conversation = 6;
			b_originator = self;		
		}
		else if (r == 5)
		{
			BotSay(": hi, welcome to the fun\n");
			b_conversation = 0;
		}
		else if (r == 6)
		{
			BotSayInit();
			BotSay2(": hey ");
			BotSay2(b_originator.netname);
			BotSay2(", glad you could make it\n");
			b_conversation = 0;
		}
		else if (r == 7)
		{
			BotSay(": great, another newbie to dice up :)\n");
			b_conversation = 6;
			b_originator = self;		
		}
		else if (r == 8)
		{
			BotSayInit();
			BotSay2(": welcome ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 9)
		{
			BotSay(": hello, i'm a frikbot, I'm here to turn you into a bloody mess\n");
			b_originator = self;
			b_conversation = 7;	
		}
		else
		{
			BotSay(": howdy\n");
			b_conversation = 0;
		}

	}
	else if (self.chat_subj == 6)
	{
		if (r == 1)
		{
			BotSayInit();
			BotSay2(": yeah shut up ");
			BotSay2(b_originator.netname);
			BotSay2(" you play like my grandmother\n");
			b_conversation = 0;
		}
		else if (r == 2)
		{
			BotSayInit();
			BotSay2(": man, ");
			BotSay2(b_originator.netname);
			BotSay2(" you are annoying\n");
			b_conversation = 0;
		}
		else if (r == 3)
		{
			BotSayInit();
			BotSay2(": if I had one wish, it would be to get rid of ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 4)
		{
			BotSay(": you will pay for that remark\n");
			b_conversation = 0;
		}
		else if (r == 5)
		{
			BotSay(": yeah, blow me\n");
			b_conversation = 6;
			b_originator = self;
			// this might be bad
		}
		else if (r == 6)
		{
			BotSayInit();
			BotSay2(": can someone kick ");
			BotSay2(b_originator.netname);
			BotSay2(", he's getting on my nerves\n");
			b_conversation = 0;
		}
		else if (r == 7)
		{
			BotSay(": yeah, right\n");
			b_conversation = 0;
		}
		else if (r == 8)
		{
			BotSay(": uh-huh, whatever, I'm still the best player here\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 9)
		{
			BotSayInit();
			BotSay2(": okay, ");
			BotSay2(b_originator.netname);
			BotSay2(" i'm going to kill you now\n");
			b_conversation = 11;
			b_originator = self;
		}
		else 
		{
			BotSayInit();
			BotSay2(": yeah right, ");
			BotSay2(b_originator.netname);
			BotSay2(" i'm better than you all\n");
			b_conversation = 7; // I like 7 don't i?
			b_originator = self;
		}
	}
	else if (self.chat_subj == 7)
	{
		if (r == 1)
		{
			BotSay(": yeah right, look at the scoreboard, loser\n");
			b_conversation = 0;
		}
		else if (r == 2)
		{
			BotSay(": you wish\n");
			b_conversation = 6;
			b_originator = self; // bwhahah
		}
		else if (r == 3)
		{
			BotSay(": you better look at the scores before you say that crap\n");
			b_conversation = 0;
		}
		else if (r == 4)
		{
			BotSay(": i don't think so, look at the scores\n");
			b_conversation = 0;
		}
		else if (r == 5)
		{
			BotSay(": look who's talking\n");
			b_conversation = 0;
		}
		else if (r == 6)
		{
			BotSayInit();
			BotSay2(": yeah, but I'm better than you ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 6; 
			b_originator = self;
		}
		else if (r == 7)
		{
			BotSayInit();
			BotSay2(": I don't think so, ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 8)
		{
			BotSay(": hah! look at the damn scores\n");
			b_conversation = 0;
		}
		else if (r == 9)
		{
			BotSay(": yeah right\n");
			b_conversation = 0;
		}
		else
		{
			BotSay(": ummmm look at the scoreboard\n");
			b_conversation = 0;
		}

	}
	else if (self.chat_subj == 8)
	{
		if (r == 1)
		{
			BotSayInit();
			BotSay2(": yeah let's go easy on ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 2)
		{
			BotSay (": tough luck, you'll die like everyone else\n");//hmm
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 3)
		{
			BotSayInit();
			BotSay2(": heh, everyone kill ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 6;
			b_originator = self;
		}
		else if (r == 4)
		{
			BotSayInit();
			BotSay2(": we'll go easy on you ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 5)
		{
			BotSay(": you'll get no pity from us\n");
			b_conversation = 0;
		}
		else if (r == 6)
		{
			BotSay(": aha! someone i might be able to kill\n");
			b_conversation = 11;
			b_originator = self;
		}
		else if (r == 7)
		{
			BotSay(": mess with the best, die like the rest\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 8)
		{
			BotSay(": yeah everyone, take it easy\n");
			b_conversation = 0;
		}
		else if (r == 9)
		{
			BotSayInit();
			BotSay2(": tough luck ");
			BotSay2(b_originator.netname);
			BotSay2(" i'm on a killing streak\n");
			b_conversation = 7;
			b_originator = self;
		}
		else
		{
			BotSayInit();
			BotSay2(": go easy on my buddy ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
	}
	else if (self.chat_subj == 9)
	{
		if (r == 1)
		{
			BotSay(": you idiot, your a bot!\n");
			b_conversation = 0;
		}
		else if (r == 2)
		{
			BotSay(": yeah lag is hitting me pretty hard too\n");
			b_conversation = 9;
			b_originator = self;
		}
		else if (r == 3)
		{
			BotSay(": yeah me too, that's why I'm not doing so good\n");
			b_conversation = 8;
			b_originator = self;
		}
		else if (r == 4)
		{
			BotSay(": ping is nothing, skill is everything. i rule\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 5)
		{
			BotSay(": try adjusting your rate\n");
			b_conversation = 0;
		}
		else if (r == 6)
		{
			BotSay(": get a better modem, me I'm on a cable\n");
			b_conversation = 0;
		}
		else if (r == 7)
		{
			BotSay(": listen server, they're the worst\n");
			b_conversation = 0;
		}
		else if (r == 8)
		{
			BotSay(": it's not the lag, it's the programming\n");
			b_conversation = 0;
		}
		else if (r == 9)
		{
			BotSayInit();
			BotSay2(": uhh... i thought ");
			BotSay2(b_originator.netname);
			BotSay2(" was a bot?\n");
			b_conversation = 10;
			b_originator = self;
		}
		else
		{
			BotSay(": lag or no lag, i can kick your ass\n");
			b_conversation = 7;
			b_originator = self;
		}

	}
	else if (self.chat_subj == 10)
	{ // simple no to a question
		if (r < 5)
			BotSay(": nope\n");
		else
			BotSay(": no\n");
		b_conversation = 0;
	}
	else if (self.chat_subj == 11)
	{
		if (r == 1)
		{
			BotSay(": haha! you suck!\n");
			b_conversation = 0;
		}
		else if (r == 2)
		{
			BotSayInit();
			BotSay2(": lol, what a llama ");
			BotSay2(b_originator.netname);
			BotSay2(" is\n");
			b_conversation = 0;
		}
		else if (r == 3)
		{
			BotSay(": you suck\n");
			b_conversation = 0;
		}
		else if (r == 4)
		{
			BotSayInit();
			BotSay2(": you suck ");
			BotSay2(b_originator.netname);
			BotSay2("\n");
			b_conversation = 0;
		}
		else if (r == 5)
		{
			BotSay(": man i feel sorry for these frikbots, they suck\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 6)
		{
			BotSay(": hehe\n");
			b_conversation = 0;
		}
		else if (r == 7)
		{
			BotSay(": you really suck\n");
			b_conversation = 0;
		}
		else if (r == 8)
		{
			BotSay(": can you guys please shut up an let me concentrate\n");
			b_conversation = 6;
			b_originator = self;
		}
		else if (r == 9)
		{
			BotSay(": hahahahahahahahahahahahahahaha!\n");
			b_conversation = 0;
		}
		else
		{
			BotSay(": what a bunch of losers you all are\n");
			b_conversation = 7;
			b_originator = self;
		}
	}
	else if (self.chat_subj == 12)
	{
		if (r == 1)
			BotSay(": gg\n");
		else if (r == 2)
			BotSay(": that was fun\n");
		else if (r == 3)
		{
			local entity t;
			t = find(world, classname, "player"); // should pick highest fragger, but oh well
			if (t == self)
				t = find(t, classname, "player"); 
			if (!t)
				return;
			BotSayInit();
                        BotSay2(": next time we should all pick on ");
			BotSay2(t.netname);
			BotSay2("\n");
		}
		else if (r == 4)
		{
			BotSay(": heh, i'm the best player here\n"); // 7 again
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 5)
			BotSay(": i had fun\n");
		else if (r == 6)
			BotSay(": good game\n");
		else if (r == 7)
		{
			BotSay(": gg, too laggy though\n");
			b_conversation = 9;
			b_originator = self;
		}
		else if (r == 8)
		{
			BotSay(": it was fun wiping the floor with all you llamas\n");
			b_conversation = 7;
			b_originator = self;
		}
		else if (r == 9)
		{
			BotSay(": man that map sucked\n"); // need a map topic
		}
		else
		{
			BotSay(": gg, cya\n");
		}
	}

	self.chat_subj = 0;
	self.chat_time = time;

};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Kick A Bot. 

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void() KickABot =
{
	local entity ty;
	ty = find(world, classname, "player");
	while (ty != world)
	{
		if (!(ty.ishuman))
		{
			
			botDisconnect(ty);
			ty.ishuman = TRUE;
			ty = world;
		}
		else
			ty = find(ty, classname, "player");
	}

};


/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

fisible, a version of visible that checks for corners of the bounding boxes

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

vector(entity ent) realorigin =
{
	if (ent.origin != '0 0 0')
		return ent.origin;
	else
		return (ent.absmin + ent.absmax) * 0.5;
};

float (entity targ) fisible =
{
	local vector	spot1, spot2, org;
	local float thruwater;
	org = realorigin(targ);

	if ((targ.classname == "player") || (pointcontents(org) == CONTENT_LAVA))
		thruwater = FALSE;
	else
		thruwater = TRUE;

	spot1 = self.origin + self.view_ofs;
	spot2 = org;
	traceline (spot1, spot2, TRUE, self);	
	if (!(trace_inopen && trace_inwater) || thruwater)	
		if ((trace_ent == targ) || (trace_fraction == 1))
			return TRUE;
	if (targ.solid == SOLID_BSP)
		return FALSE;
	spot2 = org + targ.mins;
	traceline (spot1, spot2, TRUE, self);
	if (!(trace_inopen && trace_inwater) || thruwater)	
		if ((trace_ent == targ) || (trace_fraction == 1))
			return TRUE;
	spot2 = org + targ.maxs;
	traceline (spot1, spot2, TRUE, self);	
	if (!(trace_inopen && trace_inwater) || thruwater)	
		if ((trace_ent == targ) || (trace_fraction == 1))
			return TRUE;
	return FALSE;
};
// this is used for waypoint stuff....
float (entity targ) wisible =
{
	local vector	spot1, spot2;
	local entity ignore;

	spot1 = self.origin + self.view_ofs;
	spot2 = realorigin(targ);
	
	ignore = self;
	do
	{
		traceline (spot1, spot2, TRUE, ignore);
		spot1 = realorigin(trace_ent);
                ignore = trace_ent;
	} while ((trace_ent != world) && (trace_fraction != 1));
	if (trace_endpos == spot2)
		return TRUE;
	else 
		return FALSE;
};

float (float y1, float y2) angcomp = 
{

	y1 = anglemod(y1);
	y2 = anglemod(y2);

	local float answer;
	answer = y1 - y2;
	if (answer > 180)
		answer = (360 - answer) * -1;
	else if (answer < -180)
		answer = answer + 360;
	return answer;

};


float (entity targ) fov =
{
	local vector yawn;
	local float g;
	yawn = realorigin(targ);
	yawn = (yawn + targ.view_ofs) - (self.origin + self.view_ofs);
	yawn = normalize(yawn);
	yawn = vectoangles(yawn);
	g = angcomp(self.v_angle_x, yawn_x);
	if (fabs(g) > 45)
		return FALSE;
	g = angcomp(self.v_angle_y, yawn_y);
	if (fabs(g) > 60)
		return FALSE;
 
	return TRUE;
};

