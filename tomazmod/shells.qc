float () crandom; // prototype 

void () fade_shell =
{
	if (DP_ENT_ALPHA)
	{
		if (!self.alpha)
			self.alpha = 1;

		self.alpha = self.alpha - 0.05;
	
		if (self.alpha < 0.06)
			self.think = SUB_Remove;
		else
			self.think = fade_shell;
		self.nextthink = time + 0.1;
	}
	else
		self.think = SUB_Remove;
};

//--------------------------------------------------------------------
// Displays shell and defines its dynamic manifestation 
//--------------------------------------------------------------------
void() DropShell = 
{
self.solid = SOLID_BBOX; 
setmodel (self, "progs/casing_shell.mdl"); 
setsize (self, VEC_ORIGIN, VEC_ORIGIN); 
self.angles = self.owner.angles;
setorigin (self, self.owner.origin); 
self.velocity_x = crandom()*30 - crandom()*30;
self.velocity_y = crandom()*30 - crandom()*30;
self.velocity_z = 150 + crandom()*100; 
self.avelocity_y = crandom()*500; 
self.nextthink = time + 4; 
self.think = fade_shell; 
}; 


//--------------------------------------------------------------------
// Spawns new shell entity but doesn't display it until reloadtime
//--------------------------------------------------------------------
void() SpawnShell= 
{ 
local entity shell; 

shell = spawn ();
shell.movetype = MOVETYPE_BOUNCE;
shell.owner = self;
shell.angles = self.angles;
shell.nextthink = time + 0.5; // delay shells until reload
shell.think = DropShell;
}; 

//--------------------------------------------------------------------
// Spawns new nail entity
//--------------------------------------------------------------------
void() SpawnNail= 
{ 
local entity nail; 

nail = spawn ();
nail.movetype = MOVETYPE_BOUNCE;
nail.solid = SOLID_BBOX;
setmodel (nail, "progs/casing_steel.mdl");
setsize (nail, VEC_ORIGIN, VEC_ORIGIN);
setorigin (nail, self.origin);

nail.angles = self.angles;
nail.velocity_x = crandom()*30 - crandom()*30;
nail.velocity_y = crandom()*30 - crandom()*30;
nail.velocity_z = 150 + crandom()*100; 
nail.avelocity_y = crandom()*500; 
nail.nextthink = time + 4; 
nail.think = fade_shell; 
}; 

//--------------------------------------------------------------------
// Spawns new supernail entity
//--------------------------------------------------------------------
void() SpawnSuperNail= 
{ 
local entity supernail; 

supernail = spawn ();
supernail.movetype = MOVETYPE_BOUNCE;
supernail.solid = SOLID_BBOX;
setmodel (supernail, "progs/casing_bronze.mdl");
setsize (supernail, VEC_ORIGIN, VEC_ORIGIN);
setorigin (supernail, self.origin);

supernail.angles = self.angles;
supernail.velocity_x = crandom()*30 - crandom()*30;
supernail.velocity_y = crandom()*30 - crandom()*30;
supernail.velocity_z = 150 + crandom()*100; 
supernail.avelocity_y = crandom()*500; 
supernail.nextthink = time + 4; 
supernail.think = fade_shell; 
};