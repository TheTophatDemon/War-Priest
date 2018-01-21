#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Resource/ResourceCache.h>
#include "Gameplay.h"

using namespace Urho3D;

class Gameplay;
class Lift : public LogicComponent
{
	URHO3D_OBJECT(Lift, LogicComponent);
public:
	Lift(Context* context);
	static void RegisterObject(Context* context);
	static Lift* MakeLiftComponent(Context* context, const Vector3 mov, const float rT, const float spd, const float rSpd, const float aRad);
	virtual void FixedUpdate(float timeStep);
	virtual void Start();
	~Lift();
	Vector3 movement;
	//Vector3 unitMovement;
	float restTime = 0.0f;
	float speed = 1.0f;
	float rotateSpeed = 0.0f;
	float activeRadius = 0.0f;
protected:
	WeakPtr<Gameplay> game;
	SharedPtr<ValueAnimation> valAnim;
	void OnCollision(StringHash eventType, VariantMap& eventData);
	float timer = 0.0f;
};

