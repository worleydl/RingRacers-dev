// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2011-2016 by Matthew "Inuyasha" Walsh.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  k_profiles.h
/// \brief Control profiles definition

#ifndef __PROFILES_H__
#define __PROFILES_H__

#include "doomdef.h"		// MAXPLAYERNAME
//#include "r_skins.h"		// SKINNAMESIZE	// This cuases stupid issues.
#include "g_input.h"		// Input related stuff
#include "string.h"			// strcpy etc
#include "g_game.h"			// game CVs

// We have to redefine this because somehow including r_skins.h causes a redefinition of node_t since that's used for both net nodes and BSP nodes too......
// And honestly I don't wanna refactor that.
#define SKINNAMESIZE 16

#define PROFILENAMELEN 6
#define PROFILEVER 0
#define MAXPROFILES 16
#define PROFILESFILE "kartprofiles.cfg"

#define PROFILEDEFAULTNAME "GUEST"
#define PROFILEDEFAULTPNAME "Player"
#define PROFILEDEFAULTSKIN "sonic"
#define PROFILEDEFAULTCOLOR SKINCOLOR_SAPPHIRE
#define PROFILEDEFAULTFOLLOWER "none"
#define PROFILEDEFAULTFOLLOWERCOLOR 255

// Man I wish I had more than 16 friends!!

// profile_t definition (WIP)
typedef struct profile_s 
{
	
	// Versionning
	UINT8 version;						// Version of the profile, this can be useful for backwards compatibility reading if we ever update the profile structure/format after release.

	// Profile header
	char profilename[PROFILENAMELEN+1];	// Profile name (not to be confused with player name)
	
	// Player data
	char playername[MAXPLAYERNAME+1];	// Player name
	UINT16 color;						// Default player coloUr. ...But for consistency we'll name it color.
	char follower[SKINNAMESIZE+1];		// Follower
	UINT16 followercolor;				// Follower color
	
	// Player-specific consvars.
	// @TODO: List all of those
	boolean kickstartaccel;				// cv_kickstartaccel
	
	// Finally, control data itself
	INT32 controls[num_gamecontrols][MAXINPUTMAPPING];	// Lists of all the controls, defined the same way as default inputs in g_input.c
} profile_t;


// Functions

// PR_MakeProfile
// Makes a profile from the supplied profile name, player name, colour, follower, followercolour and controls.
// The consvar values are left untouched.
profile_t PR_MakeProfile(const char *prname, const char *pname, const UINT16 col, const char *fname, UINT16 fcol, INT32 controlarray[num_gamecontrols][MAXINPUTMAPPING]);

// PR_MakeProfileFromPlayer
// Makes a profile_t from the supplied profile name, player name, colour, follower and followercolour.
// The last argument is a player number to read cvars from; as for convenience, cvars will be set directly when making a profile (since loading another one will overwrite them, this will be inconsequential)
profile_t PR_MakeProfileFromPlayer(const char *prname, const char *pname, const UINT16 col, const char *fname, UINT16 fcol, UINT8 pnum);

// PR_AddProfile(profile_t p)
// Adds a profile to profilesList and increments numprofiles.
// Returns true if succesful, false if not.
boolean PR_AddProfile(profile_t p);

// PR_SaveProfiles(void)
// Saves all the profiles in profiles.cfg
// This does not save profilesList[0] since that's always going to be the default profile.
void PR_SaveProfiles(void);

// PR_LoadProfiles(void)
// Loads all the profiles saved in profiles.cfg.
// This also loads 
void PR_LoadProfiles(void);



#endif