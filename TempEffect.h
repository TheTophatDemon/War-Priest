#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

using namespace Urho3D;

class TempEffect : public LogicComponent
{
	URHO3D_OBJECT(TempEffect, LogicComponent);
public:
	TempEffect(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~TempEffect();
	float life = 0.0f;
protected:
	float timer = 0.0f;
};

