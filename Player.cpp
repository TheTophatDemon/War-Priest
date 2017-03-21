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
	cameraPitch = 60.0f;
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
	cameraNode = game->cameraNode;
	camera = game->camera;
	cache = GetSubsystem<ResourceCache>();

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

	shrapnel = scene->CreateChild();
	shrapnelEmitter = shrapnel->CreateComponent<ParticleEmitter>();
	shrapnelEmitter->SetEffect(cache->GetResource<ParticleEffect>("Particles/shrapnel.xml"));
	shrapnelEmitter->SetEmitting(false);

	physworld = scene->GetComponent<PhysicsWorld>();
	
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

void Player::FireWeapon()
{
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, camera->GetScreenRay(0.495f, 0.5f), 35.0f, 127); //Raycast for everything except for the player
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
			node_->SetVar("CROSS COUNT", 666);
			game->Lose();
		}
		else if (node->HasComponent<Boulder>())
		{
			Vector3 imp = (Vector3::UP * 200.0f) + (-result.normal_ * 600.0f);
			result.body_->ApplyImpulse(imp);
		}
		//Make shrapnel
		shrapnel->SetWorldPosition(result.position_);
		Quaternion quaternion = Quaternion();
		quaternion.FromLookRotation(-result.normal_, Vector3::UP);
		shrapnel->SetRotation(quaternion);
		shrapnelEmitter->SetEmitting(true);
	}
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
	cameraPitch += input->GetMouseMoveY() * sensitivity;
	if (cameraPitch > PITCH_LIMIT_HIGHER) cameraPitch = PITCH_LIMIT_HIGHER;
	if (cameraPitch < PITCH_LIMIT_LOWER) cameraPitch = PITCH_LIMIT_LOWER;

	pivot->Rotate(Quaternion(input->GetMouseMoveX() * sensitivity, Vector3::UP));
	pivot->SetWorldPosition(node_->GetWorldPosition());
	cameraNode->SetRotation(Quaternion(cameraPitch, Vector3::RIGHT));

	Matrix3x4 helper = Matrix3x4(Vector3(0.0f, 9.0f, -6.0f), Quaternion::IDENTITY, 1.0f);
	helper = pivot->GetWorldTransform() * helper;
	Vector3 diff = (node_->GetWorldPosition() - helper.Translation());
	//Keep it from clipping outside of walls
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() - diff.Normalized(), -diff.Normalized()), 100.0f, 2);
	float distFromPly = diff.Length();
	if (result.body_ && result.distance_ < distFromPly)
	{
		newCameraPosition = result.position_ + result.normal_;
	}
	else
	{
		newCameraPosition = helper.Translation();
	}
	if ((cameraNode->GetWorldPosition() - newCameraPosition).Normalized().LengthSquared() < 16.0f)
	{
		cameraNode->SetWorldPosition(newCameraPosition.Lerp(cameraNode->GetWorldPosition(), 0.7f));
	}
	else
	{
		cameraNode->SetWorldPosition(newCameraPosition);
	}
}

Player::~Player()
{
}
