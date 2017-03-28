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
#include <Urho3D/Graphics/ParticleEffect.h>

#include <iostream>

#include "Gameplay.h"
#include "Actor.h"
#include "NPC.h"
#include "TempEffect.h"
#include "Boulder.h"

using namespace Urho3D;

#define PITCH_LIMIT_HIGHER 65
#define PITCH_LIMIT_LOWER 15

Player::Player(Context* context) : LogicComponent(context)
{
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
	cache = GetSubsystem<ResourceCache>(); 
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	cameraNode = game->cameraNode;
	camera = game->camera;
	
	cameraNode->SetRotation(Quaternion(55.0f, Vector3::RIGHT));

	if (!node_->HasComponent<Actor>())
	{
		actor = node_->CreateComponent<Actor>();
	}
	else
	{
		actor = node_->GetComponent<Actor>();
	}

	modelNode = node_->GetChild("model");
	if (!modelNode)
		std::cout << "PLAYER HAS NO MODEL!" << std::endl;
	newRotation = modelNode->GetRotation();
	node_->RemoveChild(modelNode);
	scene->AddChild(modelNode);
	
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Player, OnCollision));
}

void Player::FixedUpdate(float timeStep)
{
	bool forwardKey = input->GetKeyDown(scene->GetGlobalVar("FORWARD KEY").GetInt());
	bool backwardKey = input->GetKeyDown(scene->GetGlobalVar("BACKWARD KEY").GetInt());
	bool rightKey = input->GetKeyDown(scene->GetGlobalVar("RIGHT KEY").GetInt());
	bool leftKey = input->GetKeyDown(scene->GetGlobalVar("LEFT KEY").GetInt());
	bool jumpKey = input->GetKeyDown(scene->GetGlobalVar("JUMP KEY").GetInt());
	actor->Move(forwardKey, backwardKey, rightKey, leftKey, jumpKey, timeStep);

	//Decide what angle the model will be facing
	float newAngle = 0.0f;
	if (rightKey || leftKey || forwardKey || backwardKey) 
	{
		if (forwardKey)
		{
			if (rightKey)
				newAngle = 45.0f;
			if (leftKey)
				newAngle = -45.0f;
		}
		else if (backwardKey)
		{
			newAngle = 180.0f;
			if (rightKey)
				newAngle -= 45.0f;
			if (leftKey)
				newAngle += 45.0f;
		}
		else if (rightKey)
		{
			newAngle = 90.0f;
		}
		else if (leftKey)
		{
			newAngle = 270.0f;
		}
		newRotation = Quaternion(90.0f, node_->GetRotation().EulerAngles().y_ + newAngle, 0.0f);
		node_->SetRotation(pivot->GetRotation());
	}
	modelNode->SetRotation(modelNode->GetRotation().Slerp(newRotation, 0.25f));
	modelNode->SetPosition(node_->GetWorldPosition());

	HandleCamera();
}

void Player::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
}

void Player::OnAnimTrigger(StringHash eventType, VariantMap& eventData)
{
	
}

void Player::HandleCamera()
{
	float sensitivity = scene->GetGlobalVar("MOUSE SENSITIVITY").GetFloat();
	pivot->Rotate(Quaternion(input->GetMouseMoveX() * sensitivity, Vector3::UP));
	Quaternion newAngle = Quaternion();
	newAngle.FromLookRotation((node_->GetWorldPosition() - cameraNode->GetWorldPosition()).Normalized());
	cameraNode->SetRotation(Quaternion(newAngle.EulerAngles().x_, Vector3::RIGHT));
	pivot->SetWorldPosition(Vector3(node_->GetWorldPosition().x_, 0.0f, node_->GetWorldPosition().z_));
}

Player::~Player()
{
}
