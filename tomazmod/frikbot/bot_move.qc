
/***********************************************
*                                              *
*            FrikBot Movement AI               *
*       "The not so great movement AI"         *
*                                              *
***********************************************/
float(float flag) frik_recognize_plat =
{
	if ((self.classname != "waypoint") && !(self.flags & FL_ONGROUND))
		return FALSE;
	traceline(self.origin, self.origin - '0 0 64', TRUE, self);
	if (trace_ent != world)
	{
		if (flag) // afect bot movement too
		{
			if (self.keys & KEY_MOVEUP)
			{
				if (trace_ent.velocity_z > 0)
					self.keys = self.keys - (self.keys & 63); // 63 is all movement keys
			} else if (self.keys & KEY_MOVEDOWN)
			{
				if (trace_ent.velocity_z < 0)
					self.keys = self.keys - (self.keys & 63);
			}
		}
		return TRUE;
	}
	else
		return FALSE;

 
};
float(vector sdir) frik_KeysForDir =
{

	local vector keydir;
	local float outkeys, tang;
	outkeys = 0;
	if (sdir_x || sdir_y)
	{
		// Everything is tested against 60 degrees,
		// this allows the bot to overlap the keys
		// 30 degrees on each diagonal  45 degrees 
		// might look more realistic

 		keydir = vectoangles(sdir);
		tang = angcomp(keydir_y, self.v_angle_y);
		if ((tang <= 150) && (tang >= 30))
			outkeys = outkeys + KEY_MOVELEFT;
		else if ((tang >= -150) && (tang <= -30))
			outkeys = outkeys + KEY_MOVERIGHT;
		if (fabs(tang) <= 60)
			outkeys = outkeys + KEY_MOVEFORWARD;
		else if (fabs(tang) >= 120)
			outkeys = outkeys + KEY_MOVEBACK;
	}
	if (sdir_z > 0.7)
		outkeys = outkeys + KEY_MOVEUP;
	else if (sdir_z < 0.7)
		outkeys = outkeys + KEY_MOVEDOWN;
	return outkeys;

};


void(vector whichway, float danger) frik_obstructed =
{
	local vector disway, org;
	local float dist;


	if (self.obs_time > time)
		return;
	if (danger)
	{
		target_drop(self.target1);
		self.search_time = time + 1;
		// do something?
	}

	if ((vlen(self.velocity) < 30) && !danger)
	{
		disway_x = whichway_y * -1;
		disway_y = whichway_x;
		if (self.target1)
		{
			dist = vlen(self.target1.origin - (self.origin + disway * 10));
			if (dist > vlen(self.target1.origin - self.origin))
				disway = disway * -1;
		}
		self.obs_dir = disway;
		self.obs_time = time + 0.5;
		return;
	} 			
	self.obs_dir = '0 0 0' - whichway;
	self.obs_time = time + 0.05;
	return;
};
void() frik_obstacles =
{
	if (!(self.flags & FL_ONGROUND))
		return;
	local vector start, stop, ang;
	local float test, conts, dist, hgt;
	// Check to see if I can jump over something in my way
	ang = normalize(self.velocity);
	ang_z = 0;
	start = self.origin + ang * 32; // ahem
	start_z = self.origin_z + self.maxs_z;
	stop = start;
	stop_z = self.origin_z + self.mins_z;
	traceline(start, stop - '0 0 256', TRUE, self);
	if (trace_allsolid || trace_startsolid)
		return;
	hgt = trace_endpos_z - stop_z;

	if (hgt > 18)
	{
		self.button2 = TRUE;
		return;
	}
	else if (hgt <= -64) // this might be a gap
	{
		conts = pointcontents(trace_endpos + '0 0 4');
		start = stop - '0 0 8';
		stop = start + ang *  256;
		traceline(start, stop, TRUE, self);
		test = vlen(trace_endpos - start);
		if (test <= 20)
			return; // it's a walkable gap, do nothing
		ang_x = self.velocity_y * -1;
		ang_y = self.velocity_x;
		ang = normalize(ang);
		traceline(start - (ang * 10), start + (ang * 10), TRUE, self);
		if ((trace_fraction != 1) || trace_startsolid)
			return; // gap is only 20 wide, walkable	
		ang = self.velocity;
		ang_z = 0;
                dist = ((540 / sv_gravity) * vlen(ang))/* + 32*/;
		if (test > dist) // I can't make it
		{
			if (conts < -3) // bad stuff down dare
			{
				self.obs_time = 0;
				frik_obstructed(ang, TRUE);
				return;
			}
			else
			{
				if (self.target1)
				{
					stop = realorigin(self.target1);
					if ((stop_z - self.origin_z) < -32)
						return; // safe to fall
				}
				self.obs_time = 0;
				frik_obstructed(ang, TRUE);
				return;
			}
		} else {
			ang = normalize(ang);
			//look for a ledge
			traceline(self.origin, self.origin + (ang * (test + 20)), TRUE, self); 
			if (trace_fraction != 1)
			{
				if (conts < -3) // bad stuff down dare
				{
					self.obs_time = 0;
					frik_obstructed(ang, TRUE);
					return;
				}
				else
				{
					if (self.target1)
					{
						stop = realorigin(self.target1);
						if ((stop_z - self.origin_z) < -32)
							return; // safe to fall
					}
					self.obs_time = 0;
					frik_obstructed(ang, TRUE);
					return;
				}
			}	
		}
		self.button2 = TRUE; // go for it, dude!
		self.obs_dir = ang;
		self.obs_time = time + 1;
	} 
	else if (pointcontents(trace_endpos + '0 0 4') < -3)
	{
		self.obs_time = 0;
		frik_obstructed(ang, TRUE);
		return;
	}
};

void() frik_movetogoal =
{
	if (self.target1 == world)
	{
		makevectors(self.v_angle);
		frik_walkmove(v_forward);
		return;
	}
	local vector way, start, stop, ang;
	way = realorigin(self.target1) - self.origin;
	if (vlen(way) < 48)
		return;
	way = normalize(way);
	self.keys = self.keys - (self.keys & 63) + frik_KeysForDir(way);
	frik_obstacles();
	if (!frik_recognize_plat(TRUE))
		if (time < self.obs_time)
			self.keys = self.keys - (self.keys & 63) + frik_KeysForDir(self.obs_dir);

};
float(vector weird) frik_walkmove =
{
	// okay so it's not walkmove
	// sue me
 	self.keys = self.keys - (self.keys & 63) + frik_KeysForDir(weird);
	frik_obstacles();
	frik_recognize_plat(TRUE);
	if (time < self.obs_time)
	{
		self.keys = self.keys - (self.keys & 63) + frik_KeysForDir(self.obs_dir);
		return FALSE;
	} else
		return TRUE;
};

