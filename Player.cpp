#include "Player.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/DrawableEvents.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D\Physics\PhysicsUtils.h>
#include <Urho3D/Math/Matrix3x4.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D\Resource\ResourceCache.h>

#include <iostream>

#include "Gameplay.h"

using namespace Urho3D;

#define PITCH_LIMIT 45
#define ACCELERATION 1.0f
#define MAXSPEED 10.0f
#define FRICTION 0.85f
#define FALLSPEED 0.4f
#define MAXFALL 120.0f
#define JUMP 12.0f

Player::Player(Context* context) : LogicComponent(context)
{
	cameraPitch = 0.0f;
	onGround = false;
	slopeSteepness = 0.0f;
}

void Player::RegisterObject(Context* context)
{
	context->RegisterFactory<Player>();
}

void Player::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	body = node_->GetComponent<RigidBody>();
	scene = GetScene();
	cameraNode = node_->GetChild("camera");
	arms = cameraNode->GetChild("arms");
	
	if (arms) 
	{
		AnimationController* controller = arms->GetComponent<AnimationController>();
		controller->PlayExclusive("Models/Arms Rig.ani", 255, true, 0.0f);
		leftMuzzleFlash = arms->GetChild("gun.l", true)->GetChild(0U)->GetComponent<ParticleEmitter>();
		leftMuzzleFlash->SetEmitting(false);
		rightMuzzleFlash = arms->GetChild("gun.r", true)->GetChild(0U)->GetComponent<ParticleEmitter>();
		rightMuzzleFlash->SetEmitting(false);
	}
	physworld = scene->GetComponent<PhysicsWorld>();
	
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Player, OnCollision));
	SubscribeToEvent(arms, E_ANIMATIONTRIGGER, URHO3D_HANDLER(Player, OnAnimTrigger));
}

void Player::FixedUpdate(float timeStep)
{
	
	DoMovement(timeStep);

	float sensitivity = scene->GetVar("MOUSE SENSITIVITY").GetFloat();
	cameraPitch += input->GetMouseMoveY() * sensitivity;
	if (cameraPitch > PITCH_LIMIT) cameraPitch = PITCH_LIMIT;
	if (cameraPitch < -PITCH_LIMIT) cameraPitch = -PITCH_LIMIT;
	node_->Rotate(Quaternion(input->GetMouseMoveX() * sensitivity, Vector3::UP));
	cameraNode->SetRotation(Quaternion(cameraPitch, Vector3::RIGHT));
}

void Player::DoMovement(float timeStep)
{
	if (input->GetKeyDown(scene->GetVar("FORWARD KEY").GetInt()))
	{
		forward += ACCELERATION;
		if (forward > MAXSPEED) forward = MAXSPEED;
	}
	else if (input->GetKeyDown(scene->GetVar("BACKWARD KEY").GetInt()))
	{
		forward -= ACCELERATION;
		if (forward < -MAXSPEED) forward = -MAXSPEED;
	}
	else
	{
		forward *= FRICTION;
		if (fabs(forward) < 0.1f) forward = 0.0f;
	}

	if (input->GetKeyDown(scene->GetVar("RIGHT KEY").GetInt()))
	{
		strafe += ACCELERATION;
		if (strafe > MAXSPEED) strafe = MAXSPEED;
	}
	else if (input->GetKeyDown(scene->GetVar("LEFT KEY").GetInt()))
	{
		strafe -= ACCELERATION;
		if (strafe < -MAXSPEED) strafe = -MAXSPEED;
	}
	else
	{
		strafe *= FRICTION;
		if (fabs(strafe) < 0.1f) strafe = 0.0f;
	}

	fall -= FALLSPEED;
	if (fall < -MAXFALL) fall = -MAXFALL;
	if (onGround)
	{
		if (slopeSteepness != 0.0f)
		{
			fall = ((-1 / slopeSteepness) + 1) * MAXSPEED;
		}
		else
		{
			fall = 0.0f;
		}	
	}
	if (input->GetKeyDown(scene->GetVar("JUMP KEY").GetInt()) && onGround)
	{
		fall = JUMP;
	}
	else if (!input->GetKeyDown(scene->GetVar("JUMP KEY").GetInt()) && fall > 0.0f)
	{
		fall -= 0.5f;
	}

	body->SetLinearVelocity((node_->GetRotation() * Vector3(strafe, fall, forward) * timeStep * 50.0f));
	onGround = false;
	slopeSteepness = 0.0f;
	GetSlope();
}

void Player::GetSlope()
{
	//Raycast downward to get slope normal
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.5f, 0.0f), Vector3::DOWN), 500.0f, 2);
	if (result.body_)
	{
		slopeSteepness = result.normal_.y_ * 0.75f;
	}
}

void Player::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 2)
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			if (fabs(normal.y_) > 0.1f)
			{
				if (position.y_ <= node_->GetPosition().y_ + 0.5f) 
				{
					onGround = true;
				}
				if (position.y_ >= node_->GetPosition().y_ + 2.5f && fall > 0.0f && distance < 0.005f)
				{
					fall = 0.0f;
				}
			}
		}
	}
}

void Player::OnAnimTrigger(StringHash eventType, VariantMap& eventData)
{
	int command = eventData["Data"].GetInt();
	switch (command)
	{
	case 0:
		leftMuzzleFlash->SetEmitting(true);
		break;
	case 1:
		leftMuzzleFlash->SetEmitting(false);
		break;
	case 2:
		rightMuzzleFlash->SetEmitting(true);
		break;
	case 3:
		rightMuzzleFlash->SetEmitting(false);
		break;
	}
}

Player::~Player()
{
}
