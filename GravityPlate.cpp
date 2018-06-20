#include "GravityPlate.h"

#include <Urho3D/Core/Context.h>

GravityPlate::GravityPlate(Context* context) : LogicComponent(context)
{
}

void GravityPlate::RegisterObject(Context* context)
{
	context->RegisterFactory<GravityPlate>();
}

void GravityPlate::Start()
{
	body = node_->GetComponent<RigidBody>();
}

void GravityPlate::FixedUpdate(float timeStep)
{
	//const float diff = node_->GetWorldDirection().DotProduct(Vector3(0.0f, 0.0f, 1.0f));
	body->ApplyTorque(-body->GetAngularVelocity() * timeStep * 10.0f);
}

GravityPlate::~GravityPlate()
{
}
