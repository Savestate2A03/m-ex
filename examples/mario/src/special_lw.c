#include "mario.h"
////////////////////////
//  Inital SpecialLw  //
////////////////////////

///
/// 0x800E207C
/// Mirrors this: https://github.com/doldecomp/melee/blob/36f062411e35c2df887b21503d71f6304b4d61f4/src/melee/ft/chara/ftMario/ftMr_SpecialLw.c#L27
void SpecialLw_UpdateBoneRotation(GOBJ *gobj)
{
	Fighter_SetBoneRotX(gobj->userdata, 0, 0);
}

///
/// 0x800E207C
///
void SpecialLw(GOBJ *gobj)
{
	// get fighter data
	FighterData *fighter_data = gobj->userdata;

	// get special attributes pointer
	MarioAttr *mrAttr = fighter_data->special_attributes;

	// clear flag
	fighter_data->ftcmd_var.flag2 = 0;

	// change to special lw state and update
	ActionStateChange(0, 1, 0, gobj, STATE_SPECIALLWAIR, 0, 0);
	Fighter_AdvanceScript(gobj);

	// set initial Y velocity and clamp X velocity
	fighter_data->phys.self_vel.Y = mrAttr->specialLw_grounded_rise_resistance - mrAttr->specialLw_rising_tap_power;
	Fighter_ClampHorizontalVelocity(fighter_data, mrAttr->specialLw_base_air_speed);

	// clear flags for this state
	SpecialLwFtCmd *script_flags = (SpecialLwFtCmd *)(&fighter_data->ftcmd_var);
	script_flags->decay_air_speed = 0;
	script_flags->disable_rise = 0;

	// clear state variables
	SpecialLwVar *state_var = (SpecialLwVar *)(&fighter_data->state_var);
	state_var->air_speed = 0;
	state_var->on_ground = 0;

	// note: this variable is set but unused?
	state_var->x04 = mrAttr->specialLw_state_Var2 + 1;

	// set take damage and death callbacks
	fighter_data->cb.OnTakeDamage = SpecialLw_UpdateBoneRotation;
	fighter_data->cb.OnDeath_State = SpecialLw_UpdateBoneRotation;

	// spawn effect
	Effect_SpawnSync(VANILLA_EFFECT_DSPECIAL, gobj, gobj->hsd_object);

	// enable gfx bit flag
	fighter_data->flags.persistent_gfx = 1;

	// set endlag callbacks to pause and resume
	fighter_data->cb.EnterHitlag = Effect_PauseAll;
	fighter_data->cb.ExitHitlag = Effect_ResumeAll;
	return;
}
///
/// 0x800E2194
///
void SpecialAirLw(GOBJ *gobj)
{
	// get fighter data
	FighterData *fighter_data = gobj->userdata;

	// get special attributes pointer
	MarioAttr *mrAttr = fighter_data->special_attributes;

	// clear flag
	fighter_data->ftcmd_var.flag2 = 0;

	// change to special lw state and update
	ActionStateChange(0, 1, 0, gobj, STATE_SPECIALLWAIR, 0, 0);
	Fighter_AdvanceScript(gobj);

	// set initial Y velocity and clamp X velocity
	float y_vel = 0;

	// if this variable is set then set y vel reduction to initial rising power
	if (fighter_data->fighter_var.ft_var3 == 0)
		y_vel = mrAttr->specialLw_rising_tap_power;

	fighter_data->phys.self_vel.Y = mrAttr->specialLw_grounded_rise_resistance - y_vel;

	// clamp vertical velocity
	Fighter_ClampHorizontalVelocity(fighter_data, mrAttr->specialLw_base_air_speed);

	// clear flags
	SpecialLwFtCmd *script_flags = (SpecialLwFtCmd *)(&fighter_data->ftcmd_var);
	script_flags->decay_air_speed = 0;
	script_flags->disable_rise = 0;

	// clear state variables
	SpecialLwVar *state_var = (SpecialLwVar *)(&fighter_data->state_var);
	state_var->air_speed = 0;
	state_var->on_ground = 0;

	// note: this variable is set but unused?
	state_var->x04 = mrAttr->specialLw_state_Var2 + 1;

	// set take damage and death callbacks
	fighter_data->cb.OnTakeDamage = SpecialLw_UpdateBoneRotation;
	fighter_data->cb.OnDeath_State = SpecialLw_UpdateBoneRotation;

	// spawn effect
	Effect_SpawnSync(VANILLA_EFFECT_DSPECIAL, gobj, gobj->hsd_object);

	// enable gfx bit flag
	fighter_data->flags.persistent_gfx = 1;

	// set endlag callbacks to pause and resume
	fighter_data->cb.EnterHitlag = Effect_PauseAll;
	fighter_data->cb.ExitHitlag = Effect_ResumeAll;
	return;
}
////////////////////////
// Grounded SpecialLw //
////////////////////////
///
/// 0x800E22BC
///
void SpecialLw_AnimationCallback(GOBJ *gobj)
{
	FighterData *fighter_data = gobj->userdata;

	if (FrameTimerCheck(gobj) == 0)
	{
		fighter_data->cb.OnTakeDamage = NULL;
		fighter_data->cb.OnDeath_State = NULL;
		Fighter_EnterWait(gobj);
	}
	return;
}
///
/// 0x800E23DC
///
void SpecialLw_IASACallback(GOBJ *gobj)
{
	return;
}
///
/// 0x800E23E4
///
void SpecialLw_PhysicCallback(GOBJ *gobj)
{
	FighterData *fighter_data = gobj->userdata;
	MarioAttr *mrAttr = fighter_data->special_attributes;
	SpecialLwFtCmd *script_flags = (SpecialLwFtCmd *)(&fighter_data->ftcmd_var);

	float base_air_speed = mrAttr->specialLw_base_air_speed;
	float airSpeed = base_air_speed;

	SpecialLwVar *state_var = (SpecialLwVar *)(&fighter_data->state_var);

	if (script_flags->decay_air_speed != 0)
	{
		float new_air_speed = state_var->air_speed - mrAttr->speicalLw_air_speed_decel;

		state_var->air_speed = new_air_speed;

		airSpeed = base_air_speed + new_air_speed;

		if (airSpeed < 0)
			airSpeed = 0;
	}

	Fighter_QueueAllowXDrift(fighter_data, 0, mrAttr->specialLw_x_accel, airSpeed);

	Fighter_PhysGround_ApplyVelocity(gobj);

	if ((fighter_data->ftcmd_var.flag2 != 0) && ((fighter_data->input.down & HSD_BUTTON_B) != 0))
	{
		fighter_data->phys.self_vel.Y = fighter_data->phys.self_vel.Y + mrAttr->specialLw_rising_tap_power;

		fighter_data->ftcmd_var.flag2 = 0;

		Fighter_SetAirborne(fighter_data);

		ActionStateChange(fighter_data->state.frame, 1, 0, gobj, STATE_SPECIALLWAIR, 0xc4c508a, 0);

		Fighter_ClampFallSpeed(fighter_data, mrAttr->specialLw_terminal_velocity);

		Fighter_ClampHorizontalVelocity(fighter_data, mrAttr->specialLw_x_vel_clamp);

		fighter_data->cb.EnterHitlag = Effect_PauseAll;
		fighter_data->cb.ExitHitlag = Effect_ResumeAll;
	}
	return;
}
///
/// 0x80082888
///
int SpecialLw_UpdateECBBox(GOBJ *gobj, ECBSize *ecb_bones)
{
	FighterData *fighter_data = gobj->userdata;

	fighter_data->coll_data.topN_Prev.X = fighter_data->coll_data.topN_Curr.X;
	fighter_data->coll_data.topN_Prev.Y = fighter_data->coll_data.topN_Curr.Y;
	fighter_data->coll_data.topN_Prev.Z = fighter_data->coll_data.topN_Curr.Z;

	fighter_data->coll_data.topN_Curr.X = fighter_data->phys.pos.X;
	fighter_data->coll_data.topN_Curr.Y = fighter_data->phys.pos.Y;
	fighter_data->coll_data.topN_Curr.Z = fighter_data->phys.pos.Z;

	if (fighter_data->facing_direction < 0)
	{
		ecb_bones->right.X = -ecb_bones->right.X;
		ecb_bones->left.X = -ecb_bones->left.X;
	}

	int is_touching_ground = ECB_CollGround_PassLedge(&fighter_data->coll_data, ecb_bones);

	fighter_data->phys.pos.X = fighter_data->coll_data.topN_Curr.X;
	fighter_data->phys.pos.Y = fighter_data->coll_data.topN_Curr.Y;
	fighter_data->phys.pos.Z = fighter_data->coll_data.topN_Curr.Z;

	return is_touching_ground;
}
///
/// 0x800E25C4
///
void SpecialLw_CollisionCallback(GOBJ *gobj)
{
	FighterData *fighter_data = gobj->userdata;
	MarioAttr *mrAttr = fighter_data->special_attributes;
	SpecialLwVar *state_var = (SpecialLwVar *)(&fighter_data->state_var);

	ECBSize fake_ecb = {12, 0, -6, 6, 6, 6};

	if (fighter_data->phys.air_state == 0)
	{
		if (SpecialLw_UpdateECBBox(gobj, &fake_ecb) == 0)
		{
			fighter_data->ftcmd_var.flag2 = 0;
			Fighter_SetAirborne(fighter_data);
			ActionStateChange(fighter_data->state.frame, 1, 0, gobj, STATE_SPECIALLWAIR, 0xc4c508a, 0);
			Fighter_ClampFallSpeed(fighter_data, mrAttr->specialLw_terminal_velocity);
			Fighter_ClampHorizontalVelocity(fighter_data, mrAttr->specialLw_x_vel_clamp);
			fighter_data->cb.EnterHitlag = Effect_PauseAll;
			fighter_data->cb.ExitHitlag = Effect_ResumeAll;
			state_var->on_ground = 0;
		}
		else
		{
			state_var->on_ground = 1;
		}
	}
	else
	{
		if (Fighter_CollAir_DefineECB(gobj, &fake_ecb) == 0)
		{
			fighter_data->ftcmd_var.flag2 = 0;
			Fighter_SetAirborne(fighter_data);
			ActionStateChange(fighter_data->state.frame, 1, 0, gobj, STATE_SPECIALLWAIR, 0xc4c508a, 0);
			Fighter_ClampFallSpeed(fighter_data, mrAttr->specialLw_terminal_velocity);
			Fighter_ClampHorizontalVelocity(fighter_data, mrAttr->specialLw_x_vel_clamp);
			fighter_data->cb.EnterHitlag = Effect_PauseAll;
			fighter_data->cb.ExitHitlag = Effect_ResumeAll;
			state_var->on_ground = 0;
		}
		else
		{
			state_var->on_ground = 1;
		}
	}

	if ((fighter_data->ftcmd_var.flag3 == 0) || (state_var->on_ground == 0))
	{
		// Reset Rotation if not grounded
		Fighter_SetBoneRotZ(fighter_data, 0, 0);
	}
	else
	{
		// Rotate to ground slope angle
		float angle = atan2(fighter_data->coll_data.ground_slope.X, fighter_data->coll_data.ground_slope.Y);
		Fighter_SetBoneRotZ(fighter_data, 0, -1 * angle);
	}
	return;
}
////////////////////////
//  Aerial SpecialLw  //
////////////////////////
///
/// 0x800E2308
///
void SpecialAirLw_AnimationCallback(GOBJ *gobj)
{
	FighterData *fighter_data = gobj->userdata;
	MarioAttr *mrAttr = fighter_data->special_attributes;
	SpecialLwFtCmd *script_flags = (SpecialLwFtCmd *)(&fighter_data->ftcmd_var);

	if (script_flags->disable_rise != 0)
	{
		script_flags->disable_rise = 0;
		fighter_data->fighter_var.ft_var3 = 1;
	}

	if (FrameTimerCheck(gobj) == 0)
	{
		fighter_data->cb.OnTakeDamage = NULL;
		fighter_data->cb.OnDeath_State = NULL;

		if (mrAttr->specialLw_freefall_toggle == 0)
		{
			Fighter_EnterFall(gobj);
		}
		else
		{
			Fighter_EnterSpecialFall(gobj, 1, 0, 1, 1, mrAttr->specialLw_freefall_toggle);
		}
	}
	return;
}
///
/// 0x800E23E0
///
void SpecialAirLw_IASACallback(GOBJ *gobj)
{
	return;
}
///
/// 0x800E2508
///
void SpecialAirLw_PhysicCallback(GOBJ *gobj)
{
	FighterData *fighter_data = gobj->userdata;
	MarioAttr *mrAttr = fighter_data->special_attributes;
	SpecialLwVar *state_var = (SpecialLwVar *)(&fighter_data->state_var);
	SpecialLwFtCmd *script_flags = (SpecialLwFtCmd *)(&fighter_data->ftcmd_var);

	if (fighter_data->fighter_var.ft_var3 == 0)
	{
		if ((fighter_data->ftcmd_var.flag2 != 0) && ((fighter_data->input.down & HSD_BUTTON_B) != 0))
		{
			Fighter_AddClampYPosition(fighter_data, mrAttr->specialLw_rising_tap_power, mrAttr->specialLw_terminal_velocity);
		}
	}

	Fighter_Phys_ApplyVerticalAirFriction(fighter_data);

	float base_air_speed = mrAttr->specialLw_base_air_speed;
	float airSpeed = base_air_speed;

	if (script_flags->decay_air_speed != 0)
	{
		float new_air_speed = state_var->air_speed - mrAttr->speicalLw_air_speed_decel;

		state_var->air_speed = new_air_speed;

		airSpeed = base_air_speed + new_air_speed;

		if (airSpeed < 0)
			airSpeed = 0;
	}

	Fighter_AllowXDrift(fighter_data, 0, mrAttr->specialLw_x_drift, airSpeed);
	return;
}
///
/// 0x800E2778
///
void SpecialAirLw_CollisionCallback(GOBJ *gobj)
{
	FighterData *fighter_data = gobj->userdata;
	MarioAttr *mrAttr = fighter_data->special_attributes;
	SpecialLwVar *state_var = (SpecialLwVar *)(&fighter_data->state_var);

	ECBSize fake_ecb = {12, 0, -6, 6, 6, 6};

	if (Fighter_CollAir_DefineECB(gobj, &fake_ecb) == 0)
	{
		state_var->on_ground = 0;
	}
	else
	{
		fighter_data->ftcmd_var.flag2 = 0;

		Fighter_SetGrounded2(fighter_data);

		fighter_data->phys.self_vel.Y = 0;
		fighter_data->fighter_var.ft_var3 = 0;

		ActionStateChange(fighter_data->state.frame, 1, 0, gobj, STATE_SPECIALLW, 0xc4c508a, 0);

		Fighter_ClampHorizontalGroundVelocity(fighter_data, mrAttr->specialLw_base_air_speed);

		fighter_data->cb.EnterHitlag = Effect_PauseAll;
		fighter_data->cb.ExitHitlag = Effect_ResumeAll;
		state_var->on_ground = 1;
	}

	// set rotation
	if ((fighter_data->ftcmd_var.flag3 == 0) || (state_var->on_ground == 0))
	{
		// if in air reset rotation
		Fighter_SetBoneRotZ(fighter_data, 0, 0);
	}
	else
	{
		// if on ground rotate to ground slope
		float angle = atan2(fighter_data->coll_data.ground_slope.X, fighter_data->coll_data.ground_slope.Y);
		Fighter_SetBoneRotZ(fighter_data, 0, -1 * angle);
	}
	return;
}