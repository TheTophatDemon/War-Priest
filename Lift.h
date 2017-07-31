#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Lift : public LogicComponent
{
	URHO3D_OBJECT(Lift, LogicComponent);
public:
	Lift(Context* context);
	static void RegisterObject(Context* context);
	static Lift* MakeLiftComponent(Context* context, Vector3 mov, float rT, float spd, float rSpd);
	virtual void FixedUpdate(float timeStep);
	virtual void Start();
	~Lift();
	Vector3 movement;
	//Vector3 unitMovement;
	float restTime = 0.0f;
	float speed = 1.0f;
	float rotateSpeed = 0.0f;
protected:
	SharedPtr<ValueAnimation> valAnim;
	void OnCollision(StringHash eventType, VariantMap& eventData);
	float timer = 0.0f;
};

