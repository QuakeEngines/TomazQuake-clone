/***********************************************
*                                              *
*            FrikBot General AI                *
*     "The I'd rather be playing Quake AI"     *
*                                              *
***********************************************/

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Target Stack code.

target1 is the most immediate target.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
float(entity scot) target_onstack =
{
	if (scot == world)
		return FALSE;
	else if (self.target1 == scot)
		return 1;
	else if (self.target2 == scot)
		return 2;
	else if (self.target3 == scot)
		return 3;
	else if (self.target4 == scot)
		return 4;
	else
		return FALSE;
};

void(entity ent) target_add =
{
	if (ent == world)
		return;
	if (target_onstack(ent))
		return;
	self.target4 = self.target3;
	self.target3 = self.target2;
	self.target2 = self.target1;
	self.target1 = ent;
};
void(entity ent) target_drop =
{

	local float tg;
	tg = target_onstack(ent);

	if (tg == 1)
	{
		self.target1 = self.target2;
		self.target2 = self.target3;
		self.target3 = self.target4;
		self.target4 = world;
	}
	else if (tg == 2)
	{
		self.target1 = self.target3;
		self.target2 = self.target4;
		self.target3 = self.target4 = world;
	}
	else if (tg == 3)
	{
		self.target1 = self.target4;
		self.target2 = self.target3 = self.target4 = world;
	}
	else if (tg == 4)
		self.target1 = self.target2 = self.target3 = self.target4 = world;
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bot_check_lost

decide if my most immediate target should be removed.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void(entity targ) bot_check_lost =
{
	local vector dist;
	dist = realorigin(targ) - self.origin;
	dist_z = 0;
	if (targ == world)
		return;
	if (self.teleport_time != self.portal_time)
	{
		self.portal_time = self.teleport_time;
		if (self.flags & FL_WATERJUMP)
			return;
		target_drop(self.target1); // don't drop targ, it may be a higher level
		return;
	}
	if (self.enemy != world)
	{
		if ((self.enemy.health <= 0) || !(self.enemy.takedamage))
		{
			target_drop(self.enemy);
			self.enemy = world;
			self.search_time = time + 5;
			return;
		}
		else if (self.search_time <= time)
		{
			target_drop(self.enemy);
			ClearMyRoute();
			self.enemy = world;
			if (self.target1 != world)
				begin_route(self.target1);
			self.search_time = time + 5;
			return;
		}
		else if (!target_onstack(self.enemy))
		{
			target_drop(self.enemy);
			self.enemy = world;
			self.search_time = time + 5;
			return;
		}
		else if (!fisible(self.enemy))
		{
			target_drop(targ);
			self.enemy = world;
			SpawnTempWaypoint(self.ldest1, 40);
			self.search_time = time + 5;
			return;
		}

	}	
	if (targ.classname == "waypoint")
	{
		if (vlen(targ.origin - self.origin) < 50)
		{
			targ.f_sound = targ.f_sound - (targ.f_sound & clientBitFlag(self.fClientNo));
			// remove it from my route
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (self.search_time <= time)
		{
			target_drop(targ);
			ClearMyRoute();
			if (self.target1 != world)
				begin_route(self.target1);
			self.search_time = time + 5;
			targ.search_time = time + 10;
			return;
		}
	}
	else if (targ.classname == "trigger_teleport")
	{
		if (self.teleport_time != self.portal_time)
		{
			target_drop(targ);
			return;
		}
		if (self.search_time <= time)
		{
			target_drop(targ);
			ClearMyRoute();
			if (self.target1 != world)
				begin_route(self.target1);
			self.search_time = time + 5;
			return;
		}
	}
	else if (targ.classname == "player")
	{
		if (targ.health <= 0)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}			
		else if ((coop) || (teamplay && targ.team == self.team))
		{
			if (targ.target1.classname == "player")
			{
				target_drop(targ);
				self.search_time = time + 5;
				return;
			}	
			else if (vlen(targ.origin - self.origin) < 128)
			{
				if (vlen(targ.origin - self.origin) < 48)
					frik_walkmove(self.origin - targ.origin);
				else
					self.keys = 0;
				self.search_time = time + 0.5;
				return;
			}
			else if (self.search_time <= time)
			{
				target_drop(targ);
				self.search_time = time + 5;
				return;
			}

		}
		else if (self.search_time <= time)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (!fisible(targ))
		{
			begin_route(targ);
			return;
		}
	}
	else if (targ.classname == "func_button")
	{
		if (targ.state != 1)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (self.search_time <= time)
		{
			target_drop(targ);
			ClearMyRoute();
			if (self.target1 != world)
				begin_route(self.target1);
			self.search_time = time + 5;
			return;
		}
	}
	else if (targ.classname == "temp_waypoint")
	{
		if (vlen(targ.origin - self.origin) < 60)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (self.search_time <= time)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (vlen(dist) < 48)
		{
			target_drop(targ);
			self.search_time = time + 5;
			targ.search_time = time + 5;
			return;
		}
	}	
	else if ((targ.solid == SOLID_TRIGGER) && (targ.movetype == MOVETYPE_NONE))
	{
		// this is of trigger_multiple parentage
		if (targ.nextthink >= time)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (self.search_time <= time)
		{
			target_drop(targ);
			ClearMyRoute();
			if (self.target1 != world)
				begin_route(self.target1);
			self.search_time = time + 5;
			return;
		}
	}	
	else if (targ.flags & FL_ITEM)
	{
		if (targ.model == string_null)
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
		else if (vlen(targ.origin - self.origin) < 60)
		{
			target_drop(targ);
			self.search_time = time + 5;
			targ.search_time = time + 5;
			return;
		}
		else if (self.search_time <= time)
		{
			target_drop(targ);
			targ.search_time = time + 5;
			self.search_time = time + 5;
			return;
		}
		else if ((targ.origin_z - self.origin_z) > 64)
		{
			if (vlen(dist) < 64)
			{
 				target_drop(targ);
				targ.search_time = time + 5;
				self.search_time = time + 5;
				return;
			}
		}
	}
	else if (targ.flags & FL_MONSTER)
	{
		if ((targ.health <= 0) || (self.search_time <= time))
		{
			target_drop(targ);
			self.search_time = time + 5;
			return;
		}
	}
	else // unknown target
	{
		target_drop(targ);
		self.search_time = time + 5;
		return;
	}	
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bot Priority Look. What a stupid name. This is where
the bot finds things it wants to kill/grab.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
// priority scale
// 0 - 10 virtually ignore, hard to see (not implemented)
// 10 - 30 normal item range
// 30 - 50 bot will consider this a target worth changing course for
// 50 - 90 bot will hunt these as vital items

// *!* Make sure you add code to bot_check_lost to remove the target *!*

float(entity thing) priority_for_thing =
{
	local float thisp;
	thisp = 0;
	// This is the most executed function in the bot. Careful what you do here.

	if ((thing.flags & FL_ITEM) && thing.model != string_null && thing.search_time < time)
	{
		thisp = 20;
		if (thing.classname == "item_artifact_super_damage")
		{
			thisp = 65;
		}
		else if (thing.classname == "item_artifact_invulnerability")
		{
			thisp = 65;
		}
		else if (thing.classname == "item_health")
		{
			if (thing.spawnflags & 2)
				thisp = 50;
			if (self.health < 40)
				thisp = thisp + 10;
		}
		else if (thing.model == "progs/armor.mdl")
		{
			if (self.armorvalue < 100)
				thisp = thisp + 25;
			if (thing.skin == 2)
				thisp = 55;
		}
		else if (thing.classname == "weapon_supershotgun")
		{
			if (!(self.items & 2)) // IT_SUPER_SHOTGUN  
				thisp = 25;
		}
		else if (thing.classname == "weapon_nailgun")
		{
			if (!(self.items & 4)) // IT_NAILGUN 
				thisp = 30;
		}
		else if (thing.classname == "weapon_supernailgun")
		{
			if (!(self.items & 8)) // IT_SUPER_NAILGUN
				thisp = 35;
		}
		else if (thing.classname == "weapon_grenadelauncher")
		{
			if (!(self.items & 16)) // IT_GRENADE_LAUNCHER
				thisp = 45;
		}
		else if (thing.classname == "weapon_rocketlauncher")
		{
			if (!(self.items & 32)) // IT_ROCKET_LAUNCHER
				thisp = 60;
		}
		else if (thing.classname == "weapon_lightning")
		{
			if (!(self.items & 64)) // IT_LIGHTNING
				thisp = 50;
		}

	}
	else if ((thing.flags & FL_MONSTER) && thing.health > 0)
		thisp = 45;
	else if (thing.classname == "player")
	{ 
		if (thing.health > 0)
		{
			thisp = 60;

			if (thing == self)
				thisp = 0;
			else
			{
				if (thing.items & IT_INVISIBILITY)
					thisp = 2;
				else if (teamplay && thing.team == self.team)
					thisp = 70;
				else if (coop)
					thisp = 80;
			}

		}
	}
	else if (thing.classname == "temp_waypoint")
	{
		thisp = 10;
	}
	if (pointcontents(thing.origin) < -3)
		thisp = 0;

	if (thisp)
		thisp = thisp + thing.priority + floor(5 * random());
	if (thing.priority > 0)
		thing.priority = 0;

	return thisp;

};
void(float scope) bot_look_for_crap =
{
	local entity foe, best;
	local float thatp, bestp, dist;
	if (self.botskill == 0)
		scope = 0;

	if (scope == 1)
		foe = findradius(self.origin, 13000);
	else
		foe = findradius(self.origin, 500);
	bestp = 1;
	while(foe)
	{
		thatp = priority_for_thing(foe);
		if (thatp)
			if (!scope)
				if (!fisible(foe))
					thatp = 0;
		if (thatp > bestp)
		{
			bestp = thatp;
			best = foe;
			dist = vlen(self.origin - foe.origin);
		} else if (thatp == bestp)
		{
			if (vlen(self.origin - foe.origin) < dist)
			{
				bestp = thatp;
				best = foe;
				dist = vlen(self.origin - foe.origin);
			}
		}
		foe = foe.chain;
	}
	if (!target_onstack(best))
	{	
		target_add(best);
		self.search_time = time + 6;
		if (scope)
			begin_route(best);
	}
};

void() bot_look_for_players =
// ugly little function to make the bot aware of players while it's doing other things
// NIXME: I hate this.
{
	if (self.enemy)
		return;
	local entity t;
	local float r;
	t = findradius(self.origin, 1800);
	while(t)
	{
		if (t.classname == "player" || t.flags & FL_MONSTER)
		if (t != self)
		if (fov(t) || t.f_sound > time || self.botskill == 3 || t.flags & FL_MONSTER)
		if (fisible(t))
		if ((coop || (teamplay && self.team == t.team)) && t.classname == "player")
		{
			if (!self.enemy)
			if (!(self.search_time > time)) // testing
			if (!t.target1.classname == "player")
			{
				target_drop(t);
				target_add(t);

			}
		}
		else
		{
			target_drop(t);
			target_add(t);
			self.enemy = t;
		}
		t = t.chain;
	}

};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bot_path

Bot will follow a route generated by the
begin_route set of functions in bot_way.qc. This code
while it works pretty well, can get confused

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void() bot_path =
{
	local entity jj, tele;
	local float damg;
	local vector org;
	bot_check_lost(self.target1);

	org = realorigin(self.target1);

	traceline(self.origin + self.view_ofs, org, FALSE, self); // check for blockage
   	if (trace_fraction != 1)
	{
		if (trace_ent.classname == "door") // a door blocks the way
		{
			if ((trace_ent.health > 0) && (self.enemy == world))
			{
				self.enemy = trace_ent;
				self.impulse = 2;
				target_add(trace_ent);
			}
			else if (trace_ent.targetname)
			{
				tele = find(world, target, trace_ent.targetname);
				if (tele.health > 0) 
				{
					self.enemy = tele;
					self.impulse = 2;
				}
				target_add(tele);
			}		
		}
	}
      else
	{
		// this seems like a hack to me
		// if this waypoint is in common with my end target, skip it
		jj = self;
		self = self.target2;
		if (FindWayPoint() == jj.target1)
		{
			self = jj;
			target_drop(self.target1);
			self.last_way = world;
			ClearMyRoute();
			return;
		}
		self = jj;
	} 

	if (target_onstack(self.last_way))
		return; // old waypoint still being hunted
	jj = FindRoute(self.last_way);
	if (jj == world)
		return;

	target_add(jj);
	self.search_time = time + 5;
	if (self.last_way != world)
	{
		if (CheckLinked(self.last_way, jj) == 2) // waypoints are telelinked
		{
			tele = FindThing("trigger_teleport"); // this is probbly the teleport responsible
			target_add(tele);

			self.portal_time = self.teleport_time;
		}
	}
	if (jj.wallhug) // was on a door when spawned
	{
		//bprint("danger! plat spawned waypoint\n");
		local entity oself;
		oself = self;
		self = jj;
		if (!frik_recognize_plat(FALSE)) // if there is nothing there now
		{
			//bprint("double danger! nothing there now!\n");
			tele = FindThing("door"); // this is likely the door responsible (crossfingers)
			self = oself;
			if (tele.targetname)
			{
				tele = find(world, target, tele.targetname);
				if (tele.health > 0) 
				{
					self.enemy = tele;
					self.impulse = 2;
				}
				target_add(tele);
			}
		}
		else
			self = oself;
	}
	self.last_way = jj;
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bot Impulse stuff. This code causes the bot to
try to select a weapon based on what he sees
on the statusbar.

You might want to edit or remove this function
if the bot does stupid things or never selects a
new weapon you put in.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

float() BotImpulse =
{
	local	float	it;
	
	it = self.items;
	if ((self.ammo_rockets >= 1) && (it & 32)) // IT_ROCKET_LAUNCHER
		return 7;
	else if (self.waterlevel <= 1 && self.ammo_cells >= 1 && (it & 64) ) // IT_LIGHTNING
		return 8;
	else if(self.ammo_nails >= 2 && (it & 8) ) // IT_SUPER_NAILGUN
		return 5;
	else if ((self.ammo_rockets >= 1) && (it & 16)) // IT_GRENADE_LAUNCHER
		return 6;
	else if(self.ammo_shells >= 2 && (it & 2) ) // IT_SUPER_SHOTGUN
		return 3;
	else if(self.ammo_nails >= 1 && (it & 4) ) // IT_NAILGUN
		return 4;
	else if(self.ammo_shells >= 1 && (it & 1) ) // IT_SHOTGUN
		return 2;
	else return 0;
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bot_fight_style, bots select an attack style
for their current weapon

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void() BotShoot = // quick little function to stop making him shoot the wrong way ! Argh

{
	local float g;
	g = angcomp(self.v_angle_x, self.b_angle_x);
	if (fabs(g) > 30)
		return; // argh, too far away
	g = angcomp(self.v_angle_y, self.b_angle_y);
	if (fabs(g) > 30)
		return; // not again!
	self.button0 = TRUE;
};
void() switch_to_close_wep =
{
	if (random() < 0.2)
	{
	 	if (self.waterlevel <= 1 && self.ammo_cells >= 1 && self.items & 64)
			self.impulse = 8; 
		else if (self.ammo_nails >= 2 && self.items & 8)
			self.impulse = 5;
	}
};
void() bot_fight_style =
{
	local vector holly, strafe;
	local float fightstyle;
	self.ldest1 = realorigin(self.enemy);

	// Implemented fight styles
	// ========================
	// 0: Run in, attack all the way (Frikbot 0.01 thru 0.03 behavior)
	// 1: Maintain a distance between 200 and 550 units, 
	//    attack in that range and further away(grenade)
	// 2: Maintain a distance between 150 and 250 units, strafe from 
	//	side to side. Attack always (nailgun, SNG and LG)
	// 3: Same as 1, except jump randomly

	if (self.weapon == 4) // IT_NAILGUN
		fightstyle = 2;
	else if (self.weapon == 8) // IT_SUPER_NAILGUN
		fightstyle = 2;
	else if (self.weapon == 16) // IT_GRENADE_LAUNCHER
		fightstyle = 1;
	else if (self.weapon == 32) // IT_ROCKET_LAUNCHER
		fightstyle = 3;
	else if (self.weapon == 64) // IT_LIGHTNING
		fightstyle = 2;
	else
		fightstyle = 0;

	self.button0 = FALSE;
	holly = realorigin(self.enemy);
	if (fightstyle == 0) // Basic fight style, move in while shooting
	{
		traceline(self.origin + self.view_ofs, holly, FALSE, self);
		if ((trace_fraction == 1) || (trace_ent == self.enemy))
		{
			frik_movetogoal();
			BotShoot();
		}
		else 
		{
			holly = normalize(holly - self.origin);
			strafe_x = holly_y * -1;
			strafe_y = holly_x;
			if (self.wallhug == 90)
				strafe = strafe * -1;

			frik_walkmove(strafe);

			if (random() <= 0.05)
				if (self.wallhug == 90)
					self.wallhug = -90;
				else
					self.wallhug = 90;
		}
	}
	else if (fightstyle == 1) // stay between 200 and 550 units away and shoot (grenade)
	{
		if (vlen(holly - self.origin) < 180)
		{
			strafe = normalize(self.origin - holly);
			frik_walkmove(strafe);

			switch_to_close_wep();
			if (self.search_time < time + 4)
				BotShoot(); // screw it
		}
		else if (vlen(holly - self.origin) > 450)
		{
			BotShoot();
			frik_movetogoal();
		} else
			BotShoot();
	}
	else if (fightstyle == 2)
	{
		BotShoot();
		if (vlen(holly - self.origin) < 90)
		{
			strafe = normalize(self.origin - holly);
			frik_walkmove(strafe);
		}
		else if (vlen(holly - self.origin) > 180)
		{
			frik_movetogoal();
		} else {
			holly = normalize(holly - self.origin);
			strafe_x = holly_y * -1;
			strafe_y = holly_x;
			if (self.wallhug == 90)
				strafe = strafe * -1;
			
			frik_walkmove(strafe);

			if (random() <= 0.05)
			{
				if (random() < 0.02)
				{
					if (self.wallhug == 90)
						self.wallhug = -90;
					else self.wallhug = 90;
				}
			}

		}
      }
	else if (fightstyle == 3) // stay between 200 and 550 units away and shoot (rocket)
	{
		if (vlen(self.enemy.origin - self.origin) < 180)
		{
			strafe = normalize(self.origin - holly);
			frik_walkmove(strafe);
			switch_to_close_wep();
			if (self.search_time < time + 4)
				BotShoot(); // screw it
		} 
		else if (vlen(self.enemy.origin - self.origin) > 450)
		{
			BotShoot();
			frik_movetogoal();
		}
		else
			BotShoot();
		
		if (random() < 0.05)
			if (random() < 0.05)
				self.button2 = TRUE;
	} 
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

The traditional bot roaming code.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void() frika_move =
{
	local float frik_test;

	makevectors(self.v_angle);

	frik_test = frik_walkmove(v_forward);

	if (random() <= 0.02)
		if (random() <= 0.05)
			frik_test = FALSE;

	if (frik_test)
		return;
	else
		self.b_angle_y = anglemod(self.b_angle_y + (self.wallhug / 3));
	self.b_angle_x = 0;

	if (random() <= 0.02)
	{
		if (self.wallhug == 90)
			self.wallhug = -90;
		else
			self.wallhug = 90;
	}

};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bot_angle_set

Same name, relatively different thing. Now it's main purpose
is to just set the bot's keys to look at things. Mostly the same
code though. Just thought you'd like to know.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


void() bot_angle_set =
{

	local float h;
	local vector view;
	
	if (self.target1 != world)
	{
		if (self.enemy != world)
		{
			if (self.missile_speed == 0)
				self.missile_speed = 1000;
			h = vlen(self.enemy.origin - self.origin) / self.missile_speed;
			view = self.enemy.velocity * h;
			view = self.enemy.origin + view;
			if (self.weapon == 32) 
				view = view - '0 0 32';
			else if (self.weapon == 64)
				view = view - '0 0 16';
			view = normalize(view - self.origin);

			if (self.enemy.origin == '0 0 0')
				view = (((self.enemy.absmin + self.enemy.absmax) * 0.5) - self.origin);
			view = vectoangles(view);
			view_x = view_x * -1;
			self.b_angle = view;
		} else {
			view = self.target1.origin;
			if (view == '0 0 0')
				view = (self.target1.absmin + self.target1.absmax) * 0.5;
			if (self.target1.flags & FL_ITEM)
				view = view + '0 0 48';
			view = view - (self.origin + self.view_ofs);
			self.b_angle = vectoangles(view);
			self.b_angle_x = self.b_angle_x * -1;
		}
	}
	else
		self.b_angle_x = 0;
	if (self.botskill == 3)
	{
		local string j;

		j = ftos(self.b_angle_x);

		self.v_angle = self.b_angle;
		while (self.v_angle_x < -180)
			self.v_angle_x = self.v_angle_x + 360;
		while (self.v_angle_x > 180)
			self.v_angle_x = self.v_angle_x - 360;

	}
	else if (self.botskill < 2) // skill 2 handled in bot_phys
	{
		if (self.b_angle_x > 180)
			self.b_angle_x = self.b_angle_x - 360;
		self.keys = self.keys - (self.keys & (KEY_LOOKLEFT + KEY_LOOKRIGHT + KEY_LOOKUP + KEY_LOOKDOWN));


		if (angcomp(self.b_angle_y, self.v_angle_y) > 10)
			self.keys = self.keys | KEY_LOOKLEFT;
		else if (angcomp(self.b_angle_y, self.v_angle_y) < -10)
			self.keys = self.keys | KEY_LOOKRIGHT;
		if (angcomp(self.b_angle_x, self.v_angle_x) < -10)
			self.keys = self.keys | KEY_LOOKUP;
		else if (angcomp(self.b_angle_x, self.v_angle_x) > 10)
			self.keys = self.keys | KEY_LOOKDOWN;
	}
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BotAI, this is the core of the bots functioning.
This is where the bot makes all decisions.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
float stagger_think;

void() BotAI =
{
	if (self.ai_time > time)
		return;
	self.ai_time = time + 0.05;
	if (BOT_COUNT > 0)
	{
		if ((time - stagger_think) < (0.1 / BOT_COUNT))
			self.ai_time = self.ai_time + 0.1 / (2 * BOT_COUNT);
	}
	else
		return;
	BotChat();
	stagger_think = time;
	self.button2 = 0;
	
	if (self.health <= 0)
	{ 
		self.button0 = floor(random() * 2);
		self.button2 = 0;
		self.keys = 0;
		ClearMyRoute();
		if (self.temp_way)
			remove(self.temp_way);
		self.target1 = self.target2 = self.target3 = self.target4 = self.enemy = world;
		self.last_way = world;
		return;
	}


	if (self.attack_finished < time) 
		if (random() < 0.2)
			self.impulse = BotImpulse();

	if (self.enemy != world)
	{
		bot_check_lost(self.target1);
		if (fisible(self.enemy))
		{
			if (!target_onstack(self.enemy))
				self.enemy = world;
			else
				bot_fight_style();
		}
		self.last_way = world; // bot doesn't know where he is
	}
	else if (self.target1)
	{
		bot_look_for_players();
		if (route_table != self)
			frik_movetogoal();
		self.button0 = 0;
		bot_path();
	}
	else
	{
		bot_look_for_players();
		if (self.failure_time < time)
		{
			bot_look_for_crap(1); // long range
		}
		else
		{

			bot_look_for_crap(0); // short range
			if (self.target1 == world) // do roaming
			{

				frika_move();
			}
		}
		self.last_way = world; // bot doesn't know where he is
		self.button0 = 0;
	}
	bot_angle_set();

	if (time > (self.air_finished - 2) && self.waterlevel > 2)
  	{
		// checks to see if bot needs to start going up for air
		traceline (self.origin, self.origin + '0 0 6800', TRUE, self);
		if (trace_inopen)	
		{
			self.keys = KEY_LOOKUP | KEY_MOVEFORWARD;
			self.button2 = TRUE; // swim!
		}
    	}
};

