#include "GravityPlate.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "Gameplay.h"
#include "GunPriest.h"

float GravityPlate::p_constant = 0.1f;
float GravityPlate::i_constant = 0.0f;
float GravityPlate::i_max = 200.0f;
float GravityPlate::d_constant = 0.1f;
float GravityPlate::final_damp = 10.0f;

GravityPlate::GravityPlate(Context* context) : LogicComponent(context),
	rectifyTimer(0.0f),
	stopTimer(0.0f),
	lastRot(Quaternion::IDENTITY)
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
	
	SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(GravityPlate, OnCollision));
}

void GravityPlate::FixedUpdate(float timeStep)
{
	const Quaternion rot = node_->GetWorldRotation();
	rectifyTimer += timeStep;
	if (rectifyTimer > 4.0f) 
	{
		//There's a PID loop going on here
		const Vector3 p = -rot.EulerAngles();
		//const float i_f = Clamp(rectifyTimer - 2.0f, -i_max, i_max);
		//const Vector3 i = Vector3(i_f, i_f, i_f);
		//const Vector3 d = body->GetAngularVelocity();
		const Vector3 i = Vector3::ZERO;
		const Vector3 d = Vector3::ZERO;
		body->ApplyTorqueImpulse((p * p_constant + i * i_constant + d * d_constant) * timeStep * final_damp * body->GetMass());
	}
	lastRot = rot;

	/*game->gunPriest->debugRenderer->AddLine(body->GetPosition(), body->GetPosition() + body->GetAngularVelocity().Normalized(), (255 << 24) | (255 << 16) | 255, false);
	game->gunPriest->debugRenderer->AddLine(body->GetPosition(), body->GetPosition() + body->GetRotation().Axis() * body->GetRotation().w_, (255 << 24) | (255 << 8) | 255, false);
	game->gunPriest->debugRenderer->AddLine(body->GetPosition(), body->GetPosition() + node_->GetWorldDirection(), (255 << 24) | 255, false);*/
}

void GravityPlate::OnCollision(StringHash eventType, VariantMap& eventData)
{
	rectifyTimer = 0.0f;
	stopTimer = 0.0f;
	/*static float minRot = 1000000.0f;
	static float maxRot = -1000000.0f;
	const Vector3 angles = node_->GetWorldRotation().EulerAngles(); //In degrees
	minRot = Min(minRot, angles.x_);
	minRot = Min(minRot, angles.y_);
	minRot = Min(minRot, angles.z_);
	maxRot = Max(maxRot, angles.x_);
	maxRot = Max(maxRot, angles.y_);
	maxRot = Max(maxRot, angles.z_);
	std::cout << minRot << ", " << maxRot << std::endl;*/
}

GravityPlate::~GravityPlate()
{
}
