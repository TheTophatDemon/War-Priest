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
#include <Urho3D/Container/Vector.h>

#include "Enemy.h"

using namespace Urho3D;

class Actor;
class Gameplay;
class PostalPope : public Enemy
{
	URHO3D_OBJECT(PostalPope, LogicComponent);
public:
	PostalPope(Context* context);
	static void RegisterObject(Context* context);
	virtual void Revive() override;
	virtual void DelayedStart() override;
	~PostalPope();
protected:
	static const int STATE_SUMMON = 32;
	static const int STATE_THROW = 33;

	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;
	void SummonDebris();
	bool CanSummon();
	Vector<Node*> debris;
	SharedPtr<Node> spinner;
	SharedPtr<SoundSource> weeoo;
	bool summoned = false;
};

