// DR. ROBOTNIK'S RING RACERS
//-----------------------------------------------------------------------------
// Copyright (C) 2022 by Kart Krew
// Copyright (C) 2022 by Sally "TehRealSalt" Cochenour
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  k_roulette.c
/// \brief Item roulette code.

#include "k_roulette.h"

#include "d_player.h"
#include "doomdef.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
#include "p_slopes.h"
#include "p_setup.h"
#include "r_draw.h"
#include "r_local.h"
#include "r_things.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "z_zone.h"
#include "m_misc.h"
#include "m_cond.h"
#include "f_finale.h"
#include "lua_hud.h"	// For Lua hud checks
#include "lua_hook.h"	// For MobjDamage and ShouldDamage
#include "m_cheat.h"	// objectplacing
#include "p_spec.h"

#include "k_kart.h"
#include "k_battle.h"
#include "k_boss.h"
#include "k_pwrlv.h"
#include "k_color.h"
#include "k_respawn.h"
#include "k_waypoint.h"
#include "k_bot.h"
#include "k_hud.h"
#include "k_terrain.h"
#include "k_director.h"
#include "k_collide.h"
#include "k_follower.h"
#include "k_objects.h"
#include "k_grandprix.h"
#include "k_specialstage.h"

// Magic number distance for use with item roulette tiers
#define DISTVAR (2048)

// Distance when SPB can start appearing
#define SPBSTARTDIST (8*DISTVAR)

// Distance when SPB is forced onto the next person who rolls an item
#define SPBFORCEDIST (16*DISTVAR)

// Distance when the game stops giving you bananas
#define ENDDIST (14*DISTVAR)

// Consistent seed used for item reels
#define ITEM_REEL_SEED (0x22D5FAA8)

#define FRANTIC_ITEM_SCALE (FRACUNIT*6/5)

#define ROULETTE_SPEED_SLOWEST (20)
#define ROULETTE_SPEED_FASTEST (2)
#define ROULETTE_SPEED_DIST (150*DISTVAR)
#define ROULETTE_SPEED_TIMEATTACK (9)

static UINT8 K_KartItemOddsRace[NUMKARTRESULTS-1][8] =
{
	{ 0, 0, 2, 3, 4, 0, 0, 0 }, // Sneaker
	{ 0, 0, 0, 0, 0, 3, 4, 5 }, // Rocket Sneaker
	{ 0, 0, 0, 0, 2, 5, 5, 7 }, // Invincibility
	{ 2, 3, 1, 0, 0, 0, 0, 0 }, // Banana
	{ 1, 2, 0, 0, 0, 0, 0, 0 }, // Eggman Monitor
	{ 5, 5, 2, 2, 0, 0, 0, 0 }, // Orbinaut
	{ 0, 4, 2, 1, 0, 0, 0, 0 }, // Jawz
	{ 0, 3, 3, 2, 0, 0, 0, 0 }, // Mine
	{ 3, 0, 0, 0, 0, 0, 0, 0 }, // Land Mine
	{ 0, 0, 2, 2, 0, 0, 0, 0 }, // Ballhog
	{ 0, 0, 0, 0, 0, 2, 4, 0 }, // Self-Propelled Bomb
	{ 0, 0, 0, 0, 2, 5, 0, 0 }, // Grow
	{ 0, 0, 0, 0, 0, 2, 4, 2 }, // Shrink
	{ 1, 0, 0, 0, 0, 0, 0, 0 }, // Lightning Shield
	{ 0, 1, 2, 1, 0, 0, 0, 0 }, // Bubble Shield
	{ 0, 0, 0, 0, 0, 1, 3, 5 }, // Flame Shield
	{ 3, 0, 0, 0, 0, 0, 0, 0 }, // Hyudoro
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Pogo Spring
	{ 2, 1, 1, 0, 0, 0, 0, 0 }, // Super Ring
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Kitchen Sink
	{ 3, 0, 0, 0, 0, 0, 0, 0 }, // Drop Target
	{ 0, 0, 0, 3, 5, 0, 0, 0 }, // Garden Top
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Gachabom
	{ 0, 0, 2, 2, 2, 0, 0, 0 }, // Sneaker x2
	{ 0, 0, 0, 0, 4, 4, 4, 0 }, // Sneaker x3
	{ 0, 1, 1, 0, 0, 0, 0, 0 }, // Banana x3
	{ 0, 0, 1, 0, 0, 0, 0, 0 }, // Orbinaut x3
	{ 0, 0, 0, 2, 0, 0, 0, 0 }, // Orbinaut x4
	{ 0, 0, 1, 2, 1, 0, 0, 0 }, // Jawz x2
	{ 0, 0, 0, 0, 0, 0, 0, 0 }  // Gachabom x3
};

static UINT8 K_KartItemOddsBattle[NUMKARTRESULTS-1][2] =
{
	//K  L
	{ 2, 1 }, // Sneaker
	{ 0, 0 }, // Rocket Sneaker
	{ 4, 1 }, // Invincibility
	{ 0, 0 }, // Banana
	{ 1, 0 }, // Eggman Monitor
	{ 8, 0 }, // Orbinaut
	{ 8, 1 }, // Jawz
	{ 6, 1 }, // Mine
	{ 2, 0 }, // Land Mine
	{ 2, 1 }, // Ballhog
	{ 0, 0 }, // Self-Propelled Bomb
	{ 2, 1 }, // Grow
	{ 0, 0 }, // Shrink
	{ 4, 0 }, // Lightning Shield
	{ 1, 0 }, // Bubble Shield
	{ 1, 0 }, // Flame Shield
	{ 2, 0 }, // Hyudoro
	{ 3, 0 }, // Pogo Spring
	{ 0, 0 }, // Super Ring
	{ 0, 0 }, // Kitchen Sink
	{ 2, 0 }, // Drop Target
	{ 4, 0 }, // Garden Top
	{ 0, 0 }, // Gachabom
	{ 0, 0 }, // Sneaker x2
	{ 0, 1 }, // Sneaker x3
	{ 0, 0 }, // Banana x3
	{ 2, 0 }, // Orbinaut x3
	{ 1, 1 }, // Orbinaut x4
	{ 5, 1 }, // Jawz x2
	{ 0, 0 }  // Gachabom x3
};

static kartitems_t K_KartItemReelTimeAttack[] =
{
	KITEM_SNEAKER,
	KITEM_SUPERRING,
	KITEM_NONE
};

static kartitems_t K_KartItemReelBreakTheCapsules[] =
{
	KRITEM_TRIPLEORBINAUT,
	KITEM_BANANA,
	KITEM_NONE
};

static kartitems_t K_KartItemReelBoss[] =
{
	KITEM_ORBINAUT,
	KITEM_BANANA,
	KITEM_NONE
};

/*--------------------------------------------------
	boolean K_ItemEnabled(kartitems_t item)

		See header file for description.
--------------------------------------------------*/
boolean K_ItemEnabled(kartitems_t item)
{
	if (item < 1 || item >= NUMKARTRESULTS)
	{
		// Not a real item.
		return false;
	}

	if (K_CanChangeRules(true) == false)
	{
		// Force all items to be enabled.
		return true;
	}

	// Allow the user preference.
	return cv_items[item - 1].value;
}

/*--------------------------------------------------
	boolean K_ItemSingularity(kartitems_t item)

		See header file for description.
--------------------------------------------------*/
boolean K_ItemSingularity(kartitems_t item)
{
	switch (item)
	{
		case KITEM_SPB:
		case KITEM_SHRINK:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}
}

/*--------------------------------------------------
	static fixed_t K_ItemOddsScale(UINT8 playerCount)

		A multiplier for odds and distances to scale
		them with the player count.

	Input Arguments:-
		playerCount - Number of players in the game.

	Return:-
		Fixed point number, to multiply odds or
		distances by.
--------------------------------------------------*/
static fixed_t K_ItemOddsScale(UINT8 playerCount)
{
	const UINT8 basePlayer = 8; // The player count we design most of the game around.
	fixed_t playerScaling = 0;

	if (playerCount < 2)
	{
		// Cap to 1v1 scaling
		playerCount = 2;
	}

	// Then, it multiplies it further if the player count isn't equal to basePlayer.
	// This is done to make low player count races more interesting and high player count rates more fair.
	if (playerCount < basePlayer)
	{
		// Less than basePlayer: increase odds significantly.
		// 2P: x2.5
		playerScaling = (basePlayer - playerCount) * (FRACUNIT / 4);
	}
	else if (playerCount > basePlayer)
	{
		// More than basePlayer: reduce odds slightly.
		// 16P: x0.75
		playerScaling = (basePlayer - playerCount) * (FRACUNIT / 32);
	}

	return playerScaling;
}

/*--------------------------------------------------
	static UINT32 K_UndoMapScaling(UINT32 distance)

		Takes a raw map distance and adjusts it to
		be in x1 scale.

	Input Arguments:-
		distance - Original distance.

	Return:-
		Distance unscaled by mapobjectscale.
--------------------------------------------------*/
static UINT32 K_UndoMapScaling(UINT32 distance)
{
	if (mapobjectscale != FRACUNIT)
	{
		// Bring back to normal scale.
		return FixedDiv(distance, mapobjectscale);
	}

	return distance;
}

/*--------------------------------------------------
	static UINT32 K_ScaleItemDistance(UINT32 distance, UINT8 numPlayers)

		Adjust item distance for lobby-size scaling
		as well as Frantic Items.

	Input Arguments:-
		distance - Original distance.
		numPlayers - Number of players in the game.

	Return:-
		New distance after scaling.
--------------------------------------------------*/
static UINT32 K_ScaleItemDistance(UINT32 distance, UINT8 numPlayers)
{
	if (franticitems == true)
	{
		// Frantic items pretends everyone's farther apart, for crazier items.
		distance = FixedMul(distance, FRANTIC_ITEM_SCALE);
	}

	// Items get crazier with the fewer players that you have.
	distance = FixedMul(
		distance,
		FRACUNIT + (K_ItemOddsScale(numPlayers) / 2)
	);

	return distance;
}

/*--------------------------------------------------
	static UINT32 K_GetItemRouletteDistance(const player_t *player, UINT8 numPlayers)

		Gets a player's distance used for the item
		roulette, including all scaling factors.

	Input Arguments:-
		player - The player to get the distance of.
		numPlayers - Number of players in the game.

	Return:-
		The player's finalized item distance.
--------------------------------------------------*/
static UINT32 K_GetItemRouletteDistance(const player_t *player, UINT8 numPlayers)
{
	UINT32 pdis = 0;

	if (player == NULL)
	{
		return 0;
	}

#if 0
	if (specialStage.active == true)
	{
		UINT32 ufoDis = K_GetSpecialUFODistance();

		if (player->distancetofinish <= ufoDis)
		{
			// You're ahead of the UFO.
			pdis = 0;
		}
		else
		{
			// Subtract the UFO's distance from your distance!
			pdis = player->distancetofinish - ufoDis;
		}
	}
	else
#endif
	{
		UINT8 i;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator
				&& players[i].position == 1)
			{
				// This player is first! Yay!

				if (player->distancetofinish <= players[i].distancetofinish)
				{
					// Guess you're in first / tied for first?
					pdis = 0;
				}
				else
				{
					// Subtract 1st's distance from your distance, to get your distance from 1st!
					pdis = player->distancetofinish - players[i].distancetofinish;
				}
				break;
			}
		}
	}

	pdis = K_UndoMapScaling(pdis);
	pdis = K_ScaleItemDistance(pdis, numPlayers);

	if (player->bot && player->botvars.rival)
	{
		// Rival has better odds :)
		pdis = FixedMul(pdis, FRANTIC_ITEM_SCALE);
	}

	return pdis;
}

/*--------------------------------------------------
	INT32 K_KartGetItemOdds(const player_t *player, itemroulette_t *const roulette, UINT8 pos, kartitems_t item)

		See header file for description.
--------------------------------------------------*/
INT32 K_KartGetItemOdds(const player_t *player, itemroulette_t *const roulette, UINT8 pos, kartitems_t item)
{
	boolean bot = false;
	boolean rival = false;
	UINT8 position = 0;

	INT32 shieldType = KSHIELD_NONE;

	boolean powerItem = false;
	boolean cooldownOnStart = false;
	boolean notNearEnd = false;

	fixed_t newOdds = 0;
	size_t i;

	I_Assert(roulette != NULL);

	I_Assert(item > KITEM_NONE); // too many off by one scenarioes.
	I_Assert(item < NUMKARTRESULTS);

	if (player != NULL)
	{
		bot = player->bot;
		rival = (bot == true && player->botvars.rival == true);
		position = player->position;
	}

	if (K_ItemEnabled(item) == false)
	{
		return 0;
	}

	if (K_GetItemCooldown(item) > 0)
	{
		// Cooldown is still running, don't give another.
		return 0;
	}

	/*
	if (bot)
	{
		// TODO: Item use on bots should all be passed-in functions.
		// Instead of manually inserting these, it should return 0
		// for any items without an item use function supplied

		switch (item)
		{
			case KITEM_SNEAKER:
				break;
			default:
				return 0;
		}
	}
	*/
	(void)bot;

	shieldType = K_GetShieldFromItem(item);
	switch (shieldType)
	{
		case KSHIELD_NONE:
			/* Marble Garden Top is not REALLY
				a Sonic 3 shield */
		case KSHIELD_TOP:
		{
			break;
		}

		default:
		{
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (playeringame[i] == false || players[i].spectator == true)
				{
					continue;
				}

				if (shieldType == K_GetShieldFromItem(players[i].itemtype))
				{
					// Don't allow more than one of each shield type at a time
					return 0;
				}
			}
		}
	}

	if (gametype == GT_BATTLE)
	{
		I_Assert(pos < 2); // DO NOT allow positions past the bounds of the table
		newOdds = K_KartItemOddsBattle[item-1][pos];
	}
	else
	{
		I_Assert(pos < 8); // Ditto
		newOdds = K_KartItemOddsRace[item-1][pos];
	}

	newOdds <<= FRACBITS;

	switch (item)
	{
		case KITEM_BANANA:
		case KITEM_EGGMAN:
		case KITEM_SUPERRING:
		{
			notNearEnd = true;
			break;
		}

		case KITEM_ROCKETSNEAKER:
		case KITEM_JAWZ:
		case KITEM_LANDMINE:
		case KITEM_DROPTARGET:
		case KITEM_BALLHOG:
		case KRITEM_TRIPLESNEAKER:
		case KRITEM_TRIPLEORBINAUT:
		case KRITEM_QUADORBINAUT:
		case KRITEM_DUALJAWZ:
		{
			powerItem = true;
			break;
		}

		case KITEM_HYUDORO:
		case KRITEM_TRIPLEBANANA:
		{
			powerItem = true;
			notNearEnd = true;
			break;
		}

		case KITEM_INVINCIBILITY:
		case KITEM_MINE:
		case KITEM_GROW:
		case KITEM_BUBBLESHIELD:
		case KITEM_FLAMESHIELD:
		{
			cooldownOnStart = true;
			powerItem = true;
			break;
		}

		case KITEM_SPB:
		{
			cooldownOnStart = true;
			notNearEnd = true;

			if ((gametyperules & GTR_CIRCUIT) == 0)
			{
				// Needs to be a race.
				return 0;
			}

			if (roulette->firstDist < ENDDIST*2 // No SPB when 1st is almost done
				|| position == 1) // No SPB for 1st ever
			{
				return 0;
			}
			else
			{
				const UINT32 dist = max(0, ((signed)roulette->secondToFirst) - SPBSTARTDIST);
				const UINT32 distRange = SPBFORCEDIST - SPBSTARTDIST;
				const fixed_t maxOdds = 20 << FRACBITS;
				fixed_t multiplier = FixedDiv(dist, distRange);

				if (multiplier < 0)
				{
					multiplier = 0;
				}

				if (multiplier > FRACUNIT)
				{
					multiplier = FRACUNIT;
				}

				newOdds = FixedMul(maxOdds, multiplier);
			}
			break;
		}

		case KITEM_SHRINK:
		{
			cooldownOnStart = true;
			powerItem = true;
			notNearEnd = true;

			if (roulette->playing - 1 <= roulette->exiting)
			{
				return 0;
			}
			break;
		}

		case KITEM_LIGHTNINGSHIELD:
		{
			cooldownOnStart = true;
			powerItem = true;

			if (spbplace != -1)
			{
				return 0;
			}
			break;
		}

		default:
		{
			break;
		}
	}

	if (newOdds == 0)
	{
		// Nothing else we want to do with odds matters at this point :p
		return newOdds;
	}

	if ((cooldownOnStart == true) && (leveltime < (30*TICRATE)+starttime))
	{
		// This item should not appear at the beginning of a race. (Usually really powerful crowd-breaking items)
		newOdds = 0;
	}
	else if ((notNearEnd == true) && (roulette->baseDist < ENDDIST))
	{
		// This item should not appear at the end of a race. (Usually trap items that lose their effectiveness)
		newOdds = 0;
	}
	else if (powerItem == true)
	{
		// This item is a "power item". This activates "frantic item" toggle related functionality.
		if (franticitems == true)
		{
			// First, power items multiply their odds by 2 if frantic items are on; easy-peasy.
			newOdds *= 2;
		}

		if (rival == true)
		{
			// The Rival bot gets frantic-like items, also :p
			newOdds *= 2;
		}

		newOdds = FixedMul(newOdds, FRACUNIT + K_ItemOddsScale(roulette->playing));
	}

	newOdds = FixedInt(FixedRound(newOdds));
	return newOdds;
}

/*--------------------------------------------------
	static UINT8 K_FindUseodds(const player_t *player, itemroulette_t *const roulette)

		Gets which item bracket the player is in.
		This can be adjusted depending on which
		items being turned off.

	Input Arguments:-
		player - The player the roulette is for.
		roulette - The item roulette data.

	Return:-
		The item bracket the player is in, as an
		index to the array.
--------------------------------------------------*/
static UINT8 K_FindUseodds(const player_t *player, itemroulette_t *const roulette)
{
	UINT8 i;
	UINT8 useOdds = 0;
	UINT8 distTable[14];
	UINT8 distLen = 0;
	UINT8 totalSize = 0;
	boolean oddsValid[8];

	for (i = 0; i < 8; i++)
	{
		UINT8 j;

		if (gametype == GT_BATTLE && i > 1)
		{
			oddsValid[i] = false;
			continue;
		}

		for (j = 1; j < NUMKARTRESULTS; j++)
		{
			if (K_KartGetItemOdds(player, roulette, i, j) > 0)
			{
				break;
			}
		}

		oddsValid[i] = (j < NUMKARTRESULTS);
	}

#define SETUPDISTTABLE(odds, num) \
	totalSize += num; \
	if (oddsValid[odds]) \
		for (i = num; i; --i) \
			distTable[distLen++] = odds;

	if (gametype == GT_BATTLE) // Battle Mode
	{
		useOdds = 0;
	}
	else
	{
		SETUPDISTTABLE(0,1);
		SETUPDISTTABLE(1,1);
		SETUPDISTTABLE(2,1);
		SETUPDISTTABLE(3,2);
		SETUPDISTTABLE(4,2);
		SETUPDISTTABLE(5,3);
		SETUPDISTTABLE(6,3);
		SETUPDISTTABLE(7,1);

		for (i = 0; i < totalSize; i++)
		{
			fixed_t pos = 0;
			fixed_t dist = 0;
			UINT8 index = 0;

			if (i == totalSize-1)
			{
				useOdds = distTable[distLen - 1];
				break;
			}

			pos = ((i << FRACBITS) * distLen) / totalSize;
			dist = FixedMul(DISTVAR << FRACBITS, pos) >> FRACBITS;
			index = FixedInt(FixedRound(pos));

			if (roulette->dist <= (unsigned)dist)
			{
				useOdds = distTable[index];
				break;
			}
		}
	}

#undef SETUPDISTTABLE

	return useOdds;
}

/*--------------------------------------------------
	static boolean K_ForcedSPB(const player_t *player, itemroulette_t *const roulette)

		Determines special conditions where we want
		to forcefully give the player an SPB.

	Input Arguments:-
		player - The player the roulette is for.
		roulette - The item roulette data.

	Return:-
		true if we want to give the player a forced SPB,
		otherwise false.
--------------------------------------------------*/
static boolean K_ForcedSPB(const player_t *player, itemroulette_t *const roulette)
{
	if (K_ItemEnabled(KITEM_SPB) == false)
	{
		return false;
	}

	if (!(gametyperules & GTR_CIRCUIT))
	{
		return false;
	}

	if (player == NULL)
	{
		return false;
	}

	if (player->position <= 1)
	{
		return false;
	}

	if (spbplace != -1)
	{
		return false;
	}

	if (itemCooldowns[KITEM_SPB - 1] > 0)
	{
		return false;
	}

#if 0
	if (roulette->playing <= 2)
	{
		return false;
	}
#endif

	return (roulette->secondToFirst >= SPBFORCEDIST);
}

/*--------------------------------------------------
	static void K_InitRoulette(itemroulette_t *const roulette)

		Initializes the data for a new item roulette.

	Input Arguments:-
		roulette - The item roulette data to initialize.

	Return:-
		N/A
--------------------------------------------------*/
static void K_InitRoulette(itemroulette_t *const roulette)
{
	size_t i;

#ifndef ITEM_LIST_SIZE
	if (roulette->itemList == NULL)
	{
		roulette->itemListCap = 8;
		roulette->itemList = Z_Calloc(
			sizeof(SINT8) * roulette->itemListCap,
			PU_STATIC,
			&roulette->itemList
		);

		if (roulette->itemList == NULL)
		{
			I_Error("Not enough memory for item roulette list\n");
		}
	}
#endif

	roulette->itemListLen = 0;
	roulette->index = 0;

	roulette->useOdds = UINT8_MAX;
	roulette->baseDist = roulette->dist = 0;
	roulette->playing = roulette->exiting = 0;
	roulette->firstDist = roulette->secondDist = UINT32_MAX;
	roulette->secondToFirst = 0;

	roulette->elapsed = 0;
	roulette->tics = roulette->speed = ROULETTE_SPEED_TIMEATTACK; // Some default speed

	roulette->active = true;
	roulette->eggman = false;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] == false || players[i].spectator == true)
		{
			continue;
		}

		roulette->playing++;

		if (players[i].exiting)
		{
			roulette->exiting++;
		}

		if (players[i].position == 1)
		{
			roulette->firstDist = K_UndoMapScaling(players[i].distancetofinish);
		}

		if (players[i].position == 2)
		{
			roulette->secondDist = K_UndoMapScaling(players[i].distancetofinish);
		}
	}

	// Calculate 2nd's distance from 1st, for SPB
	if (roulette->firstDist != UINT32_MAX && roulette->secondDist != UINT32_MAX)
	{
		roulette->secondToFirst = roulette->secondDist - roulette->firstDist;
		roulette->secondToFirst = K_ScaleItemDistance(roulette->secondToFirst, 16 - roulette->playing); // Reversed scaling
	}
}

/*--------------------------------------------------
	static void K_PushToRouletteItemList(itemroulette_t *const roulette, kartitems_t item)

		Pushes a new item to the end of the item
		roulette's item list.

	Input Arguments:-
		roulette - The item roulette data to modify.
		item - The item to push to the list.

	Return:-
		N/A
--------------------------------------------------*/
static void K_PushToRouletteItemList(itemroulette_t *const roulette, kartitems_t item)
{
#ifdef ITEM_LIST_SIZE
	if (roulette->itemListLen >= ITEM_LIST_SIZE)
	{
		I_Error("Out of space for item reel! Go and make ITEM_LIST_SIZE bigger I guess?\n");
		return;
	}
#else
	I_Assert(roulette->itemList != NULL);

	if (roulette->itemListLen >= roulette->itemListCap)
	{
		roulette->itemListCap *= 2;
		roulette->itemList = Z_Realloc(
			roulette->itemList,
			sizeof(SINT8) * roulette->itemListCap,
			PU_STATIC,
			&roulette->itemList
		);

		if (roulette->itemList == NULL)
		{
			I_Error("Not enough memory for item roulette list\n");
		}
	}
#endif

	roulette->itemList[ roulette->itemListLen ] = item;
	roulette->itemListLen++;
}

/*--------------------------------------------------
	static void K_AddItemToReel(const player_t *player, itemroulette_t *const roulette, kartitems_t item)

		Adds an item to a player's item reel. Unlike
		pushing directly with K_PushToRouletteItemList,
		this function handles special behaviors (like
		padding with extra Super Rings).

	Input Arguments:-
		player - The player to add to the item roulette.
			This is valid to be NULL.
		roulette - The player's item roulette data.
		item - The item to push to the list.

	Return:-
		N/A
--------------------------------------------------*/
static void K_AddItemToReel(const player_t *player, itemroulette_t *const roulette, kartitems_t item)
{
	K_PushToRouletteItemList(roulette, item);

	if (player == NULL)
	{
		return;
	}

	// If we're in ring debt, pad out the reel with
	// a BUNCH of Super Rings.
	if (K_ItemEnabled(KITEM_SUPERRING) == true
		&& player->rings <= 0
		&& (gametyperules & GTR_SPHERES) == 0)
	{
		K_PushToRouletteItemList(roulette, KITEM_SUPERRING);
	}
}

/*--------------------------------------------------
	static void K_CalculateRouletteSpeed(itemroulette_t *const roulette)

		Determines the speed for the item roulette,
		adjusted for progress in the race and front
		running.

	Input Arguments:-
		roulette - The item roulette data to modify.

	Return:-
		N/A
--------------------------------------------------*/
static void K_CalculateRouletteSpeed(itemroulette_t *const roulette)
{
	fixed_t frontRun = 0;
	fixed_t progress = 0;
	fixed_t total = 0;

	if (modeattacking || roulette->playing <= 1)
	{
		// Time Attack rules; use a consistent speed.
		roulette->tics = roulette->speed = ROULETTE_SPEED_TIMEATTACK;
		return;
	}

	if (roulette->baseDist > ENDDIST)
	{
		// Being farther in the course makes your roulette faster.
		progress = min(FRACUNIT, FixedDiv(roulette->baseDist - ENDDIST, ROULETTE_SPEED_DIST));
	}

	if (roulette->baseDist > roulette->firstDist)
	{
		// Frontrunning makes your roulette faster.
		frontRun = min(FRACUNIT, FixedDiv(roulette->baseDist - roulette->firstDist, ENDDIST));
	}

	// Combine our two factors together.
	total = min(FRACUNIT, (frontRun / 2) + (progress / 2));

	if (leveltime < starttime + 30*TICRATE)
	{
		// Don't impact as much at the start.
		// This makes it so that everyone gets to enjoy the lowest speed at the start.
		if (leveltime < starttime)
		{
			total = FRACUNIT;
		}
		else
		{
			const fixed_t lerp = FixedDiv(leveltime - starttime, 30*TICRATE);
			total = FRACUNIT + FixedMul(lerp, total - FRACUNIT);
		}
	}

	roulette->tics = roulette->speed = ROULETTE_SPEED_FASTEST + FixedMul(ROULETTE_SPEED_SLOWEST - ROULETTE_SPEED_FASTEST, total);
}

/*--------------------------------------------------
	void K_FillItemRouletteData(const player_t *player, itemroulette_t *const roulette)

		See header file for description.
--------------------------------------------------*/
void K_FillItemRouletteData(const player_t *player, itemroulette_t *const roulette)
{
	UINT32 spawnChance[NUMKARTRESULTS] = {0};
	UINT32 totalSpawnChance = 0;
	size_t rngRoll = 0;

	UINT8 numItems = 0;
	kartitems_t singleItem = KITEM_SAD;

	size_t i;

	K_InitRoulette(roulette);

	if (player != NULL)
	{
		roulette->baseDist = K_UndoMapScaling(player->distancetofinish);
		K_CalculateRouletteSpeed(roulette);
	}

	// SPECIAL CASE No. 1:
	// Give only the debug item if specified
	if (cv_kartdebugitem.value != KITEM_NONE)
	{
		K_PushToRouletteItemList(roulette, cv_kartdebugitem.value);
		return;
	}

	// SPECIAL CASE No. 2:
	// Use a special, pre-determined item reel for Time Attack / Free Play
	if (bossinfo.boss == true)
	{
		for (i = 0; K_KartItemReelBoss[i] != KITEM_NONE; i++)
		{
			K_PushToRouletteItemList(roulette, K_KartItemReelBoss[i]);
		}

		return;
	}
	else if (modeattacking || roulette->playing <= 1)
	{
		switch (gametype)
		{
			case GT_RACE:
			default:
			{
				for (i = 0; K_KartItemReelTimeAttack[i] != KITEM_NONE; i++)
				{
					K_PushToRouletteItemList(roulette, K_KartItemReelTimeAttack[i]);
				}
				break;
			}
			case GT_BATTLE:
			{
				for (i = 0; K_KartItemReelBreakTheCapsules[i] != KITEM_NONE; i++)
				{
					K_PushToRouletteItemList(roulette, K_KartItemReelBreakTheCapsules[i]);
				}
				break;
			}
		}

		return;
	}

	// SPECIAL CASE No. 3:
	// Only give the SPB if conditions are right
	if (K_ForcedSPB(player, roulette) == true)
	{
		K_AddItemToReel(player, roulette, KITEM_SPB);
		return;
	}

	// SPECIAL CASE No. 4:
	// If only one item is enabled, always use it
	for (i = 1; i < NUMKARTRESULTS; i++)
	{
		if (K_ItemEnabled(i) == true)
		{
			numItems++;
			if (numItems > 1)
			{
				break;
			}

			singleItem = i;
		}
	}

	if (numItems < 2)
	{
		// singleItem = KITEM_SAD by default,
		// so it will be used when all items are turned off.
		K_AddItemToReel(player, roulette, singleItem);
		return;
	}

	// Special cases are all handled, we can now
	// actually calculate actual item reels.
	roulette->dist = K_GetItemRouletteDistance(player, roulette->playing);
	roulette->useOdds = K_FindUseodds(player, roulette);

	for (i = 1; i < NUMKARTRESULTS; i++)
	{
		spawnChance[i] = (
			totalSpawnChance += K_KartGetItemOdds(player, roulette, roulette->useOdds, i)
		);
	}

	if (totalSpawnChance == 0)
	{
		// This shouldn't happen, but if it does, early exit.
		// Maybe can happen if you enable multiple items for
		// another gametype, so we give the singleItem as a fallback.
		K_AddItemToReel(player, roulette, singleItem);
		return;
	}

	// Create the same item reel given the same inputs.
	P_SetRandSeed(PR_ITEM_ROULETTE, ITEM_REEL_SEED);

	while (totalSpawnChance > 0)
	{
		rngRoll = P_RandomKey(PR_ITEM_ROULETTE, totalSpawnChance);
		for (i = 1; i < NUMKARTRESULTS && spawnChance[i] <= rngRoll; i++)
		{
			continue;
		}

		K_AddItemToReel(player, roulette, i);

		for (; i < NUMKARTRESULTS; i++)
		{
			// Be sure to fix the remaining items' odds too.
			if (spawnChance[i] > 0)
			{
				spawnChance[i]--;
			}
		}

		totalSpawnChance--;
	}
}

/*--------------------------------------------------
	void K_StartItemRoulette(player_t *const player)

		See header file for description.
--------------------------------------------------*/
void K_StartItemRoulette(player_t *const player)
{
	itemroulette_t *const roulette = &player->itemRoulette;
	size_t i;

	K_FillItemRouletteData(player, roulette);

	// Make the bots select their item after a little while.
	// One of the few instances of bot RNG, would be nice to remove it.
	player->botvars.itemdelay = P_RandomRange(PR_UNDEFINED, TICRATE, TICRATE*3);

	// Prevent further duplicates of items that
	// are intended to only have one out at a time.
	for (i = 0; i < roulette->itemListLen; i++)
	{
		kartitems_t item = roulette->itemList[i];
		if (K_ItemSingularity(item) == true)
		{
			K_SetItemCooldown(item, TICRATE<<4);
		}
	}
}

/*--------------------------------------------------
	void K_StartEggmanRoulette(player_t *const player)

		See header file for description.
--------------------------------------------------*/
void K_StartEggmanRoulette(player_t *const player)
{
	itemroulette_t *const roulette = &player->itemRoulette;
	K_StartItemRoulette(player);
	roulette->eggman = true;
}

/*--------------------------------------------------
	fixed_t K_GetRouletteOffset(itemroulette_t *const roulette, fixed_t renderDelta)

		See header file for description.
--------------------------------------------------*/
fixed_t K_GetRouletteOffset(itemroulette_t *const roulette, fixed_t renderDelta)
{
	const fixed_t curTic = (roulette->tics << FRACBITS) - renderDelta;
	const fixed_t midTic = roulette->speed * (FRACUNIT >> 1);

	return FixedMul(FixedDiv(midTic - curTic, ((roulette->speed + 1) << FRACBITS)), ROULETTE_SPACING);
}

/*--------------------------------------------------
	static void K_KartGetItemResult(player_t *const player, kartitems_t getitem)

		Initializes a player's item to what was
		received from the roulette.

	Input Arguments:-
		player - The player receiving the item.
		getitem - The item to give to the player.

	Return:-
		N/A
--------------------------------------------------*/
static void K_KartGetItemResult(player_t *const player, kartitems_t getitem)
{
	if (K_ItemSingularity(getitem) == true)
	{
		K_SetItemCooldown(getitem, 20*TICRATE);
	}

	player->botvars.itemdelay = TICRATE;
	player->botvars.itemconfirm = 0;

	player->itemtype = K_ItemResultToType(getitem);
	player->itemamount = K_ItemResultToAmount(getitem);
}

/*--------------------------------------------------
	void K_KartItemRoulette(player_t *const player, ticcmd_t *const cmd)

		See header file for description.
--------------------------------------------------*/
void K_KartItemRoulette(player_t *const player, ticcmd_t *const cmd)
{
	itemroulette_t *const roulette = &player->itemRoulette;
	boolean confirmItem = false;

	// This makes the roulette cycle through items.
	// If this isn't active, you shouldn't be here.
	if (roulette->active == false)
	{
		return;
	}

	if (roulette->itemListLen == 0
#ifndef ITEM_LIST_SIZE
		|| roulette->itemList == NULL
#endif
		)
	{
		// Invalid roulette setup.
		// Escape before we run into issues.
		roulette->active = false;
		return;
	}

	if (roulette->elapsed > TICRATE>>1) // Prevent accidental immediate item confirm
	{
		if (roulette->elapsed > TICRATE<<4)
		{
			// Waited way too long, forcefully confirm the item.
			confirmItem = true;
		}
		else
		{
			// We can stop our item when we choose.
			confirmItem = !!(cmd->buttons & BT_ATTACK);
		}
	}

	// If the roulette finishes or the player presses BT_ATTACK, stop the roulette and calculate the item.
	// I'm returning via the exact opposite, however, to forgo having another bracket embed. Same result either way, I think.
	// Finally, if you get past this check, now you can actually start calculating what item you get.
	if (confirmItem == true && (player->pflags & (PF_ITEMOUT|PF_EGGMANOUT|PF_USERINGS)) == 0)
	{
		if (roulette->eggman == true)
		{
			// FATASS JUMPSCARE instead of your actual item
			player->eggmanexplode = 4*TICRATE;

			//player->karthud[khud_itemblink] = TICRATE;
			//player->karthud[khud_itemblinkmode] = 1;
			//player->karthud[khud_rouletteoffset] = K_GetRouletteOffset(roulette, FRACUNIT);

			if (P_IsDisplayPlayer(player) && !demo.freecam)
			{
				S_StartSound(NULL, sfx_itrole);
			}
		}
		else
		{
			kartitems_t finalItem = roulette->itemList[ roulette->index ];

			K_KartGetItemResult(player, finalItem);

			player->karthud[khud_itemblink] = TICRATE;
			player->karthud[khud_itemblinkmode] = 0;
			player->karthud[khud_rouletteoffset] = K_GetRouletteOffset(roulette, FRACUNIT);

			if (P_IsDisplayPlayer(player) && !demo.freecam)
			{
				S_StartSound(NULL, sfx_itrolf);
			}
		}

		// We're done, disable the roulette
		roulette->active = false;
		return;
	}

	roulette->elapsed++;

	if (roulette->tics == 0)
	{
		roulette->index = (roulette->index + 1) % roulette->itemListLen;
		roulette->tics = roulette->speed;

		// This makes the roulette produce the random noises.
		roulette->sound = (roulette->sound + 1) % 8;

		if (P_IsDisplayPlayer(player) && !demo.freecam)
		{
			S_StartSound(NULL, sfx_itrol1 + roulette->sound);
		}
	}
	else
	{
		roulette->tics--;
	}
}
