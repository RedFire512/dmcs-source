// Definitions for Quake 3 Arena-style Bobbing

#ifndef QUAKE3BOB_H
#define QUAKE3BOB_H

#define ITEM_ROTATION_RATE	( 360.0f / 4.0f )
#define ITEM_BOB_DISTANCE 4.0

inline void Quake3Bob(CBaseEntity *pEntity, Vector vecBaseOrigin)
{
	float bobscale = 4.0 + (pEntity->entindex() * 0.001);
	float boboffset = (cos((gpGlobals->curtime + 1000) * bobscale) * ITEM_BOB_DISTANCE) + ITEM_BOB_DISTANCE;

	pEntity->SetLocalOrigin(vecBaseOrigin + Vector{ 0.0, 0.0, boboffset });
}

inline void Quake3Rotate(CBaseEntity *pEntity, QAngle &vecAngles)
{
	vecAngles.y += ITEM_ROTATION_RATE * gpGlobals->frametime;
	if (vecAngles.y >= 360)
		vecAngles.y -= 360;

	pEntity->SetLocalAngles(vecAngles);
}

#endif