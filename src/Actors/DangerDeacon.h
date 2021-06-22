/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/StaticModel.h>

#include "Enemy.h"

using namespace Urho3D;

class Actor;
class DangerDeacon : public Enemy //He's dangerous. He's out of control!
{
	URHO3D_OBJECT(DangerDeacon, LogicComponent);
public:
	DangerDeacon(Context* context);
	static void RegisterObject(Context* context);
	virtual void DelayedStart() override;
	virtual void Revive() override;
	~DangerDeacon();
protected:
	static const int STATE_CHASE = 32;
	static const int STATE_EXPLODE = 33;
	static const int STATE_POSE = 34;

	static float EXPLODE_RANGE;
	static float BLAST_RANGE;
	static float STUN_TIME;
	static float DAMAGE;

	void OnSettingsChange(StringHash eventType, VariantMap& eventData);
	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;
	SharedPtr<Node> orbThing;
	SharedPtr<StaticModel> orbModel;
	float strafeAmt;
	float fallTimer;
};

