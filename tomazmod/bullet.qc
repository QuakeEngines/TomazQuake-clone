void () fade_bullethole =
{
	if (DP_ENT_ALPHA)
	{
		self.alpha = self.alpha - 0.05;
	
		if (self.alpha < 0.06)
			self.think = SUB_Remove;
		else
			self.think = fade_bullethole;
		self.nextthink = time + 0.1;
	}
	else
		self.think = SUB_Remove;
};

void (vector pos) placebullethole = 
{
	local entity new;
	local vector norm;
	
	new = spawn ();
	new.owner = new;
	new.alpha = 0.99;
	new.movetype = MOVETYPE_NONE;
	new.solid = SOLID_NOT;
	new.classname = "bullethole";
	setmodel (new, "progs/bullethole.spr");
	setsize (new,VEC_ORIGIN,VEC_ORIGIN);
	norm = trace_plane_normal;
	norm_x = (FALSE - norm_x);
	norm_y = (FALSE - norm_y);
	new.angles = vectoangles (norm);
	makevectors (self.angles);
	setorigin (new,(pos - (v_forward * 0.2) + (v_up * 0.2)));
	if (pointcontents(new.origin) == CONTENT_SKY)
	{
		remove(new);
		return;
	}
	new.think = fade_bullethole;
	new.nextthink = (time + 4.0);
};