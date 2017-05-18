#include "Lift.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <iostream>

Lift* Lift::MakeLiftComponent(Context* context, Vector3 mov, float rT, float spd, float rSpd)
{
	Lift* lift = new Lift(context);
	lift->movement = mov;
	lift->restTime = rT;
	lift->speed = spd;
	lift->rotateSpeed = rSpd;
	return lift;
}

Lift::Lift(Context* context) : LogicComponent(context)
{
}

void Lift::RegisterObject(Context* context)
{
	context->RegisterFactory<Lift>();
}

void Lift::Start()
{
	Vector3 pointA = node_->GetWorldPosition();
	Vector3 pointB = pointA + movement;
	SharedPtr<ValueAnimation> val(new ValueAnimation(context_));
	val->SetKeyFrame(0.0f, pointA);
	val->SetKeyFrame(restTime, pointA);
	val->SetKeyFrame(restTime + speed, pointB);
	val->SetKeyFrame((restTime * 2.0f) + speed, pointB);
	val->SetKeyFrame((restTime + speed) * 2.0f, pointA);
	node_->SetAttributeAnimation("Position", val, WM_LOOP, 1.0f);
}

void Lift::FixedUpdate(float timeStep)
{
	node_->Rotate(Quaternion(rotateSpeed * timeStep, Vector3::UP), TS_LOCAL);
}

Lift::~Lift()
{
}
