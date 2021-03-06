#ifndef TU_MAPITEM_H
#define TU_MAPITEM_H

#include <game/mapitems.h>

namespace tu
{

enum
{
	MAPLAYERTYPE_ENTITIES = LAYERTYPE_QUADS+1,
};
	
enum
{
	ENTITYPOINTTYPE_SPAWN=0,
	ENTITYPOINTTYPE_SPAWN_RED,
	ENTITYPOINTTYPE_SPAWN_BLUE,
	ENTITYPOINTTYPE_FLAGSTAND_RED,
	ENTITYPOINTTYPE_FLAGSTAND_BLUE,
	ENTITYPOINTTYPE_ARMOR,
	ENTITYPOINTTYPE_HEALTH,
	ENTITYPOINTTYPE_WEAPON_SHOTGUN,
	ENTITYPOINTTYPE_WEAPON_GRENADE,
	ENTITYPOINTTYPE_WEAPON_NINJA,
	ENTITYPOINTTYPE_WEAPON_LASER,
	ENTITYPOINTTYPE_WEAPON_GUN,
	ENTITYPOINTTYPE_WEAPON_HAMMER,
};

struct CMapEntity_Point
{
	int x;
	int y;
	int m_Type;
};

struct CMapItem_LayerEntities
{
	enum { CURRENT_VERSION=1 };

	CMapItemLayer m_Layer;
	int m_Version;
	int m_ModName;
	
	int m_NumPoints;
	int m_PointsData;

	int m_aName[3];
};

}

#endif
