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
#include "Zeus.h"

using namespace Urho3D;

#define PITCH_LIMIT_HIGHER 65
#define PITCH_LIMIT_LOWER 15

#define STATE_DEFAULT 0
#define STATE_REVIVE 1
#define STATE_SLIDE 2
#define STATE_DEAD 3

#define ACCELERATION 150.0f
#define FRICTION 0.85f
#define FALLSPEED 50.0f
#define MAXFALL 150.0f
#define JUMPSTRENGTH 18.0f
#define WALKSPEED 15.0f
#define SLIDESPEED 20.0f

Vector3 Player::orgShapeSize = Vector3::ONE;
Vector3 Player::orgShapePos = Vector3(0.0f, 1.5f, 0.0f);

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
	shape = node_->GetComponent<CollisionShape>(); //The main shape is always first
	cache = GetSubsystem<ResourceCache>(); 
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	cameraNode = game->cameraNode;
	camera = game->camera;
	
	cameraNode->SetRotation(Quaternion(55.0f, Vector3::RIGHT));
	orgShapeSize = shape->GetSize();
	orgShapePos = shape->GetPosition();

	if (!node_->HasComponent<Actor>())
	{
		actor = node_->CreateComponent<Actor>();
	}
	else
	{
		actor = node_->GetComponent<Actor>();
	}
	actor->acceleration = ACCELERATION;
	actor->friction = FRICTION;
	actor->fallspeed = FALLSPEED;
	actor->maxfall = MAXFALL;
	actor->jumpStrength = JUMPSTRENGTH;
	actor->maxspeed = WALKSPEED;

	groundDetector = node_->GetChild("groundDetector")->GetComponent<RigidBody>();
	if (!groundDetector)
		std::cout << "GROUND DETECTOR MISSING" << std::endl;

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
	bool diddly = false;

	PODVector<RigidBody*> grounds;

	switch (state) 
	{
	case STATE_DEFAULT: ///////////////////////////////////////////////////////////////////////////////////////////
		stateTimer += timeStep;

		if (!actor->onGround && shadowRaycast.body_)
		{
			dropShadow->SetEnabled(true);
		}
		else
		{
			dropShadow->SetEnabled(false);
		}

		if (rightKey)
		{
			moveX += ACCELERATION * timeStep;
			if (moveX > WALKSPEED)
				moveX = WALKSPEED;
		}
		else if (leftKey)
		{
			moveX -= ACCELERATION * timeStep;
			if (moveX < -WALKSPEED)
				moveX = -WALKSPEED;
		}
		else
		{
			moveX *= FRICTION;
			if (fabs(moveX) < 0.1f) moveX = 0.0f;
		}

		if (forwardKey)
		{
			moveZ += ACCELERATION * timeStep;
			if (moveZ > WALKSPEED)
				moveZ = WALKSPEED;
		}
		else if (backwardKey)
		{
			moveZ -= ACCELERATION * timeStep;
			if (moveZ < -WALKSPEED)
				moveZ = -WALKSPEED;
		}
		else
		{
			moveZ *= FRICTION;
			if (fabs(moveZ) < 0.1f) moveZ = 0.0f;
		}

		if (jumpKey)
		{
			actor->Jump();
		}

		actor->SetMovement(pivot->GetWorldRotation() * Vector3(moveX, 0.0f, moveZ));

		//Decide what angle the model will be facing
		if (rightKey || leftKey || forwardKey || backwardKey)
		{
			Quaternion dir = Quaternion();
			dir.FromLookRotation(actor->rawMovement.Normalized().CrossProduct(Vector3::UP));
			node_->SetRotation(dir);
			newRotation = dir;
		}

		if (input->GetMouseButtonDown(MOUSEB_LEFT))
		{
			ChangeState(STATE_REVIVE);
		}
		else if (input->GetMouseButtonDown(MOUSEB_RIGHT) && actor->onGround && stateTimer > 0.5f)
		{
			ChangeState(STATE_SLIDE);
		}
		else if (input->GetMouseButtonDown(MOUSEB_RIGHT) && !actor->onGround && stateTimer - lastShield > 0.5f)
		{
			lastShield = stateTimer;
			Zeus::MakeShield(scene, node_->GetWorldPosition() + Vector3::UP*1.5f, 5.0f);
		}

		if (!groundDetector->IsActive())
			groundDetector->Activate();
		groundDetector->SetPosition(body->GetPosition());
		physworld->GetRigidBodies(grounds, groundDetector);
		
		for (PODVector<RigidBody*>::Iterator i = grounds.Begin(); i != grounds.End(); ++i)
		{
			RigidBody* b = (RigidBody*)*i;
			if (b)
			{
				if (b->GetCollisionLayer() & 2)
				{
					diddly = true;
					break;
				}
			}
		}

		//Select Animation
		if (!diddly)
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

		actor->Move(timeStep);
		break;
	case STATE_REVIVE: ////////////////////////////////////////////////////////////////////////////////////
		animController->PlayExclusive("Models/grungle_revive.ani", 0, true, 0.2f);
		stateTimer += 1;
		
		moveX *= FRICTION;
		if (fabs(moveX) < 0.1f) moveX = 0.0f;
		moveZ *= FRICTION;
		if (fabs(moveZ) < 0.1f) moveZ = 0.0f;

		actor->SetMovement(pivot->GetWorldRotation() * Vector3(moveX, 0.0f, moveZ));
		actor->Move(timeStep);

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
				Zeus::MakeLightBeam(scene, nearestEnemy->GetNode()->GetWorldPosition());
				nearestEnemy->Revive();
			}
		}
		break;
	case STATE_SLIDE: /////////////////////////////////////////////////////////////////////////////////
		animController->PlayExclusive("Models/grungle_slide.ani", 0, false, 0.2f);

		actor->SetMovement(node_->GetWorldRotation() * Vector3::FORWARD * SLIDESPEED);
		actor->Move(timeStep);

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
	pivot->SetWorldPosition(Vector3(node_->GetWorldPosition().x_, 0.0f, node_->GetWorldPosition().z_));
	modelNode->SetPosition(node_->GetWorldPosition());
	modelNode->SetRotation(modelNode->GetRotation().Slerp(newRotation, 0.25f));
	HandleCamera();
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
			if (position.y_ > node_->GetWorldPosition().y_ + 0.5f && state == STATE_SLIDE)
			{
				actor->KnockBack(25.0f, Quaternion(node_->GetWorldRotation().YawAngle() + 180.0f, Vector3::UP));
				ChangeState(STATE_DEFAULT);
			}
		}
	}
}

void Player::PostUpdate(StringHash eventType, VariantMap& eventData)
{
	
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
		actor->KnockBack(20.0f, source->GetWorldRotation());
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
	Vector3 worldPos = body->GetPosition();
	Quaternion newAngle = Quaternion();
	newAngle.FromLookRotation((worldPos - cameraNode->GetWorldPosition()).Normalized());
	cameraNode->SetWorldRotation(newAngle);
	//cameraNode->Rotate(Quaternion(input->GetMouseMoveY() * 0.25f, Vector3::RIGHT), TS_LOCAL);
	
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
	if (newState == STATE_DEFAULT && state != STATE_DEFAULT)
	{
		lastShield = -1000.0f;
	}
	if (newState == STATE_SLIDE && state != STATE_SLIDE)
	{
		actor->maxspeed = SLIDESPEED;
		node_->SetRotation(Quaternion(0.0f, newRotation.EulerAngles().y_ + 90.0f, 0.0f));
		shape->SetSize(Vector3(1.0f, 0.33f, 1.0f));
		shape->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
	}
	else if (newState != STATE_SLIDE && state == STATE_SLIDE)
	{
		actor->maxspeed = WALKSPEED;
		shape->SetSize(orgShapeSize);
		shape->SetPosition(orgShapePos);
	}
	stateTimer = 0;
	state = newState;
}

Player::~Player()
{
}
