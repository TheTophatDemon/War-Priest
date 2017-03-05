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
#include "Actor.h"
#include "NPC.h"

using namespace Urho3D;

#define PITCH_LIMIT 45

Player::Player(Context* context) : LogicComponent(context)
{
	cameraPitch = 0.0f;
}

void Player::RegisterObject(Context* context)
{
	context->RegisterFactory<Player>();
}

void Player::Start()
{
	node_->SetVar("Cross Count", 0);

	game = GetScene()->GetComponent<Gameplay>();
	body = node_->GetComponent<RigidBody>();
	scene = GetScene();
	cameraNode = node_->GetChild("camera");
	camera = cameraNode->GetComponent<Camera>();
	arms = cameraNode->GetChild("arms");

	if (!node_->HasComponent<Actor>())
	{
		actor = node_->CreateComponent<Actor>();
	}
	else
	{
		actor = node_->GetComponent<Actor>();
	}
	
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
	bool forwardKey = input->GetKeyDown(scene->GetVar("FORWARD KEY").GetInt());
	bool backwardKey = input->GetKeyDown(scene->GetVar("BACKWARD KEY").GetInt());
	bool rightKey = input->GetKeyDown(scene->GetVar("RIGHT KEY").GetInt());
	bool leftKey = input->GetKeyDown(scene->GetVar("LEFT KEY").GetInt());
	bool jumpKey = input->GetKeyDown(scene->GetVar("JUMP KEY").GetInt());
	actor->Move(forwardKey, backwardKey, rightKey, leftKey, jumpKey, timeStep);

	float sensitivity = scene->GetVar("MOUSE SENSITIVITY").GetFloat();
	cameraPitch += input->GetMouseMoveY() * sensitivity;
	if (cameraPitch > PITCH_LIMIT) cameraPitch = PITCH_LIMIT;
	if (cameraPitch < -PITCH_LIMIT) cameraPitch = -PITCH_LIMIT;
	node_->Rotate(Quaternion(input->GetMouseMoveX() * sensitivity, Vector3::UP));
	cameraNode->SetRotation(Quaternion(cameraPitch, Vector3::RIGHT));
}

void Player::FireWeapon()
{
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, camera->GetScreenRay(0.5f, 0.5f), 650.0f, 4U);
	if (result.body_)
	{
		Node* node = result.body_->GetNode();
		if (node->HasComponent<NPC>())
		{
			NPC* npc = node->GetComponent<NPC>();
			npc->ChangeState(2, 1000);
			result.body_->ApplyImpulse((Vector3::UP * 500.0f) + (-result.normal_ * 950.0f));
			result.body_->SetAngularVelocity(Vector3::ONE * 2.5f);
			game->FlashScreen(Color(1.0f, 0.0f, 0.0f, 0.7f), 0.02f);
		}
	}
}

void Player::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
}

void Player::OnAnimTrigger(StringHash eventType, VariantMap& eventData)
{
	int command = eventData["Data"].GetInt();
	switch (command)
	{
	case 0:
		leftMuzzleFlash->SetEmitting(true);
		FireWeapon();
		break;
	case 1:
		leftMuzzleFlash->SetEmitting(false);
		break;
	case 2:
		rightMuzzleFlash->SetEmitting(true);
		FireWeapon();
		break;
	case 3:
		rightMuzzleFlash->SetEmitting(false);
		break;
	}
}

Player::~Player()
{
}
