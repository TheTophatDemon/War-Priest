#include "Enemy.h"
#include "Urho3D/Physics/CollisionShape.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <iostream>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "WeakChild.h"
#include "GunPriest.h"
#include "Settings.h"
#include "Zeus.h"

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_DROWN 2


Enemy::Enemy(Context* context) : LogicComponent(context), 
	distanceFromPlayer(10000.0f), 
	deltaTime(0.0f), 
	revived(false),
	active(false),
	walking(false),
	turnTimer(0.0f),
	stateTimer(0.0f),
	state(0)
{}

void Enemy::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	body = node_->GetComponent<RigidBody>();
	modelNode = node_->GetChild("model");
	shape = node_->GetComponent<CollisionShape>();
	oldShape = new CollisionShape(context_);
	oldShape->SetSize(shape->GetSize());
	oldShape->SetPosition(shape->GetPosition());

	if (!node_->HasComponent<Actor>())
		actor = node_->CreateComponent<Actor>();
	else
		actor = node_->GetComponent<Actor>();
	actor->SetEnabled(false);

	if (modelNode->HasComponent<AnimationController>())
		animController = modelNode->GetComponent<AnimationController>();
	else
		animController = modelNode->CreateComponent<AnimationController>();
	soundSource = node_->CreateComponent<SoundSounder>();

	SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Enemy, OnCollision));

	state = -1;
	ChangeState(STATE_DEAD);
}

void Enemy::DelayedStart()
{
	modelNode->SetRotation(Quaternion(-90.0f, Vector3::UP));
	target = game->playerNode;

	node_->Rotate(Quaternion(Random(0.0f, 360.0f), Vector3::UP), TS_LOCAL);
	newRotation = node_->GetWorldRotation();
}

void Enemy::FixedUpdate(float timeStep)
{
	deltaTime = timeStep;
	Vector3 plyPos = game->playerNode->GetWorldPosition(); plyPos.y_ = 0.0f;
	Vector3 ourPos = node_->GetWorldPosition(); ourPos.y_ = 0.0f;
	distanceFromPlayer = (ourPos - plyPos).Length();
	//The max distance from the player for this enemy to be updated is dependant on the graphics settings AND whether or not the camera faces them.
	float visdist = 100.0f; 
	const Vector3 camDiff = node_->GetWorldPosition() - game->cameraNode->GetWorldPosition();
	const float camDot = camDiff.DotProduct(game->cameraNode->GetWorldDirection());
	if (camDot < 0.0f) visdist *= .5f;
	if (Settings::AreGraphicsFast()) visdist *= .8f;

	if (distanceFromPlayer < visdist)
	{
		active = true;
		Execute();
		if (state != STATE_DEAD) node_->SetWorldRotation(node_->GetWorldRotation().Slerp(newRotation, 0.25f));
	}
	else
	{
		active = false;
		body->SetLinearVelocity(Vector3::ZERO);
	}
	if (state == STATE_DROWN)
	{
		stateTimer += timeStep;
		actor->SetInputFPS(false, false, false, false);
		actor->Move(timeStep);
		if (stateTimer > 0.75f)
		{
			Zeus::MakeLightBeam(scene, node_->GetWorldPosition(), 2048.0f);
			node_->Remove();
		}
	}
	else if (state == STATE_DEAD) 
	{
		KeepOnGround();
	}
}

void Enemy::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 256)
	{
		ChangeState(STATE_DROWN);
	}
}

void Enemy::EndFrameCheck(StringHash eventType, VariantMap& eventData)
{
	if (distanceFromPlayer < 80.0f)
	{
		body->SetEnabled(true);
	}
	else
	{
		body->SetEnabled(false);
	}
}

//Defines behavior for walking and turning haphazardly. Does not actually move the actor, but it modifies the "walking" variable and "newRotation".
void Enemy::Wander(bool pause, float slopeIntolerance, float wallMargin)
{
	turnTimer += deltaTime;
	if (turnTimer > 0.6f)
	{
		turnTimer = 0.0f;
		TurnRandomly();
		if (pause)
		{
			walking = Random(1.0f) > 0.5f;
		}
		else
		{
			walking = true;
		}
	}
	if (CheckLift())
	{
		walking = true;
	}
	if (walking)
	{
		if (CheckCliff(slopeIntolerance))
		{
			walking = false;
			turnTimer = 0.5f;
		}
		PhysicsRaycastResult result;
		if (CheckWall(result, wallMargin))
		{
			ReflectOffNormal(result.normal_);
		}
	}
}

//Turn by an angle within a certain range in either direction
void Enemy::TurnRandomly(const float min, const float max)
{
	float turnAmt = Random(30.0f, 180.0f);
	if (Random(1.0f) > 0.5f) turnAmt = -turnAmt;
	newRotation = Quaternion(node_->GetWorldRotation().y_ + turnAmt, Vector3::UP);
}

//Changes direction based off a normal vector. Used for a sort of "bouncing" movement.
void Enemy::ReflectOffNormal(const Vector3 normal)
{
	const Vector3 perpAxis = normal.CrossProduct(Vector3::UP);
	const float paraComponent = (-node_->GetWorldDirection()).DotProduct(normal);
	const float perpComponent = (-node_->GetWorldDirection()).DotProduct(perpAxis);
	newRotation.FromLookRotation(perpAxis * -perpComponent + normal * paraComponent);
}

//Returns true if enemy is about to run into a wall
bool Enemy::CheckWall(PhysicsRaycastResult& result, const float wallMargin)
{
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.1f, 0.0f), node_->GetWorldDirection()), wallMargin, 2U);
	if (result.body_)
	{
		if (fabs(result.normal_.y_) < 0.1f)
		{
			return true;
		}
	}
	return false;
}

//Returns true if there's a moving platform above you
bool Enemy::CheckLift()
{
	PhysicsRaycastResult result;
	physworld->SphereCast(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 2.7f, 0.0f), Vector3::UP), 1.5f, 250.0f, 2U);
	if (result.body_)
	{
		return true;
	}
	return false;
}

//Returns true if the enemy is about to walk somewhere undesirable
bool Enemy::CheckCliff(const float slopeIntolerance)
{
	//Two rays are cast downward from in front of the actor, one to the left and on to the right. 
	//Imagine it as an isosceles triangle with the tip being the enemy's position
	PhysicsRaycastResult result, result2;
	const Vector3 base = node_->GetWorldPosition() + Vector3(0.0f, 1.2f, 0.0f) + node_->GetWorldDirection() * 1.5f;
	physworld->RaycastSingle(result, Ray(base + (node_->GetWorldRotation() * Vector3::RIGHT), Vector3::DOWN), 3.0f, 258); //2+256
	physworld->RaycastSingle(result2, Ray(base + (node_->GetWorldRotation() * Vector3::LEFT), Vector3::DOWN), 3.0f, 258); //2+256
	if (actor->fall <= 0.1f)
	{
		const float ny1 = fabs(result.normal_.y_);
		const float ny2 = fabs(result2.normal_.y_);
		if (!result.body_
			|| ny1 < slopeIntolerance
			|| !result2.body_
			|| ny2 < slopeIntolerance)
		{
			return true;
		}
		//Avoid liquid, too!
		if (result.body_)
		{
			if (result.body_->GetCollisionLayer() & 256) return true;
		}
		if (result2.body_)
		{
			if (result2.body_->GetCollisionLayer() & 256) return true;
		}
	}
	return false;
}

void Enemy::Dead() //This function defines the defualt behavior for being dead
{
	actor->SetEnabled(false);
	body->SetLinearVelocity(Vector3::ZERO);
	body->SetAngularVelocity(Vector3::ZERO);
}

void Enemy::Revive()
{
	revived = true;
	node_->Translate(Vector3(0.0f, 0.1f, 0.0f), TS_LOCAL);
}

void Enemy::ChangeState(const int newState)
{
	if (newState != state)
	{
		EnterState(newState);
		LeaveState(state);
		stateTimer = 0.0f;
	}
	state = newState;
}

void Enemy::EnterState(const int newState)
{
	if (newState == STATE_DEAD)
	{
		shape->SetSize(Vector3(oldShape->GetSize().y_, oldShape->GetSize().x_ * 0.5f, oldShape->GetSize().y_));
		shape->SetPosition(Vector3(0.0f, -oldShape->GetSize().x_ * 0.5f, 0.0f));
		body->SetMass(0.0f);
	}
	else if (newState == STATE_DROWN)
	{
		soundSource->Play("Sounds/env_splash.wav");
		ParticleEmitter* splashEmit = node_->CreateComponent<ParticleEmitter>();
		splashEmit->SetEffect(cache->GetResource<ParticleEffect>("Particles/splash.xml"));
	}
}

void Enemy::LeaveState(const int oldState)
{
	if (oldState == STATE_DEAD)
	{
		actor->SetEnabled(true);
		shape->SetSize(oldShape->GetSize());
		shape->SetPosition(oldShape->GetPosition());
		body->SetMass(120.0f);
	}
}

void Enemy::FaceTarget()
{
	Quaternion face = Quaternion();
	Vector3 diff = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
	diff.y_ = 0.0f;
	face.FromLookRotation(diff, Vector3::UP);
	newRotation = face;
}

void Enemy::KeepOnGround()
{
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.5f, 0.0f), Vector3::DOWN), 500.0f, 2);
	if (result.body_)
	{
		node_->SetWorldPosition(result.position_);
		if (result.body_->GetNode()->HasTag("lift"))
		{
			//The corpse's mass cannot be anything other than 0. However, we need lifts to recognize their presence so that
			//corpses rotate when they're resting on lifts.
			//Here, we're making a fake NodeCollision event and having the lift's node send it
			//This will trick the lift into registering the corpse as an object that moves along with it
			VariantMap map = VariantMap();
			map.Insert(Pair<StringHash, Variant>(NodeCollision::P_BODY, Variant(result.body_)));
			map.Insert(Pair<StringHash, Variant>(NodeCollision::P_OTHERBODY, Variant(body)));
			map.Insert(Pair<StringHash, Variant>(NodeCollision::P_OTHERNODE, Variant(node_)));
			map.Insert(Pair<StringHash, Variant>(NodeCollision::P_TRIGGER, Variant(false)));
			VectorBuffer contacts = VectorBuffer();
			contacts.WriteVector3(node_->GetWorldPosition()); //Position
			contacts.WriteVector3(Vector3(0.0f, -1.0f, 0.0f)); //Normal (Make it think we're on top)
			contacts.WriteFloat(0.0f); //Distance
			contacts.WriteFloat(0.0f); //Impulse
			map.Insert(Pair<StringHash, Variant>(NodeCollision::P_CONTACTS, Variant(contacts)));
			result.body_->GetNode()->SendEvent(E_NODECOLLISION, map);
		}
	}
	else
	{
		std::cout << "ENEMY IN THE WATER" << std::endl;
	}
}

Enemy::~Enemy()
{
	
}
