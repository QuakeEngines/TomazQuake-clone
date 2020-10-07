/*
######
#Rain#
######
*/

// ***********
// RAIN EFFECT
// ***********

void () func_rain = 
{
	if (TQ_RAIN)
	{
		setmodel(self, self.model);
		self.modelindex = 0;
		self.classname = "func_rain";

		if (!self.drops)
			self.drops = 10;

		WriteByte  (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte  (MSG_BROADCAST, TE_RAIN);
		WriteCoord (MSG_BROADCAST, self.absmin_x);
		WriteCoord (MSG_BROADCAST, self.absmin_y);
		WriteCoord (MSG_BROADCAST, self.absmin_z);
		WriteCoord (MSG_BROADCAST, self.absmax_x);
		WriteCoord (MSG_BROADCAST, self.absmax_y);
		WriteCoord (MSG_BROADCAST, self.absmax_z);
		WriteShort (MSG_BROADCAST, self.drops);

		self.nextthink = time + 0.1;
		self.think = func_rain;
	}
};

/*
######
#Snow#
######
*/

void () func_snow = 
{
	if (TQ_SNOW)
	{
		setmodel(self, self.model);
		self.modelindex = 0;
		self.classname = "func_snow";

		if (!self.flakes)
			self.flakes = 10;

		WriteByte  (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte  (MSG_BROADCAST, TE_SNOW);
		WriteCoord (MSG_BROADCAST, self.absmin_x);
		WriteCoord (MSG_BROADCAST, self.absmin_y);
		WriteCoord (MSG_BROADCAST, self.absmin_z);
		WriteCoord (MSG_BROADCAST, self.absmax_x);
		WriteCoord (MSG_BROADCAST, self.absmax_y);
		WriteCoord (MSG_BROADCAST, self.absmax_z);
		WriteShort (MSG_BROADCAST, self.flakes);

   		self.nextthink = time + 0.1;
   		self.think = func_snow;
	}
};

/*
###########
#Lightning#
###########
*/
void () lightning_Attack;

//##############################

void () func_lightning =
{
	setmodel(self, self.model);
	self.modelindex = 0;
	precache_sound ("weapons/lstart.wav");
	precache_model("progs/flame2.mdl");
	self.classname = "func_lightning";
	self.nextthink = (time + RandomRange (0, 10));
	self.think = lightning_Attack;
};

//##############################

void(vector org) MakeFlame =		// Creates an invisible model at the base of the lightning which glows
{
local entity	flame;

	org_z = org_z + 5;

	flame = spawn();
	flame.classname = "flame";
	flame.movetype = MOVETYPE_FLY;
	setmodel (flame, "progs/flame2.mdl");
	flame.frame = 1;
	flame.effects = flame.effects | EF_DIMLIGHT;
	flame.solid = SOLID_BBOX;
	flame.angles = '0 0 0';
	setorigin (flame, org);
	flame.nextthink = time + 6; // Remove self after 3 seconds.
	flame.think = SUB_Remove;
};

//##############################

void () lightning_Attack =
{
	local vector org;
	local vector targ;
	local vector org2;
	
	sound (self, CHAN_VOICE, "weapons/lstart.wav", TRUE, ATTN_NORM);
      
	org = (self.absmin + self.absmax) * 0.5;
	org_x = RandomRange(self.absmin_x, self.absmax_x);
	org_y = RandomRange(self.absmin_y, self.absmax_y);
    
	org2 = self.origin;
	org2_z = org2_z - 10000;

	traceline (org, org2, TRUE, self);

	targ = (org - '0 0 256');

	targ_z = trace_endpos_z;

	targ_x = (targ_x + RandomRange (32, -32));
	targ_y = (targ_y + RandomRange (32, -32));

	MakeFlame (targ);	

	self.nextthink = (time + RandomRange (1, 10));
	self.think = lightning_Attack;
	LightningDamage (org, targ, self, 10);
	WriteByte (MSG_ALL, SVC_TEMPENTITY);
	WriteByte (MSG_ALL, TE_LIGHTNING2);
	WriteEntity (MSG_ALL, world);
	WriteCoord (MSG_ALL, org_x);
	WriteCoord (MSG_ALL, org_y);
	WriteCoord (MSG_ALL, org_z);
	WriteCoord (MSG_ALL, targ_x);
	WriteCoord (MSG_ALL, targ_y);
	WriteCoord (MSG_ALL, targ_z);
};