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
#define STATE_WIN 4

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
	reviveCount = 0;
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
	game = GetScene()->GetComponent<Gameplay>();
	body = node_->GetComponent<RigidBody>();
	shape = node_->GetComponent<CollisionShape>(); //The main shape is always first
	cache = GetSubsystem<ResourceCache>(); 
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	input = GetSubsystem<Input>();
	cameraNode = game->cameraNode;
	camera = game->camera;
	pivot = scene->CreateChild();
	cameraNode->SetParent(pivot);
	
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

	if (modelNode->HasComponent<AnimationController>())
		animController = modelNode->GetComponent<AnimationController>();
	else
		animController = modelNode->CreateComponent<AnimationController>();

	//Drop Shadow
	dropShadow = scene->CreateChild();
	StaticModel* shadModel = dropShadow->CreateComponent<StaticModel>();
	shadModel->SetModel(cache->GetResource<Model>("Models/shadow.mdl"));
	shadModel->SetMaterial(cache->GetResource<Material>("Materials/shadow.xml"));

	bloodEmitter = node_->GetChild("blood")->GetComponent<ParticleEmitter>();
	bloodEmitter->SetEmitting(false);
	
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Player, OnCollision));
}

void Player::FixedUpdate(float timeStep)
{	
	float newAngle = 0.0f;

	forwardKey = input->GetKeyDown(scene->GetGlobalVar("FORWARD KEY").GetInt());
	backwardKey = input->GetKeyDown(scene->GetGlobalVar("BACKWARD KEY").GetInt());
	rightKey = input->GetKeyDown(scene->GetGlobalVar("RIGHT KEY").GetInt());
	leftKey = input->GetKeyDown(scene->GetGlobalVar("LEFT KEY").GetInt());
	jumpKey = input->GetKeyDown(scene->GetGlobalVar("JUMP KEY").GetInt());

	if (game->winState == 1)
	{
		ChangeState(STATE_WIN);
	}
	if (hurtTimer > 0)
	{
		hurtTimer -= 1;
		if (hurtTimer <= 0)
		{
			bloodEmitter->SetEmitting(false);
		}
	}
	if (input->GetKeyDown(KEY_K) || node_->GetWorldPosition().y_ < -40.0f)
	{
		health = 0.0f;
	}
	if (health <= 0.0f)
	{
		ChangeState(STATE_DEAD);
	}

	HandleShadow();

	switch (state) 
	{
	case STATE_DEFAULT:
		ST_Default(timeStep);
		break;
	case STATE_REVIVE:
		ST_Revive(timeStep);
		break;
	case STATE_SLIDE:
		ST_Slide(timeStep);
		break;
	case STATE_DEAD:
		break;
	case STATE_WIN:
		ST_Win(timeStep);
		break;
	}

	if (state != STATE_WIN && state != STATE_DEAD) 
	{
		modelNode->SetPosition(node_->GetWorldPosition());
		modelNode->SetRotation(modelNode->GetRotation().Slerp(newRotation, 0.25f));
		HandleCamera();
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
			if (position.y_ > node_->GetWorldPosition().y_ + 0.5f && state == STATE_SLIDE)
			{
				actor->KnockBack(25.0f, Quaternion(node_->GetWorldRotation().YawAngle() + 180.0f, Vector3::UP));
				ChangeState(STATE_DEFAULT);
			}
		}
	}
}

void Player::OnHurt(Node* source, int amount)
{
	if (state != STATE_WIN) 
	{
		health -= amount;
		bloodEmitter->SetEmitting(true);
		hurtTimer = 20;
		if (source)
		{
			actor->KnockBack(20.0f, source->GetWorldRotation());
		}
	}
}

void Player::HandleCamera()
{
	pivot->SetWorldPosition(Vector3(node_->GetWorldPosition().x_, 0.0f, node_->GetWorldPosition().z_));
	pivot->Rotate(Quaternion(input->GetMouseMoveX() * scene->GetGlobalVar("MOUSE SENSITIVITY").GetFloat(), Vector3::UP));
	Vector3 worldPos = body->GetPosition();
	Quaternion newAngle = Quaternion();
	newAngle.FromLookRotation((worldPos - cameraNode->GetWorldPosition()).Normalized());
	cameraNode->SetWorldRotation(newAngle);
}

void Player::HandleShadow()
{
	PhysicsRaycastResult shadowRaycast;
	Vector3 doot = Vector3(0.0f, 0.1f, 0.0f);
	physworld->RaycastSingle(shadowRaycast, Ray(node_->GetWorldPosition() + doot, Vector3::DOWN), 500.0f, 2);
	if (shadowRaycast.body_)
	{
		dropShadow->SetEnabled(true);
		dropShadow->SetWorldPosition(shadowRaycast.position_ + Vector3(0.0f, 0.1f, 0.0f));
		Quaternion q = Quaternion();
		q.FromLookRotation(shadowRaycast.normal_.CrossProduct(Vector3::FORWARD));
		dropShadow->SetRotation(q);
	}
	else
	{
		dropShadow->SetEnabled(false);
	}
	if (!actor->onGround && shadowRaycast.body_)
		dropShadow->SetEnabled(true);
	else
		dropShadow->SetEnabled(false);
}

void Player::ChangeState(int newState)
{
	if (state != newState)
	{
		EnterState(newState);
		LeaveState(state);
	}
	stateTimer = 0;
	state = newState;
}

void Player::EnterState(int newState)
{
	PODVector<Node*> children;
	Node* gibs;
	switch (newState) 
	{
		case STATE_DEFAULT:
			lastShield = -1000.0f;
			break;
		case STATE_SLIDE:
			actor->maxspeed = SLIDESPEED;
			node_->SetRotation(Quaternion(0.0f, newRotation.EulerAngles().y_ + 90.0f, 0.0f));
			shape->SetSize(Vector3(1.0f, 0.2f, 1.0f));
			shape->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
			break;
		case STATE_DEAD:
			bloodEmitter->SetEmitting(true);
			//Make gibs
			gibs = scene->CreateChild();
			gibs->LoadXML(cache->GetResource<XMLFile>("Objects/playergibs.xml")->GetRoot());
			gibs->SetWorldPosition(node_->GetWorldPosition());
			gibs->SetWorldRotation(node_->GetWorldRotation());
			gibs->SetScale(modelNode->GetScale());
			//PODVector<Node*> children;
			gibs->GetChildren(children, true);
			for (PODVector<Node*>::Iterator i = children.Begin(); i != children.End(); ++i)
			{
				Node* n = (Node*)*i;
				if (n)
				{
					n->GetComponent<RigidBody>()->ApplyImpulse(Vector3(Random(-250.0f, 250.0f), Random(-10.0f, 500.0f), Random(-250.0f, 250.0f)));
					n->CloneComponent(bloodEmitter, 0U);
				}
			}
			//Remove everything
			HandleCamera();
			node_->Remove();
			modelNode->Remove();
			dropShadow->Remove();
			groundDetector->Remove();

			game->FlashScreen(Color::RED, 0.02f);
			game->Lose();
			break;
		case STATE_WIN:
			modelNode->SetParent(node_);
			dropShadow->Remove();
			groundDetector->Remove();
			break;
	}
}

void Player::LeaveState(int oldState)
{
	if (oldState != STATE_SLIDE)
	{
		actor->maxspeed = WALKSPEED;
		shape->SetSize(orgShapeSize);
		shape->SetPosition(orgShapePos);
	}
}

void Player::ST_Default(float timeStep)
{
	PODVector<RigidBody*> grounds;
	bool diddly = false;

	stateTimer += timeStep;

	if (rightKey)
	{
		moveX += ACCELERATION * timeStep;
	}
	else if (leftKey)
	{
		moveX -= ACCELERATION * timeStep;
	}
	else
	{
		moveX *= FRICTION;
		if (fabs(moveX) < 0.1f) moveX = 0.0f;
	}
	moveX = Clamp(moveX, -WALKSPEED, WALKSPEED);

	if (forwardKey)
	{
		moveZ += ACCELERATION * timeStep;
	}
	else if (backwardKey)
	{
		moveZ -= ACCELERATION * timeStep;
	}
	else
	{
		moveZ *= FRICTION;
		if (fabs(moveZ) < 0.1f) moveZ = 0.0f;
	}
	moveZ = Clamp(moveZ, -WALKSPEED, WALKSPEED);

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

	//!!!!
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

	actor->Move(timeStep);
}

void Player::ST_Revive(float timeStep)
{
	stateTimer += 1;

	moveX *= FRICTION;
	if (fabs(moveX) < 0.1f) moveX = 0.0f;
	moveZ *= FRICTION;
	if (fabs(moveZ) < 0.1f) moveZ = 0.0f;

	actor->SetMovement(pivot->GetWorldRotation() * Vector3(moveX, 0.0f, moveZ));

	animController->PlayExclusive("Models/grungle_revive.ani", 0, true, 0.2f);
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
				if (e->state == 0) //'Tis dead
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
			reviveCount += 1;
		}
	}
	actor->Move(timeStep);
}

void Player::ST_Slide(float timeStep)
{
	stateTimer += timeStep;
	animController->PlayExclusive("Models/grungle_slide.ani", 0, false, 0.2f);

	actor->SetMovement(node_->GetWorldRotation() * Vector3::FORWARD * SLIDESPEED);
	actor->Move(timeStep);

	if (stateTimer > 0.5f)
	{
		stateTimer = 0;
		ChangeState(STATE_DEFAULT);
	}
}

void Player::ST_Win(float timeStep)
{
	actor->SetMovement(0.0f, 0.0f);
	actor->Move(timeStep);
	animController->PlayExclusive("Models/grungle_idle.ani", 0, true, 0.2f);
}

Player::~Player()
{
}
