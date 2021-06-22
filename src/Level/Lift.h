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
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Resource/ResourceCache.h>
#include "../Gameplay.h"

using namespace Urho3D;

class Gameplay;
class Lift : public LogicComponent
{
	URHO3D_OBJECT(Lift, LogicComponent);
public:
	Lift(Context* context);
	static void RegisterObject(Context* context);
	virtual void FixedUpdate(float timeStep);
	virtual void Start();
	~Lift();
	Vector3 pointB;
	Vector3 pointA;
	float restTime = 0.0f;
	float speed = 1.0f;
	float rotateSpeed = 0.0f;
	float activeRadius = 0.0f;
	bool wait = false;
protected:
	void OnCollisionStart(StringHash eventType, VariantMap& eventData);
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void ChangeState(const int newState);
	void SetTarget(Vector3& targ);
	
	SharedPtr<ValueAnimation> blinkAnimation;

	Vector<Pair<WeakPtr<Node>, float>> childCache; //Keeps track of all actors on top of the platform so they can be moved with it each frame.
	//The float in each pair keeps track of how much time it has been since the actor stopped colliding with the platform.
	//It is neccessary because the physics engine periodically "forgets" that the actor is on the platform for a frame or two, resulting in jittering.

	SharedPtr<RigidBody> body;
	WeakPtr<Gameplay> game;
	SharedPtr<Node> glowNode;
	SharedPtr<Material> glowMat;
	Matrix3x4 oldTransform;
	Vector3 target;
	Vector3 moveDir;
	int rotateDirection;
	
	int state;
	float stateTimer;
};

