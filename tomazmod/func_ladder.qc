/*
===============
fun_ladder.qc
by Jballou
jballou@quakemods.net
note from Koolio: i took this from the FBI mod
with permission off course then changed it to actually WORK
=================
*/
void()  LadderCheck =
{
        local float     dist;
        local float     grav;
        local vector    XY_vel;
        local vector    angles1;

        grav = cvar ("sv_gravity");
        XY_vel = self.velocity;

        if ((XY_vel_x) || (XY_vel_y))
        {
                self.velocity = XY_vel * 0.3;
                if (self.angles_x > 10)
                {

                        dist = (self.LadderOrigin_z + (self.LadderSize_z * 0.5)) - (self.origin_z + self.view_ofs_z);
                        if (dist < 0)
                        {
                                angles1 = vectoangles (self.LadderOrigin - self.origin);
                                makevectors (angles1);
                                self.velocity = v_forward * 50;
                                self.velocity_z = grav * 0.5;
                        }
                        else
                                self.velocity_z = grav * 0.4;
                }

                if (self.angles_x < -10)
                        self.velocity_z = grav * -0.4;
        }
        else
                self.velocity_z = grav * frametime;
};


void()  LadderFieldTouch =
{        
        local float     facing;
        local float     facing1, facing2;

        // Determine angle player would be facing to be looking to look
        // DIRECTLY at the ladder
        facing = vectoyaw ((self.absmin + (self.size * 0.5)) - other.origin);

        // Compare to the player's actual yaw.
        // NB: Take two checks to account for the fact that the difference
        // between 4 degrees and 358 is actually only 6. I can't think of
        // a better way right now.
        facing1 = fabs(facing - other.angles_y);
        facing2 = fabs(facing - (other.angles_y + 360));

        if (facing1 < facing2)
                facing = facing1;
        else
                facing = facing2;

        if ((other.flags & FL_CLIENT) && (facing < 45))
        {
                other.onladder = TRUE;
                other.LadderSize = self.size;
                other.LadderOrigin = self.absmin + (self.size * 0.5);
                other.flags = other.flags | FL_ONLADDER;
        }
};

/*QUAKED func_ladder (0 .5 .8)
To use a ladder, face it and then look up or down and press forward to go
up or down the ladder.. simple!
*/
void() func_ladder =
{        
//        self.movetype = MOVETYPE_NONE;
      //  self.solid = SOLID_TRIGGER;
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setmodel (self, self.model);	// set size and link into world
	self.model = "";
       // SpawnLadderField ();
	self.touch = LadderFieldTouch;

};
