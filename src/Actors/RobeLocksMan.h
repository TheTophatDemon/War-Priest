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
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>

#include "Enemy.h"
#include "../SoundSounder.h"

enum class Weapon
{
	Launcher,
	Bomb,
	PaintGun
};

class RobeLocksMan : public Enemy
{
	URHO3D_OBJECT(RobeLocksMan, LogicComponent);
public:
	RobeLocksMan(Context* context);
	static void RegisterObject(Context* context);
	virtual void DelayedStart() override;
	virtual void Revive() override;
	~RobeLocksMan();
protected:
	void OnProjectileHit(StringHash eventType, VariantMap& eventData);
	virtual void Wander(bool pause = false, float slopeIntolerance = 0.5f, float wallMargin = 2.0f) override;
	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;

	SharedPtr<SoundSounder> soundSource;
	SharedPtr<Node> weaponNode;
	SharedPtr<StaticModel> weaponModel;
	Weapon currentWeapon;
	float shootTimer;
	float chaseTimer;
};

