#include "GravityPlate.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "Gameplay.h"
#include "GunPriest.h"

GravityPlate::GravityPlate(Context* context) : LogicComponent(context),
	rotationDirection(Quaternion::IDENTITY),
	rotationAxis(Vector3::ZERO),
	rotationForce(1.0f)
{
}

void GravityPlate::RegisterObject(Context* context)
{
	context->RegisterFactory<GravityPlate>();
}

void GravityPlate::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	body = node_->GetComponent<RigidBody>();
}

void GravityPlate::FixedUpdate(float timeStep)
{
	rotationDirection.FromAngleAxis(rotationForce * timeStep, rotationAxis);
	node_->Rotate(rotationDirection, TS_LOCAL);
}

GravityPlate::~GravityPlate()
{
}
