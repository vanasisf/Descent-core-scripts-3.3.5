/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
GameObject::GameObject(uint64 guid)
{
	m_objectTypeId = TYPEID_GAMEOBJECT;
	internal_object_type = INTERNAL_OBJECT_TYPE_GAMEOBJECT;
	m_valuesCount = GAMEOBJECT_END;
	m_uint32Values = _fields;
	memset(m_uint32Values, 0,(GAMEOBJECT_END)*sizeof(uint32));
	m_updateMask.SetCount(GAMEOBJECT_END);
	SetUInt32Value( OBJECT_FIELD_TYPE,TYPE_GAMEOBJECT|TYPE_OBJECT);
	SetUInt64Value( OBJECT_FIELD_GUID,guid);
	m_wowGuid.Init(GetGUID());
 
	SetFloatValue( OBJECT_FIELD_SCALE_X, 1);//info->Size  );

	counter=0;//not needed at all but to prevent errors that var was not inited, can be removed in release

	bannerslot = bannerauraslot = -1;

	m_summonedGo = false;
	invisible = false;
	invisibilityFlag = INVIS_FLAG_NORMAL;
	spell = 0;
	m_summoner = NULL;
	charges = -1;
	m_ritualcaster = 0;
	m_ritualtarget = 0;
	m_ritualmembers = NULL;
	m_ritualspell = 0;

	m_quests = NULL;
	pInfo = NULL;
	myScript = NULL;
	m_spawn = 0;
	loot.gold = 0;
	m_deleted = false;
//	usage_remaining = 1;
	m_respawnCell=NULL;
	m_battleground = NULL;
//	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS, 100); // spawn dors as closed ?
	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS, 0xFF); // mostly used for 3.1.1 client. But why ?
	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, GO_STATE_READY); // mostly used for 3.1.1 client. But why ?
	m_rotation = 0;
}


GameObject::~GameObject()
{
	Virtual_Destructor();
}

void GameObject::Virtual_Destructor()
{
	if(myScript != NULL)
	{
		myScript->Destroy();	 //deletes script !
		myScript = NULL;
	}
	sEventMgr.RemoveEvents(this);
	if(m_ritualmembers)
	{
		delete[] m_ritualmembers;
		m_ritualmembers = NULL;
	}

	uint32 guid = GetUInt32Value(OBJECT_FIELD_CREATED_BY);
	if(guid)
	{
		Player *plr = objmgr.GetPlayer(guid);
		if(plr && plr->GetSummonedObject() == this)
			plr->SetSummonedObject(NULL);

		if(plr == m_summoner)
			m_summoner = 0;
	}

	if(m_respawnCell!=NULL)
	{
		m_respawnCell->_respawnObjects.erase(this);
		m_respawnCell = NULL;
	}

	if (m_summonedGo && m_summoner)
	{
		for(int i = 0; i < 4; i++)
			if (m_summoner->m_ObjectSlots[i] == GetLowGUID())
				m_summoner->m_ObjectSlots[i] = 0;
		m_summoner = NULL;
	}

	if( m_battleground != NULL && m_battleground->GetType() == BATTLEGROUND_ARATHI_BASIN )
	{
		if( bannerslot >= 0 && static_cast<ArathiBasin*>(m_battleground)->m_controlPoints[bannerslot] == this )
			static_cast<ArathiBasin*>(m_battleground)->m_controlPoints[bannerslot] = NULL;

		if( bannerauraslot >= 0 && static_cast<ArathiBasin*>(m_battleground)->m_controlPointAuras[bannerauraslot] == this )
			static_cast<ArathiBasin*>(m_battleground)->m_controlPointAuras[bannerauraslot] = NULL;
		m_battleground = NULL;
	}
	Object::Virtual_Destructor();
	m_objectTypeId = TYPEID_UNUSED;
	internal_object_type = INTERNAL_OBJECT_TYPE_NONE;
}

bool GameObject::CreateFromProto(uint32 entry,uint32 mapid, float x, float y, float z, float ang, float r0, float r1, float r2, float r3)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	pInfo= GameObjectNameStorage.LookupEntry(entry);
	if(!pInfo)
	{
		return false;
	}

	Object::_Create( mapid, x, y, z, ang );
	SetUInt32Value( OBJECT_FIELD_ENTRY, entry );
	
	SetPosition(x, y, z, ang);
	SetParentRotation(0, r0);
	SetParentRotation(1, r1);
	SetParentRotation(2, r2);
	SetParentRotation(3, r3);
	UpdateRotation();

    SetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS, 0 );
	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, GO_STATE_READY );
	SetUInt32Value( GAMEOBJECT_DISPLAYID, pInfo->DisplayID );
	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPEID, pInfo->Type ); 
	SetFloatValue( OBJECT_FIELD_SCALE_X, pInfo->Scale );
   
	InitAI();

	 return true;
}

/*
void GameObject::Create(uint32 mapid, float x, float y, float z, float ang)
{
	Object::_Create( mapid, x, y, z, ang);

	SetFloatValue( GAMEOBJECT_POS_X, x);
	SetFloatValue( GAMEOBJECT_POS_Y, y );
	SetFloatValue( GAMEOBJECT_POS_Z, z );
	SetFloatValue( GAMEOBJECT_FACING, ang );
	//SetUInt32Value( GAMEOBJECT_TIMESTAMP, (uint32)time(NULL));
}

void GameObject::Create( uint32 guidlow, uint32 guidhigh,uint32 displayid, uint8 state, uint32 entryid, float scale,uint32 typeId, uint32 type,uint32 flags, uint32 mapid, float x, float y, float z, float ang )
{
	Object::_Create( mapid, x, y, z, ang);

	SetUInt32Value( OBJECT_FIELD_ENTRY, entryid );
	SetFloatValue( OBJECT_FIELD_SCALE_X, scale );
	SetUInt32Value( GAMEOBJECT_DISPLAYID, displayid );
	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, state  );
	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPEID, typeId  );
	SetUInt32Value( GAMEOBJECT_FLAGS, flags );
}*/

void GameObject::TrapSearchTarget()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	Update(100);
}

void GameObject::Update(uint32 p_time)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE

	Object::Update( p_time );	//99% this does nothing

	if(!IsInWorld())
	{ 
		return;
	}

	if(m_deleted)
	{ 
		return;
	}

	if(spell && (GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE) == GO_STATE_READY))
	{
		if(checkrate > 1)
		{
			counter++;
			if(counter < checkrate)
			{ 
				return;
			}
			counter = 0;
		}
		ObjectSet::iterator itr = GetInRangeSetBegin();
		ObjectSet::iterator it2 = itr;
		ObjectSet::iterator iend = GetInRangeSetEnd();
		Unit * pUnit;
		float dist;
		this->AquireInrangeLock(); //make sure to release lock before exit function !
		for(; it2 != iend;)
		{
			itr = it2;
			++it2;
			dist = GetDistanceSq((*itr));
			if( (*itr) != m_summoner && (*itr)->IsUnit() && dist <= range)
			{
				pUnit = SafeUnitCast(*itr);

				if(m_summonedGo)
				{
					if(!m_summoner)
					{
						ExpireAndDelete();
						ReleaseInrangeLock();
						return;
					}
					if(!isAttackable(m_summoner,pUnit))
						continue;
				}
				
				Spell * sp=SpellPool.PooledNew();
				sp->Init((Object*)this,spell,true,NULL);
				SpellCastTargets tgt((*itr)->GetGUID());
				tgt.m_destX = GetPositionX();
				tgt.m_destY = GetPositionY();
				tgt.m_destZ = GetPositionZ();
				sp->prepare(&tgt);

				// proc on trap trigger
				if( pInfo->Type == GAMEOBJECT_TYPE_TRAP )
				{
					if( m_summoner != NULL )
						m_summoner->HandleProc( PROC_ON_TRAP_TRIGGER, pUnit, spell );
				} 

				if(m_summonedGo)
				{
					ExpireAndDelete();
					this->ReleaseInrangeLock();
					return;
				}

				if(spell->EffectImplicitTargetA[0] == 16 ||
					spell->EffectImplicitTargetB[0] == 16)
				{
					this->ReleaseInrangeLock();
					return;	 // on area dont continue.
				}
			}
		}
		this->ReleaseInrangeLock();
	}
}

void GameObject::OnPrePushToWorld()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	UpdateGoOrientation();
}

void GameObject::Spawn(MapMgr * m)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	PushToWorld(m);	
	CALL_GO_SCRIPT_EVENT(this, OnSpawn)();
	loot.looters.clear();
}

void GameObject::Despawn(uint32 delay, uint32 respawntime)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if(delay)
	{
		if( sEventMgr.HasEvent(this,EVENT_GAMEOBJECT_EXPIRE) == false )
			sEventMgr.AddEvent(this, &GameObject::Despawn, respawntime, EVENT_GAMEOBJECT_EXPIRE, delay, 1,0);	//need to have this executed in world context
		return;
	}
	else
		Despawn( respawntime );
}

void GameObject::Despawn(uint32 respawntime)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if( !IsGameObject() )
	{
		sLog.outDebug(" GameObject::Despawn : !!! omg invalid call on wrong object !\n");
		ASSERT(false);
		return;
	}
	//zack: so what if it got removed before ?
//	if(!IsInWorld())
//  {
//	 
//		return;
//	}

	//empty loot
	loot.items.clear();

	//This is for go get deleted while looting
	if(m_spawn)
	{
		SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, m_spawn->state);
		SetUInt32Value(GAMEOBJECT_FLAGS, m_spawn->flags);
	}

	CALL_GO_SCRIPT_EVENT(this, OnDespawn)();

	if(respawntime)
	{
		//just to make sure we save values
		m_respawnCell = GetMapCell();
		MapMgr *oldMap = GetMapMgr();

		if( oldMap )
		{
			sEventMgr.RemoveEvents(this);
			//remove from world also deletes all our events
			Object::RemoveFromWorld(false);

			//needs to be after removing from world
			m_respawnCell->_respawnObjects.insert( this );
			sEventMgr.AddEvent(oldMap, &MapMgr::EventRespawnGameObject, this, GetPositionX(), GetPositionY(), EVENT_GAMEOBJECT_ITEM_SPAWN, respawntime, 1, 0);
		}
	}
	else
	{
		Object::RemoveFromWorld(true);
		ExpireAndDelete();
	}
}

void GameObject::SaveToDB()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	std::stringstream ss;
	ss << "REPLACE INTO gameobject_spawns VALUES("
		<< ((m_spawn == NULL) ? 0 : m_spawn->id) << ","
		<< "'" << GetEntry() << "',"
		<< "'" << GetMapId() << "',"
		<< "'" << GetPositionX() << "',"
		<< "'" << GetPositionY() << "',"
		<< "'" << GetPositionZ() << "',"
		<< "'" << GetOrientation() << "',"
		<< "'" << GetFloatValue(GAMEOBJECT_PARENTROTATION) << "',"
		<< "'" << GetFloatValue(GAMEOBJECT_PARENTROTATION_1) << "',"
		<< "'" << GetFloatValue(GAMEOBJECT_PARENTROTATION_2) << "',"
		<< "'" << GetFloatValue(GAMEOBJECT_PARENTROTATION_3) << "',"
		<< "'" << (uint32)GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE) << "',"
		<< "'" << GetUInt32Value(GAMEOBJECT_FLAGS) << "',"
		<< "'" << GetUInt32Value(GAMEOBJECT_FACTION) << "',"
		<< "'" << GetFloatValue(OBJECT_FIELD_SCALE_X) << "',"
		<< "0)";
	WorldDatabase.Execute(ss.str().c_str());

  /*  std::stringstream ss;
	if (!m_sqlid)
		m_sqlid = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);

	ss << "DELETE FROM gameobjects WHERE id=" << m_sqlid;
	sDatabase.Execute(ss.str().c_str());

	ss.rdbuf()->str("");
	ss << "INSERT INTO gameobjects VALUES ( "
		<< m_sqlid << ", "
		<< m_position.x << ", "
		<< m_position.y << ", "
		<< m_position.z << ", "
		<< m_position.o << ", "
		<< GetZoneId() << ", "
		<< GetMapId() << ", '";

	for( uint32 index = 0; index < m_valuesCount; index ++ )
		ss << GetUInt32Value(index) << " ";

	ss << "', ";
	ss << GetEntry() << ", 0, 0)"; 

	sDatabase.Execute( ss.str( ).c_str( ) );*/
}

void GameObject::SaveToFile(std::stringstream & name)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
/*	std::stringstream ss;
	if (!m_sqlid)
		m_sqlid = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);

	 ss.rdbuf()->str("");
	 ss << "INSERT INTO gameobjects VALUES ( "
		<< m_sqlid << ", "
		<< m_position.x << ", "
		<< m_position.y << ", "
		<< m_position.z << ", "
		<< m_position.o << ", "
		<< GetZoneId() << ", "
		<< GetMapId() << ", '";

	for( uint32 index = 0; index < m_valuesCount; index ++ )
		ss << GetUInt32Value(index) << " ";

	ss << "', ";
	ss << GetEntry() << ", 0, 0)"; 

	FILE * OutFile;

	OutFile = fopen(name.str().c_str(), "wb");
	if (!OutFile) return;
	fwrite(ss.str().c_str(),1,ss.str().size(),OutFile);
	fclose(OutFile);
*/
}

void GameObject::InitAI()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	
	if(!pInfo)
	{ 
		return;
	}
	
	// this fixes those fuckers in booty bay
	if(pInfo->SpellFocus == 0 &&
		pInfo->sound1 == 0 &&
		pInfo->sound2 == 0 &&
		pInfo->sound3 != 0 &&
		pInfo->sound5 != 3 &&
		pInfo->sound9 == 1)
		return;

	if(pInfo->DisplayID == 1027)//Shaman Shrine
	{
		if(pInfo->ID != 177964 && pInfo->ID != 153556)
		{
			//Deactivate
			//SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
		}
	}


	uint32 spellid = 0;
	if(pInfo->Type==GAMEOBJECT_TYPE_TRAP)
	{	
		spellid = pInfo->sound3;
	}
	else if(pInfo->Type == GAMEOBJECT_TYPE_SPELL_FOCUS)
	{
		// get spellid from attached gameobject - by sound2 field
		if( pInfo->sound2 == 0 )
		{ 
			return;
		}
		if( GameObjectNameStorage.LookupEntry( pInfo->sound2 ) == NULL )
		{ 
			return;
		}
		spellid = GameObjectNameStorage.LookupEntry( pInfo->sound2 )->sound3;
	}
	else if(pInfo->Type == GAMEOBJECT_TYPE_RITUAL)
	{	
		m_ritualmembers = new uint64[pInfo->SpellFocus];
		memset(m_ritualmembers,0,sizeof(uint64)*pInfo->SpellFocus);
	}
    else if(pInfo->Type == GAMEOBJECT_TYPE_CHEST)
    {
        Lock *pLock = dbcLock.LookupEntry(GetInfo()->SpellFocus);
        if(pLock)
        {
            for(uint32 i=0; i < 5; i++)
            {
               if( pLock->locktype[i] == GO_LOCKTYPE_SKILL_REQ ) //locktype;
               {
                    //herbalism and mining;
                    if(pLock->lockmisc[i] == LOCKTYPE_MINING || pLock->lockmisc[i] == LOCKTYPE_HERBALISM)
                    {
						CalcMineRemaining(true);
                    }
                }
            }
        }

    }
	else if ( pInfo->Type == GAMEOBJECT_TYPE_FISHINGHOLE )
	{
		CalcFishRemaining( true );
	}

	if( myScript == NULL )
		myScript = sScriptMgr.CreateAIScriptClassForGameObject(GetEntry(), this);

	// hackfix for bad spell in BWL
	if(!spellid || spellid == 22247)
	{ 
		return;
	}

	SpellEntry *sp= dbcSpell.LookupEntry(spellid);
	if(!sp)
	{
		spell = NULL;
		return;
	}
	else
	{
		spell = sp;
	}
	//ok got valid spell that will be casted on target when it comes close enough
	//get the range for that 
	
	float r = 0;

	for(uint32 i=0;i<3;i++)
	{
		if(sp->Effect[i])
		{
			float t = GetRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[i]));
			if(t > r)
				r = t;
		}
	}

	if(r < 0.1)//no range
		r = GetMaxRange(dbcSpellRange.LookupEntry(sp->rangeIndex));
	if( pInfo->Type==GAMEOBJECT_TYPE_TRAP )
		r = MAX( r, GetMaxRange(dbcSpellRange.LookupEntry(sp->rangeIndex)) );

	range = r*r;//square to make code faster
	checkrate = 20;//once in 2 seconds
	
}

bool GameObject::Load(GOSpawn *spawn)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if(!CreateFromProto(spawn->entry,0,spawn->x,spawn->y,spawn->z,spawn->facing,spawn->o,spawn->o1,spawn->o2,spawn->o3))
	{ 
		return false;
	}

	m_spawn = spawn;
	m_phase = spawn->phase;

	SetUInt32Value(GAMEOBJECT_FLAGS,spawn->flags);

	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE,spawn->state);	

	if(spawn->faction)
	{
		SetFaction(spawn->faction);
	}
	SetFloatValue(OBJECT_FIELD_SCALE_X,spawn->scale);
	_LoadQuests();
	CALL_GO_SCRIPT_EVENT(this, OnCreate)();
	CALL_GO_SCRIPT_EVENT(this, OnSpawn)();

	InitAI();

	_LoadQuests();
	return true;
}

void GameObject::DeleteFromDB()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if( m_spawn != NULL )
		WorldDatabase.Execute("DELETE FROM gameobject_spawns WHERE id=%u", m_spawn->id);
}

void GameObject::EventCloseDoor()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
//	SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, GO_STATE_READY);
	SetUInt32Value(GAMEOBJECT_FLAGS, 0);
}

void GameObject::UseFishingNode(Player *player)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	sEventMgr.RemoveEvents( this );
	if( GetUInt32Value( GAMEOBJECT_FLAGS ) != GAMEOBJECT_CLICKABLE ) // Clicking on the bobber before something is hooked
	{
		player->GetSession()->OutPacket( SMSG_FISH_NOT_HOOKED );
		EndFishing( player, true );
		return;
	}
	
	/* Unused code: sAreaStore.LookupEntry(GetMapMgr()->GetAreaID(GetPositionX(),GetPositionY()))->ZoneId*/
	uint32 zone = player->GetAreaID();
	if( zone == 0 ) // If the player's area ID is 0, use the zone ID instead
		zone = player->GetZoneId();

	FishingZoneEntry *entry = FishingZoneStorage.LookupEntry( zone );

	if( entry == NULL ) // No fishing information found for area or zone, log an error, and end fishing
	{
		sLog.outError( "ERROR: Fishing zone information for zone %d not found!", zone );
		EndFishing( player, true );
		return;
	}
	uint32 maxskill = entry->MaxSkill;
	uint32 minskill = entry->MinSkill;

	if( player->_GetSkillLineCurrent( SKILL_FISHING, false ) < maxskill )	
		player->_AdvanceSkillLine( SKILL_FISHING, float2int32( 1.0f * sWorld.getRate( RATE_SKILLRATE ) ) );

	GameObject * school = NULL;
	this->AquireInrangeLock(); //make sure to release lock before exit function !
	for ( InRangeSet::iterator it = GetInRangeSetBegin(); it != GetInRangeSetEnd(); ++it )
	{
		if ( (*it) == NULL || (*it)->GetTypeId() != TYPEID_GAMEOBJECT || (*it)->GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPEID) != GAMEOBJECT_TYPE_FISHINGHOLE)
			continue;
		school = SafeGOCast( *it );
		if ( !isInRange( school, (float)school->GetInfo()->sound1 ) )
		{
			school = NULL;
			continue;
		}
		else
			break;
	}
	this->ReleaseInrangeLock();

	if ( school != NULL ) // open school loot if school exists
	{
//		lootmgr.FillGOLoot( &school->loot, school->GetEntry(), school->GetMapMgr() ? ( school->GetMapMgr()->iInstanceMode ? true : false ) : false );
		// zack : blizz recycles gameobject entry. We use something that is same for all entry variations
		lootmgr.FillGOLoot(&school->loot,school->GetInfo()->sound1, (school->GetMapMgr() != NULL) ? (school->GetMapMgr()->instance_difficulty) : 0);
		player->SendLoot( school->GetGUID(), LOOT_FISHING );

		EndFishing( player, false );
		school->CatchFish();
		if ( !school->CanFish() )
//			sEventMgr.AddEvent( school, &GameObject::Despawn, ( 1800000 + RandomUInt( 3600000 ) ), EVENT_GAMEOBJECT_EXPIRE, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT ); // respawn in 30 - 90 minutes
//			sEventMgr.AddEvent( school, &GameObject::Despawn, ( 300000 + RandomUInt( 300000 ) ), EVENT_GAMEOBJECT_EXPIRE, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT ); // respawn in 30 - 90 minutes
//			sEventMgr.AddEvent( school, &GameObject::Despawn, ( school->GetInfo()->respawn_time + RandomUInt( school->GetInfo()->respawn_time ) ), EVENT_GAMEOBJECT_EXPIRE, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT ); 
			sEventMgr.AddEvent( school, &GameObject::Despawn, school->GetInfo()->respawn_time, EVENT_GAMEOBJECT_EXPIRE, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT ); 
	}
	else if( Rand( ( ( player->_GetSkillLineCurrent( SKILL_FISHING, true ) - minskill ) * 100 ) / maxskill ) ) // Open loot on success, otherwise FISH_ESCAPED.
	{	
		lootmgr.FillFishingLoot( &loot, zone );
		player->SendLoot( GetGUID(), LOOT_FISHING );
		player->Event_Achiement_Received( ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE,LOOT_FISHING,ACHIEVEMENT_UNUSED_FIELD_VALUE,1,ACHIEVEMENT_EVENT_ACTION_ADD);
		EndFishing( player, false );
	}
	else // Failed
	{
		player->GetSession()->OutPacket( SMSG_FISH_ESCAPED );
		EndFishing( player, true );
	}

}

void GameObject::EndFishingEvent(uint64 guid, bool abort )
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if( IsInWorld() )
	{
		Player *player = GetMapMgr()->GetPlayer( guid );
		if( player )
			EndFishing( player, abort );
	}
	else
		EndFishing( NULL, abort );
}

void GameObject::EndFishing(Player *player, bool abort )
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if( player )
	{
		Spell * spell = player->GetCurrentSpell();
		
		if(spell)
		{
			if(abort)   // abort becouse of a reason
			{
				//FIXME: here 'failed' should appear over progress bar
				spell->SendChannelUpdate(0);
				//spell->cancel();
				spell->finish();
			}
			else		// spell ended
			{
				spell->SendChannelUpdate(0);
				spell->finish();
			}
		}
	}

	if(!abort)
	{
//		sEventMgr.AddEvent(this, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_EXPIRE, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
//		sEventMgr.AddEvent(this, &GameObject::Despawn, ( GetInfo()->respawn_time + RandomUInt( GetInfo()->respawn_time ) ), EVENT_GAMEOBJECT_EXPIRE, 1, 1, 0 ); 
		sEventMgr.AddEvent(this, &GameObject::Despawn, (uint32)20000, GetInfo()->respawn_time, EVENT_GAMEOBJECT_EXPIRE, 1, 1, 0 ); 
	}
	else
//		ExpireAndDelete();
//		sEventMgr.AddEvent( this, &GameObject::Despawn, ( GetInfo()->respawn_time + RandomUInt( GetInfo()->respawn_time ) ), EVENT_GAMEOBJECT_EXPIRE, 1, 1, 0 ); 
		sEventMgr.AddEvent( this, &GameObject::Despawn, GetInfo()->respawn_time , EVENT_GAMEOBJECT_EXPIRE, 1, 1, 0 ); 
}

void GameObject::FishHooked(uint64 guid)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if( IsInWorld() )
	{
		Player *player = GetMapMgr()->GetPlayer( guid );
		if( player && player->GetSession() )
		{
			sStackWolrdPacket(  data, SMSG_GAMEOBJECT_CUSTOM_ANIM, 50); 
			data << GetGUID();
			data << (uint32)(0); // value < 4
			player->GetSession()->SendPacket(&data);
		}
	}
	//SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, 0);
	//BuildFieldUpdatePacket(player, GAMEOBJECT_FLAGS, GAMEOBJECT_CLICKABLE);
	SetUInt32Value(GAMEOBJECT_FLAGS, GAMEOBJECT_CLICKABLE);
 }

/////////////
/// Quests

void GameObject::AddQuest(QuestRelation *Q)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	m_quests->push_back(Q);
}

void GameObject::DeleteQuest(QuestRelation *Q)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	list<QuestRelation *>::iterator it;
	for( it = m_quests->begin(); it != m_quests->end(); ++it )
	{
		if( ( (*it)->type == Q->type ) && ( (*it)->qst == Q->qst ) )
		{
			delete (*it);
			(*it) = NULL;
			m_quests->erase(it);
			break;
		}
	}
}

Quest* GameObject::FindQuest(uint32 quest_id, uint8 quest_relation)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
   
	list< QuestRelation* >::iterator it;
	for( it = m_quests->begin(); it != m_quests->end(); ++it )
	{
		QuestRelation* ptr = (*it);
		if( ( ptr->qst->id == quest_id ) && ( ptr->type & quest_relation ) )
		{
			return ptr->qst;
		}
	}
	return NULL;
}

uint16 GameObject::GetQuestRelation(uint32 quest_id)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	uint16 quest_relation = 0;
	list< QuestRelation* >::iterator it;
	for( it = m_quests->begin(); it != m_quests->end(); ++it )
	{
		if( (*it) != NULL && (*it)->qst->id == quest_id )
		{
			quest_relation |= (*it)->type;
		}
	}
	return quest_relation;
}

uint32 GameObject::NumOfQuests()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	return (uint32)m_quests->size();
}

void GameObject::_LoadQuests()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	sQuestMgr.LoadGOQuests(this);
}

/////////////////
// Summoned Go's

void GameObject::_Expire()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	sEventMgr.RemoveEvents(this);
	if(IsInWorld())
		RemoveFromWorld(true);

	sGarbageCollection.AddObject( this );
}

void GameObject::ExpireAndDelete()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if( !IsGameObject() )
	{
		sLog.outDebug(" GameObject::Despawn : !!! omg invalid call on wrong object !\n");
		ASSERT(false);
		return;
	}
	if(m_deleted)
	{	 
		return;
	}

	m_deleted = true;
	
	/* remove any events */
	sEventMgr.RemoveEvents(this);
	if( IsInWorld() )
		sEventMgr.AddEvent(this, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
	else
		_Expire();
}

void GameObject::Deactivate()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
}

void GameObject::CallScriptUpdate()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	ASSERT(myScript);
	if( myScript )
		myScript->AIUpdate();
}

void GameObject::OnPushToWorld()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	Object::OnPushToWorld();
	CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnGameObjectPushToWorld )( this );
}

void GameObject::OnRemoveInRangeObject(Object* pObj)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	Object::OnRemoveInRangeObject(pObj);
	if(m_summonedGo && m_summoner == pObj)
	{
		for(int i = 0; i < 4; i++)
			if (m_summoner->m_ObjectSlots[i] == GetLowGUID())
				m_summoner->m_ObjectSlots[i] = 0;

		m_summoner = 0;
		ExpireAndDelete();
	}
}

void GameObject::RemoveFromWorld(bool free_guid)
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	sStackWolrdPacket( data, SMSG_GAMEOBJECT_DESPAWN_ANIM, 10 );
	data << GetGUID();
	SendMessageToSet(&data,true);

	sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET);
	Object::RemoveFromWorld(free_guid);
}

bool GameObject::HasLoot()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
    int count=0;
    for(vector<__LootItem>::iterator itr = loot.items.begin(); itr != loot.items.end(); ++itr)
	{
		if( itr->item.itemproto->Bonding == ITEM_BIND_QUEST || itr->item.itemproto->Bonding == ITEM_BIND_QUEST2 )
			continue;

		count += (itr)->iItemsCount;
	}

    return (count>0);

}

uint32 GameObject::GetGOReqSkill()  
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE
	if(GetEntry() == 180215) 
	{ 
		return 300;
	}

	if(GetInfo() == NULL)
	{ 
		return 0;
	}

	Lock *lock = dbcLock.LookupEntry( GetInfo()->SpellFocus );
	if(!lock) 
	{ 
		return 0;
	}
	for(uint32 i=0;i<5;i++)
		if(lock->locktype[i] == GO_LOCKTYPE_SKILL_REQ && lock->minlockskill[i])
		{
			return lock->minlockskill[i];
		}
	return 0;
}

void GameObject::UpdateGoOrientation()
{
	INSTRUMENT_TYPECAST_CHECK_GO_OBJECT_TYPE

	m_rotation = 0;

	//normalize it : 0 - 6.28
	float ori = GetOrientation();
	if( ori > 6.28f )
		ori = ori - ((int)(ori/6.28f))*6.28f;
	if( ori < 0.0f )
		ori = ori - ((int)(ori/6.28f))*6.28f;
	//convert it to range : [-3.14,3.14]
	float new_ori;
	if( ori > 3.14f )
		new_ori = ori - 6.14f;
	else
		new_ori = ori;
	SetOrientation( new_ori );

	float ang = GetOrientation();
    static double const atan_pow = atan(pow(2.0f, -20.0f));
	float f_rot1; 
	if( ang < 0 )
		f_rot1 = 1 + sin( ((float)M_PI+ang) / 2.3648f )*(1.0f + ang / (float)M_PI); //there must be a much much better way for this. Just could not find it yet :(
	else
		f_rot1 = sin(ang / 2.0f); 
	int64 i_rot1 = (int64)( f_rot1 / atan_pow );
	m_rotation |= (i_rot1 << 43 >> 43) & 0x00000000001FFFFF;
	//float f_rot2 = sin(0.0f / 2.0f);
	//int64 i_rot2 = f_rot2 / atan(pow(2.0f, -20.0f));
	//rotation |= (((i_rot2 << 22) >> 32) >> 11) & 0x000003FFFFE00000;

	//float f_rot3 = sin(0.0f / 2.0f);
	//int64 i_rot3 = f_rot3 / atan(pow(2.0f, -21.0f));
	//rotation |= (i_rot3 >> 42) & 0x7FFFFC0000000000;

	//from blizz : 
	//2.7838	1031841
	//2.60926	1011652
	//2.43473	983765
	//2.37364	972222
	//1.78896	817768
	//1.56207	738213
	//1.48353	708407
	//1.25664	616337
	//1.24791	612630
	//0.863937	438996
	//0.67195	345705
	//0.575957	297811
	//0.0698136	36595
	//0.0261791	13725

	//-3.11539	1048666
	//-2.82743	1061486
	//-2.75761	1067842
	//-2.46964	1107203
	//-2.23402	1154698
	//-2.08567	1191355
	//-2.03331	1205494
	//-1.69297	1311815
	//-1.46608	1395518
	//-0.98611	1600841
	//-0.890117	1645729
	//-0.680677	1747131
	//-0.445059	1865734
}

void GameObject::SetRotation(float rad)
{
	if (rad > (float)M_PI)
		rad -= 2*(float)M_PI;
	else if (rad < -(float)M_PI)
		rad += 2*(float)M_PI;
	float sin = sinf(rad/2.f);

	if(sin >= 0)	
		rad = 1.f + 0.125f * sin;
	else
		rad = 1.25f + 0.125f * sin;
//	SetFloatValue(GAMEOBJECT_ROTATION, rad);
}

void GameObject::UpdateRotation()
{
	static double const atan_pow = atan(pow(2.0f, -20.0f));

	double f_rot1 = sin(GetOrientation() / 2.0f);
	double f_rot2 = cos(GetOrientation() / 2.0f);

	int64 i_rot1 = int64(f_rot1 / atan_pow *(f_rot2 >= 0 ? 1.0f : -1.0f));
	int64 rotation = (i_rot1 << 43 >> 43) & 0x00000000001FFFFF;

	m_rotation = rotation;

	float r2=GetParentRotation(2);
	float r3=GetParentRotation(3);
	if(r2== 0.0f && r3== 0.0f)
	{
		r2 = (float)f_rot1;
		r3 = (float)f_rot2;
		SetParentRotation(2, r2);
		SetParentRotation(3, r3);
	}
}
 
void GameObject::TakeDamage(uint32 ammount, Object* mcaster, Object* pcaster, uint32 spellid)
{
	if(pInfo->Type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
		return;

	if(Health > ammount)
		Health -= ammount;
	else if(Health < ammount)
		Health = 0;

	if(Health == 0)
		return;

	if(HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED) && !HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED))
	{
		if(Health <= 0)
		{
			RemoveFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED);
			SetFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED);
			SetUInt32Value(GAMEOBJECT_DISPLAYID,pInfo->Unknown1);
		}
	}
	else if(!HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED) && !HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED))
	{
		if(Health <= pInfo->sound5)
		{
			if(pInfo->Unknown1 == 0)
				Health = 0;
			else if(Health == 0)
				Health = 1;
			SetFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED);
			SetUInt32Value(GAMEOBJECT_DISPLAYID,pInfo->sound4);
		}
	}

	WorldPacket data(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 20);
	data << mcaster->GetNewGUID() << pcaster->GetNewGUID();
	data << uint32(ammount) << spellid;
	mcaster->SendMessageToSet(&data, (mcaster->IsPlayer() ? true : false));
}

void GameObject::Rebuild()
{
	RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED);
	RemoveFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED);
	SetUInt32Value(GAMEOBJECT_DISPLAYID, pInfo->DisplayID);
	Health = pInfo->SpellFocus + pInfo->sound5;
}
