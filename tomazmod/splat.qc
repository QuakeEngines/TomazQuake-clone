float() crandom; //function prototype for crandom

//SplatThink: Controls behavior of splats after they have hit the wall
void() SplatThink =
{
        if ( (self.attack_finished <= time) ||
             (self.flags & FL_ONGROUND) )
        {
                remove(self); //remove if: time "runs out" or on ground
                return;
        }
         self.nextthink = time + 0.2;
};

//SplatTouch: Called(by the engine) when splats touch the world or an entity
//after being spawned
void() SplatTouch =
{
        if ( (other != world) ||
             (pointcontents(self.origin) <= -3) ||
             (self.flags & FL_ONGROUND) )
        {
                remove(self); //remove if: didn't hit wall, in liquid, or on ground
                return;
        }
        self.velocity = self.avelocity = '0 0 0'; //stop moving and spinning
        self.movetype = MOVETYPE_FLY; //changed to remove effect of gravity
        self.touch = SUB_Null; //don't call this (touch) function again
        self.attack_finished = time + 20 + (2*random()); //set random "time limit"

        self.think = SplatThink;
        self.nextthink = time + 0.2;
};

//ThrowBloodSplat: This will create a blood splat at "org", moving in
//direction "dir", and owned by "own"
void(vector dir, vector org, entity own) ThrowBloodSplat =
{
        local entity splat;
        local vector dir;
   
        if ( !((own.flags & FL_MONSTER) ||
               (own.classname == "player")) )
                return; //only monsters and players should create splats!

        splat = spawn();
        splat.owner = own; //move through hit monster/player
        splat.movetype = MOVETYPE_TOSS; //gravity with no bouncing
        splat.solid = SOLID_BBOX; //does not move through other entities (besides owner)
  
        dir = normalize(dir); //make sure "dir" has length 1
        splat.velocity = dir * (900 + 100*random()); //random velocity in direction of shot
        splat.velocity_x = splat.velocity_x + crandom()*80; //randomize x velocity (+/- 80)
        splat.velocity_y = splat.velocity_y + crandom()*80; //randomize y velocity (+/- 80)
        splat.velocity_z = splat.velocity_z + 240 + 100*random(); //randomize z velocity (+ 240-340)
        splat.touch = SplatTouch;
	
        splat.nextthink = time + 5;
        splat.think = SUB_Remove;

//	  setmodel (splat, "sprites/!bloodsplatter.spr32");
        setsize (splat, '0 0 0', '0 0 0');     
        setorigin (splat, org); //start splat at point of damage
};