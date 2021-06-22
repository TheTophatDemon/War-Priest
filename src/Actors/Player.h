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
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include "../SoundSounder.h"
#include "Enemy.h"

using namespace Urho3D;

class Gameplay;
class Actor;
class Player : public LogicComponent
{
	friend class Gameplay;
	URHO3D_OBJECT(Player, LogicComponent);
public:
	Player(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Player();
	
	bool cheating = false;
	static const float reviveCooldownMax;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnProjectileHit(StringHash eventType, VariantMap& eventData);
	void OnBeamed(StringHash eventType, VariantMap& eventData);
	void OnSettingsChange(StringHash eventType, VariantMap& eventData);
	void OnCutsceneEvent(StringHash eventType, VariantMap& eventData);
	void OnKeyPress(StringHash eventType, VariantMap& eventData);
	void OnCheatWindowEvent(StringHash eventType, VariantMap& eventData);
	void Hurt(Node* source, int damage);
	void HandleCamera(const float timeStep);
	void HandleShadow();
	void ChangeState(int newState);
	void EnterState(int newState);
	void LeaveState(int oldState);
	void ST_Default(float timeStep);
	void ST_Slide(float timeStep);
	void ST_Win(float timeStep);
	void ST_Drown(float timeStep);
	void HandleNearestCorpse();

	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;
	WeakPtr<Gameplay> game;
	WeakPtr<PhysicsWorld> physworld;
	SharedPtr<Node> cameraNode;
	SharedPtr<Camera> camera;
	SharedPtr<Actor> actor;
	SharedPtr<RigidBody> body;
	SharedPtr<Node> modelNode;
	SharedPtr<AnimationController> animController;
	SharedPtr<Node> dropShadow;
	SharedPtr<RigidBody> groundDetector;
	SharedPtr<ParticleEmitter> bloodEmitter;
	SharedPtr<CollisionShape> shape;
	SharedPtr<Input> input;
	SharedPtr<Node> pivot;
	SharedPtr<SoundSounder> soundSource;
	SharedPtr<Window> cheatWindow;
	WeakPtr<Enemy> nearestCorpse; //Weakptr instead of Sharedptr avoids big-ass memory leak.
	WeakPtr<Node> currentCheckpoint;
	WeakPtr<Node> splashNode;
	WeakPtr<Node> beamNode;
	WeakPtr<Node> arrowNode;
	
	Vector3 slideDirection;
	Vector3 startingPosition;

	static const Matrix3x4 cameraOffset;

	String cheatString;
	
	int state;
	int health;
	int reviveCount = 0;
	int lastState;
	int drownPhase;
	float stateTimer;
	float reviveCooldown;
	float cameraPitch;
	float cameraYaw;
	float flexTimer;
	float hurtTimer;

	float walkSpeed;
	float slideSpeed;

	bool speedy = false;
	bool lastChance = false;
	bool revived = false;
	bool hovering = false;
	bool firstPerson = false;
};

