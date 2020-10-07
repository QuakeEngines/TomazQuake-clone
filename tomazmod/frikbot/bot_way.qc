 /***********************************************
*                                              *
*             FrikBot Waypoints                *
*         "The better than roaming AI"         *
*                                              *
***********************************************/

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Waypoint Linking code

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


float (entity e1, entity e2) CheckLinked =
{
	if ((e1 == e2) || (e2 == world) || (e1 == world))
		return FALSE;
	else if (e1.target1 == e2)
	{
		if (e1.chat_time & 1)
			return 2;
		else return TRUE;
	}
	else if (e1.target2 == e2)
	{
		if (e1.chat_time & 2)
			return 2;
		else return TRUE;
	}
	else if (e1.target3 == e2)
	{
		if (e1.chat_time & 4)
			return 2;
		else return TRUE;
	}
	else if (e1.target4 == e2)
	{
		if (e1.chat_time & 8)
			return 2;
		else return TRUE;
	}

	else return FALSE;
};


float (entity e1, entity e2) LinkWays =
{
	if ((e1 == e2) || (e2 == world) || (e1 == world))
		return FALSE;
	else if (CheckLinked(e1, e2))
		return FALSE; // already linked!!!

	if (e1.target1 == world)
	{
		e1.target1 = e2;
		return TRUE;
	}
	else if (e1.target2 == world)
	{
		e1.target2 = e2;
		return TRUE;
	}
	else if (e1.target3 == world)
	{
		e1.target3 = e2;
                return TRUE;
	}
	else if (e1.target4 == world)
	{
		e1.target4 = e2;
		return TRUE;
	}
	else return FALSE;

};
// Link Ways part 2, used only for teleporters

float (entity e1, entity e2) TeleLinkWays =
{
	if ((e1 == e2) || (e2 == world) || (e1 == world))
		return FALSE;
	else if (CheckLinked(e1, e2))
		return FALSE; // already linked!!!

	if (e1.target1 == world)
	{
		e1.target1 = e2;
                e1.chat_time = e1.chat_time | 1;
		return TRUE;
	}
	else if (e1.target2 == world)
	{
		e1.target2 = e2;
                e1.chat_time = e1.chat_time | 2;
		return TRUE;
	}
	else if (e1.target3 == world)
	{
		e1.target3 = e2;
                e1.chat_time = e1.chat_time | 4;
            return TRUE;
	}
	else if (e1.target4 == world)
	{
		e1.target4 = e2;
                e1.chat_time = e1.chat_time | 8;
		return TRUE;
	}
	else return FALSE;

};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Temporary Marker code

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void() TempWay_Remove =
{
	local entity oself;
	oself = self;
	self = self.goalentity;
	target_drop(oself);
	self = oself;
};

void(vector org, float prior) SpawnTempWaypoint =
{	
	local entity tep;

	if (self.temp_way.classname == "temp_waypoint")
		tep = self.temp_way;
	else
		tep = spawn();
	self.temp_way = tep;
	tep.classname = "temp_waypoint";
	tep.search_time = 0;
	tep.think = TempWay_Remove;
	tep.nextthink = time + 7;
	tep.goalentity = self;
	tep.solid = SOLID_TRIGGER;
	tep.movetype = MOVETYPE_NONE;
	tep.priority = prior;
	tep.search_time = 0;
	setorigin(tep, org);
	target_add(tep);
	self.search_time = time + 6;
	
	setsize(tep, VEC_HULL_MIN, VEC_HULL_MAX); // FIXME: convert these to numerical
	//setmodel (tep, "progs/h_player.mdl"); // DEBUG
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Waypoint Spawning Code

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

// This is used quite a bit, by many different functions
// big lag causer

entity() FindWayPoint =// return closest, fisible waypoint
{
	local entity t;
	local float dst, tdst;
	local entity best;
	t = findradius(self.origin, 800); // I'm now officially cheating
	dst = 100000;
	best = world;
	while(t != world)
	{
		if (t.classname == "waypoint")
		{
			tdst = vlen(t.origin - self.origin);
			if (tdst < dst)
			{
                                if (fisible(t))
				{
					dst = tdst;
					best = t;
				}
			}
		}
                t = t.chain;
	} 
	return best;
};

entity(vector org) make_waypoint = 
{
	local entity point;
	point = spawn();
	point.classname = "waypoint";
	point.search_time = time; // don't double back for me;
	point.solid = SOLID_TRIGGER;
	point.movetype = MOVETYPE_NONE;
	setorigin(point, org);
	
	setsize(point, VEC_HULL_MIN, VEC_HULL_MAX); // FIXME: convert these to numerical

	WAYPOINTS = WAYPOINTS + 1;
	//bprint(ftos(WAYPOINTS));
	//bprint("\n");
	point.count = WAYPOINTS;
	if (cvar("developer") == 2)
		setmodel(point, "progs/s_bubble.spr"); // DEBUG
	return point;
};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Dynamic Waypoint spawning and linking. Not very good
all things considered.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

void() DynamicWaypoint =
{
	if (WAYPOINT_MODE != 1)
		return;
	else if (fMaxClients < 2) // don't waypoint in SP
		return;
	else if (self.health <= 0)
	{
		self.last_way = world;
		return;
	}
	else if ((self.teleport_time > self.portal_time) && !(self.flags & FL_WATERJUMP))
	{
		self.chat_time = 2;
		self.portal_time = self.teleport_time;
	}
	else if (!(self.flags & FL_ONGROUND) && (self.waterlevel < 2) && !self.chat_time)
		self.chat_time = 1; // on real humans chat time is used to flag
					// that reverse links shouldn't be created
	if (self.ai_time > time)
		return;
	else if (!((self.flags & FL_ONGROUND) || self.waterlevel == 3))
		return;

	self.ai_time = time + 0.1;

	local entity newp;
	
	newp = FindWayPoint();
	self.ldest1 = self.origin + self.view_ofs;
	if(newp != world)
      {
		if (vlen(newp.origin - self.origin) < 192)
		{
			if (vlen(newp.origin - self.origin) < 64)
			{
				if (newp != self.last_way)
				{
					if (!self.chat_time)
						LinkWays(newp, self.last_way);
					if (self.chat_time == 2)
						TeleLinkWays(self.last_way, newp);
					else if (wisible(self.last_way))
						LinkWays(self.last_way, newp);
				
				}
				self.last_way = newp;
				self.chat_time = 0;
			}
			if (cvar("developer") == 2)
			{
				if (newp.target1 != world)
					DeveloperLightning(newp, newp.target1);
				if (newp.target2 != world)
					DeveloperLightning(newp, newp.target2);
				if (newp.target3 != world)
					DeveloperLightning(newp, newp.target3);
				if (newp.target4 != world)
					DeveloperLightning(newp, newp.target4);
			}
			return;
			
		}
	}
	if (frik_recognize_plat(FALSE)) 
	{
		if (vlen(trace_ent.velocity) > 0)
		{
			if (self.wallhug)
				return;
			self.wallhug = TRUE;
			//bprint("on a plat!!!!!\n");
		}
		else
			self.wallhug = FALSE;
	}
	else
		self.wallhug = FALSE;

	newp = make_waypoint(self.ldest1);
	if (!wisible(newp))
		setorigin(newp, self.origin + self.view_ofs);
	else
	{
		if (!self.chat_time)
			LinkWays(newp, self.last_way);
		if (self.chat_time == 2)
			TeleLinkWays(self.last_way, newp);
		else if (wisible(self.last_way))
			LinkWays(self.last_way, newp);
		self.last_way = newp;
		self.chat_time = 0;
	}
	local entity oself;
	oself = self;
	self = newp;
	if (frik_recognize_plat(FALSE))
	{
		if (trace_ent.classname == "door")
			newp.wallhug = TRUE;
	}
	self = oself;

};

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Waypoint Loading from file

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void() ClearAllWays =
{

	local entity t;
	t= find(world,classname, "waypoint");
	while(t != world)
	{
		remove(t);
		t = find(t, classname, "waypoint");
	}
	WAYPOINTS = 0;
};
void() FixNumbers =
{
	local entity t;
	WAYPOINTS = 0;

	t= find(world,classname, "waypoint");
	while(t != world)
	{
		WAYPOINTS = WAYPOINTS + 1;
		t.count = WAYPOINTS;
		t = find(t, classname, "waypoint");
	}

};
entity(float num) WaypointForNum =
{
	local entity t;
	if (num == 0)
		return world;

	t = find(world, classname, "waypoint");
	while (t != world)
	{
		if (t.count == num)
			return t;
		t = find(t, classname, "waypoint");
	}
	return world;
};

void() FixThisWaypoint = 
{
	self.enemy.target1 = WaypointForNum(self.enemy.missile_speed);
	self.enemy.target2 = WaypointForNum(self.enemy.chat_subj);
	self.enemy.target3 = WaypointForNum(self.enemy.keys);
	self.enemy.target4 = WaypointForNum(self.enemy.tfrags);
	self.enemy = find(self.enemy, classname, "waypoint");
	self.nextthink = time + 0.01;
	if (self.enemy == world)
	{
		remove(self);
		fixer = world;
	}
};
void() FixWaypoints =
{
	if (!fixer)
	{
		fixer = spawn();
		fixer.nextthink = time + 0.01;
		fixer.think = FixThisWaypoint;
		fixer.enemy = find(world, classname, "waypoint");
	}


};


/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

FindRoute & FindThing used by the pathing code in bot_ai.qc

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

entity route_table;

entity(string s) FindThing =
{
	local entity t;
	local float tdst, dst;
	local entity best;
	dst = 100000;
        best = world;
        t = find (world, classname, s);
	while (t != world)
	{
		tdst = vlen(((t.absmin + t.absmax) * 0.5) - self.origin);
            if (tdst < dst)
		{
			dst = tdst;
			best = t;
		} 
		t = find(t, classname, s);
	}
	return best;
};



// FindRoute, this is a key function in the pathing.
// The name is a bit misleading, this code finds the
// closest waypoint that is part of a route calculated
// by the begin_route and end_route routines
// This is a definite path to an object.

entity(entity lastone) FindRoute =
{
	// kinda like FindWaypoint, only of this bots route though
	local entity t, best;
	local float dst, tdst, flag, flag2;
	flag = clientBitFlag(self.fClientNo);
	t = find(world, classname, "waypoint");
	dst = 100000;
	best = world;
	flag2 = FALSE;
	while(t != world)
	{
		tdst = vlen(t.origin - self.origin);
            if ((tdst < dst) &&  (t.f_sound & flag))
		{
			flag2 = TRUE;
			if ((lastone == world) || (CheckLinked(lastone, t))) // hack
			{
				dst = tdst;
				best = t;
			}
		} 
		t = find(t, classname, "waypoint");
	} // -- this causes some trouble with the original code, FIXME  --
	if ((best == world) && !flag2)
	{
		// no waypoints were found on his route
		// use my new less-robust pathing code :-|

		// find a route with no precalculation...sorta
		if (self.target1 == world || self.target1.classname == "waypoint")
			return world;
		dst = vlen(realorigin(self.target1) - self.origin);
		best = world;
		if (self.last_way)
			lastone = self.last_way;
		else
			lastone = FindWayPoint();

		t = find(world, classname, "waypoint");
		while(t != world)
		{
			tdst = vlen(t.origin - self.target1.origin);
			if (tdst < dst)
			{
				if (t.search_time < time)
				{
					if (CheckLinked(lastone, t)) 
					{
						if (fisible(t))
						{
							best = t;
							dst = tdst;
						}
					}
				}
			}
			t = find(t, classname, "waypoint");
		}
		if (best)
			best.search_time = time + 5;

	}
	return best;
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Route & path table management

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void() ClearRouteTable =
{
	// cleans up route table

	local entity t;
	t = find(world, classname, "waypoint");
	while (t != world)
	{
		t.enemy = world;
		t.items = -1; // not in table
		t = find(t, classname, "waypoint");
	}
};
void() ClearMyRoute =
{
	local float flag;
	flag = clientBitFlag(self.fClientNo);

	// removes all route markers of self
	local entity t;
	t = find(world, classname, "waypoint");
	while (t != world)
	{
		t.f_sound = t.f_sound - (t.f_sound & flag);
		t = find(t, classname, "waypoint");
	}
};
float() TableIsBusy =
{
	if (route_table == world)
		return FALSE;
	else if (!clientIsActive(route_table.fClientNo))
		return FALSE;
	else
		return TRUE;
};
/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Actual routing code. end_route and begin_route
will mark out the shortest path from object A to
object B utilizing the waypoint map of the level.

Also accounts for 'telelinks' between two waypoints

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void() end_route =
{
	local entity t;
	local float dist;

	if (route_table == world)
		return;
	self = route_table;
	route_table = world; // set the table as free
        t = FindWayPoint();
	if (t.enemy == world)
	{
		if (self.target1)
		{
			self.target1.search_time = time + 5;
			self.failure_time = time + 1;
			target_drop(self.target1);
		}
		return;
	}

	dist = clientBitFlag(self.fClientNo);

	while(t) // mark the path
	{
		if (t.f_sound & dist)
		{
			t = world;
			return;
		}
		t.f_sound = t.f_sound | dist;
		t = t.enemy; // back track to item
	}  
};

void() WaypointThink =
{
	local entity t;
	local float dist, flag, link;
	if (self.items == -1)
		return; // somebody messed up!
	flag = TRUE;
        t = find(world, classname, "waypoint");

	while(t != world)
	{
		if (t != self)
		{
			link = CheckLinked(t, self);
			if (link)
			{
				if (link != 2)
					dist = vlen(self.origin - t.origin) + self.items;
				else
					dist = self.items;
				if ((dist < t.items) || (t.items == -1))
				{
					t.items = dist;
					t.enemy = self;
					t.think = WaypointThink;
					t.nextthink = time + (frametime * 2);
					flag = FALSE;
				}
			} 
			if (t.nextthink != 0)
				flag = FALSE;
		}

		t = find(t, classname, "waypoint");
	}

	// if flag is false, the waypoint table is still being parsed
	if (flag)
	{
                self.think = end_route;
		self.nextthink = time;
	}
	
};

void(entity this) begin_route =
{
	local entity bot, that;
	if (this == world)
		return;

	ClearMyRoute();
	if (TableIsBusy())
	{
		target_drop(this);
		self.failure_time = time + 2;
		this.search_time = time + 2;
		return;
	}
	route_table = self;
	ClearRouteTable();
	bot = self;
      self = this;
	that = FindWayPoint();
	if (that != world)
	{
      	that.enemy = this;
      	that.items = vlen(that.origin - this.origin);
		that.nextthink = time;
		that.think = WaypointThink;
	}
	else 
	{
		route_table = world;
		self = bot;
		target_drop(this);
		this.search_time = time + 10;
		self.failure_time = time + 1;
	}
	self = bot;
};
