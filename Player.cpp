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
#include "Enemy.h"

using namespace Urho3D;

#define PITCH_LIMIT_HIGHER 65
#define PITCH_LIMIT_LOWER 15

#define STATE_DEFAULT 0
#define STATE_REVIVE 1
#define STATE_SLIDE 2
#define STATE_DEAD 3

#define WALKSPEED 15.0f
#define SLIDESPEED 20.0f

Player::Player(Context* context) : LogicComponent(context)
{
	hailTimer = 0;
	stateTimer = 0;
	hurtTimer = 0;
	state = STATE_DEFAULT;
	health = 100;
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
	actor->maxspeed = WALKSPEED;

	//Get Model
	modelNode = node_->GetChild("model");
	if (!modelNode)
		std::cout << "PLAYER HAS NO MODEL!" << std::endl;
	newRotation = modelNode->GetRotation();
	node_->RemoveChild(modelNode);
	scene->AddChild(modelNode);
	animController = modelNode->CreateComponent<AnimationController>();
	animController->PlayExclusive("Models/grungle_default.ani", 0, true, 0.0f);

	//Drop Shadow
	dropShadow = scene->CreateChild();
	StaticModel* shadModel = dropShadow->CreateComponent<StaticModel>();
	shadModel->SetModel(cache->GetResource<Model>("Models/shadow.mdl"));
	shadModel->SetMaterial(cache->GetResource<Material>("Materials/shadow.xml"));

	bloodEmitter = node_->GetChild("blood")->GetComponent<ParticleEmitter>();
	bloodEmitter->SetEmitting(false);
	
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Player, OnCollision));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Player, PostUpdate));
}

void Player::FixedUpdate(float timeStep)
{
	//Shadow
	PhysicsRaycastResult shadowRaycast;
	Vector3 doot = Vector3(0.0f, 0.1f, 0.0f);
	physworld->RaycastSingle(shadowRaycast, Ray(node_->GetWorldPosition() + doot, Vector3::DOWN), 500.0f, 2);
	HandleShadow(shadowRaycast);

	bool forwardKey = input->GetKeyDown(scene->GetGlobalVar("FORWARD KEY").GetInt());
	bool backwardKey = input->GetKeyDown(scene->GetGlobalVar("BACKWARD KEY").GetInt());
	bool rightKey = input->GetKeyDown(scene->GetGlobalVar("RIGHT KEY").GetInt());
	bool leftKey = input->GetKeyDown(scene->GetGlobalVar("LEFT KEY").GetInt());
	bool jumpKey = input->GetKeyDown(scene->GetGlobalVar("JUMP KEY").GetInt());
	float newAngle = 0.0f;

	switch (state) 
	{
	case STATE_DEFAULT: ///////////////////////////////////////////////////////////////////////////////////////////
		actor->Move(forwardKey, backwardKey, rightKey, leftKey, jumpKey, timeStep);
		stateTimer += timeStep;
		//Decide what angle the model will be facing
		if (rightKey || leftKey || forwardKey || backwardKey)
		{
			Quaternion mov = Quaternion(); mov.FromLookRotation(actor->movement.Normalized());
			newRotation = Quaternion(0.0f, mov.EulerAngles().y_ - 90.0f, 0.0f);
			node_->SetRotation(pivot->GetRotation());
		}

		if (input->GetMouseButtonDown(MOUSEB_LEFT))
		{
			ChangeState(STATE_REVIVE);
		}
		else if (input->GetMouseButtonDown(MOUSEB_RIGHT) && actor->onGround && stateTimer > 0.5f)
		{
			ChangeState(STATE_SLIDE);
		}

		//Select Animation
		if (shadowRaycast.distance_ > 0.15f + (1.0f / actor->slopeSteepness) || !shadowRaycast.body_)
		{
			animController->PlayExclusive("Models/grungle_jump.ani", 0, false, 0.2f);
		}
		else
		{
			if (forwardKey || backwardKey || leftKey || rightKey)
			{
				animController->PlayExclusive("Models/grungle_walk.ani", 0, true, 0.2f);
				hailTimer = 0;
			}
			else
			{
				hailTimer += 1;
				if (hailTimer == 500)
				{
					animController->PlayExclusive("Models/grungle_hailmary.ani", 0, false, 0.2f);
				}
				else if (hailTimer < 500)
				{
					animController->PlayExclusive("Models/grungle_idle.ani", 0, true, 0.2f);
				}
				else if (animController->IsAtEnd("Models/grungle_hailmary.ani"))
				{
					hailTimer = 0;
				}
			}
		}
		break;
	case STATE_REVIVE: ////////////////////////////////////////////////////////////////////////////////////
		animController->PlayExclusive("Models/grungle_revive.ani", 0, true, 0.2f);
		stateTimer += 1;
		actor->Move(false, false, false, false, false, timeStep);
		if (animController->GetTime("Models/grungle_revive.ani") >= animController->GetLength("Models/grungle_revive.ani") * 0.9f)
		{
			ChangeState(STATE_DEFAULT);
		}
		if (stateTimer == 20) 
		{
			PODVector<Node*> enemies;
			scene->GetChildrenWithTag(enemies, "enemy", true);
			float smallestDistance = 10000.0f;
			Enemy* nearestEnemy = nullptr;
			for (PODVector<Node*>::Iterator i = enemies.Begin(); i != enemies.End(); ++i)
			{
				Node* enemy = (Node*)*i;
				if (enemy)
				{
					Enemy* e = enemy->GetDerivedComponent<Enemy>();
					if (e->state == 0) 
					{
						float dist = (enemy->GetWorldPosition() - node_->GetWorldPosition()).Length();
						if (dist < 8.0f)
						{
							if (dist < smallestDistance)
							{
								smallestDistance = dist;
								nearestEnemy = e;
							}
						}
					}
				}
			}
			if (nearestEnemy)
			{
				nearestEnemy->Revive();
			}
		}
		break;
	case STATE_SLIDE: /////////////////////////////////////////////////////////////////////////////////
		animController->PlayExclusive("Models/grungle_slide.ani", 0, false, 0.2f);
		actor->Move(true, false, false, false, false, timeStep);
		stateTimer += timeStep;
		if (stateTimer > 0.5f)
		{
			stateTimer = 0;
			ChangeState(STATE_DEFAULT);
		}
		break;
	}

	if (hurtTimer > 0)
	{
		hurtTimer -= 1;
		if (hurtTimer <= 0)
		{
			bloodEmitter->SetEmitting(false);
		}
	}
}

void Player::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
}

void Player::PostUpdate(StringHash eventType, VariantMap& eventData)
{
	modelNode->SetRotation(modelNode->GetRotation().Slerp(newRotation, 0.25f));
	modelNode->SetPosition(body->GetPosition());
	HandleCamera();
}

void Player::OnAnimTrigger(StringHash eventType, VariantMap& eventData)
{
	
}

void Player::OnHurt(Node* source, int amount)
{
	health -= amount;
	bloodEmitter->SetEmitting(true);
	hurtTimer = 20;
	if (source) 
	{
		actor->KnockBack(15.0f, source->GetWorldRotation());
	}
}

void Player::HandleCamera()
{
	if (scene->HasComponent<Gameplay>()) 
	{
		float sensitivity = scene->GetGlobalVar("MOUSE SENSITIVITY").GetFloat();
		pivot->Rotate(Quaternion(input->GetMouseMoveX() * sensitivity, Vector3::UP));
	}
	else
	{
		pivot->Rotate(Quaternion(0.1f, Vector3::UP));
	}
	Quaternion newAngle = Quaternion();
	newAngle.FromLookRotation((node_->GetWorldPosition() - cameraNode->GetWorldPosition()).Normalized());
	cameraNode->SetRotation(Quaternion(newAngle.EulerAngles().x_, Vector3::RIGHT));
	pivot->SetWorldPosition(Vector3(node_->GetWorldPosition().x_, 0.0f, node_->GetWorldPosition().z_));
}

void Player::HandleShadow(PhysicsRaycastResult result)
{
	if (result.body_)
	{
		dropShadow->SetEnabled(true);
		dropShadow->SetWorldPosition(result.position_ + Vector3(0.0f, 0.1f, 0.0f));
		Quaternion q = Quaternion();
		q.FromLookRotation(result.normal_);
		Vector3 eul = q.EulerAngles();
		q.FromEulerAngles(eul.x_ + 90.0f, eul.y_, eul.z_);
		dropShadow->SetRotation(q);
	}
	else
	{
		dropShadow->SetEnabled(false);
	}
}

void Player::ChangeState(int newState)
{
	if (newState == STATE_SLIDE && state != STATE_SLIDE)
	{
		actor->maxspeed = SLIDESPEED;
		node_->SetRotation(Quaternion(0.0f, newRotation.EulerAngles().y_ + 90.0f, 0.0f));
	}
	else if (newState != STATE_SLIDE && state == STATE_SLIDE)
	{
		actor->maxspeed = WALKSPEED;
	}
	stateTimer = 0;
	state = newState;
}

Player::~Player()
{
}
