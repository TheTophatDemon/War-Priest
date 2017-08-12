#include "TempTemplar.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/Animation.h>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "Zeus.h"
#include "WeakChild.h"
#include "GunPriest.h"
#include "Settings.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_ATTACK 32

#define SHIELD_SIZE 18.0f
#define MELEE_RANGE 3.0f
#define MELEE_DAMAGE 10.0f

#define IDLE_ANIM "Models/enemy/temptemplar_idle.ani"
#define REVIVE_ANIM "Models/enemy/temptemplar_revive.ani"
#define WALK_ANIM "Models/enemy/temptemplar_walk.ani"
#define SWING_ANIM "Models/enemy/temptemplar_swing.ani"

TempTemplar::TempTemplar(Context* context) : Enemy(context)
{
	deltaTime = 0.0f;
}

void TempTemplar::DelayedStart()
{
	cache->GetResource<Animation>(IDLE_ANIM);
	cache->GetResource<Animation>(WALK_ANIM);
	cache->GetResource<Animation>(REVIVE_ANIM);
	cache->GetResource<Animation>(SWING_ANIM);

	Enemy::DelayedStart();
	actor->maxspeed = 5.0f;

	//Make shield
	shield = scene->CreateChild("shield");
	shield->AddTag("tempshield");
	CollisionShape* cs = shield->CreateComponent<CollisionShape>();
	cs->SetSphere(1.0f);
	RigidBody* rb = shield->CreateComponent<RigidBody>();
	rb->SetCollisionLayer(17);//1+16
	rb->SetTrigger(true);
	rb->SetLinearFactor(Vector3::ZERO);
	rb->SetAngularFactor(Vector3::ZERO);
	
	shieldModel = shield->CreateComponent<StaticModel>();
	shieldModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	shieldModel->SetMaterial(cache->GetResource<Material>("Materials/shield.xml"));
	shieldModel->SetEnabled(false);
	shield->SetScale(0.5f);
	WeakChild::MakeWeakChild(shield, node_);
	subShield = shield->CreateChild("subshield");
	StaticModel* subModel = (StaticModel*)subShield->CloneComponent(shieldModel);
	subShield->SetScale(0.95f);

	//Spin those shields
	SharedPtr<ValueAnimation> spinAnim(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion::IDENTITY);
	shield->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);
	subShield->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 2.0f);

	SubscribeToEvent(shield, E_NODECOLLISION, URHO3D_HANDLER(TempTemplar, OnShieldCollision));

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);
	animController->SetSpeed(WALK_ANIM, 6.0f);
}

void TempTemplar::RegisterObject(Context* context)
{
	context->RegisterFactory<TempTemplar>();
}

void TempTemplar::Execute()
{
	float targetDist = 10000.0f;
	if (target)
	{
		targetDist = (target->GetWorldPosition() - node_->GetWorldPosition()).Length();
	}
	shield->SetWorldPosition(node_->GetWorldPosition());
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		if (shield->GetScale().x_ < SHIELD_SIZE)
		{
			shield->SetScale(shield->GetScale() + Vector3(deltaTime * 10.0f, deltaTime * 10.0f, deltaTime * 10.0f));
		}
		else
		{
			shield->SetScale(SHIELD_SIZE);
			if (targetDist < MELEE_RANGE)
			{
				ChangeState(STATE_ATTACK);
			}
		}
		Wander(false, true);
		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		break;
	case STATE_ATTACK:
		animController->PlayExclusive(SWING_ANIM, 0, true, 0.2f);
		stateTimer += deltaTime;

		FaceTarget();
		if (stateTimer > 0.5f && !attacked && target && targetDist < MELEE_RANGE)
		{
			attacked = true;
			VariantMap map = VariantMap();
			map.Insert(Pair<StringHash, Variant>(StringHash("perpetrator"), node_));
			map.Insert(Pair<StringHash, Variant>(StringHash("victim"), Variant(target)));
			map.Insert(Pair<StringHash, Variant>(StringHash("damage"), MELEE_DAMAGE));
			SendEvent(StringHash("ProjectileHit"), map);
		}
		if (stateTimer > animController->GetLength(SWING_ANIM) * 0.9f)
		{
			ChangeState(STATE_WANDER);
		}

		actor->SetMovement(0.0f, 0.0f);
		actor->Move(deltaTime);
		break;
	}
}

void TempTemplar::Dead()
{
	Enemy::Dead();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.0f);
	if (animController->GetTime(REVIVE_ANIM) >= animController->GetLength(REVIVE_ANIM) * 0.9f)
	{
		ChangeState(STATE_WANDER);
	}
}

void TempTemplar::EnterState(const int newState)
{
	if (newState == STATE_DEAD)
	{
		shape->SetSize(Vector3(oldShape->GetSize().y_, oldShape->GetSize().x_ * 0.1f, oldShape->GetSize().y_));
		shape->SetPosition(Vector3(0.0f, oldShape->GetSize().x_, 0.0f));
		body->SetMass(0.0f);
	}
	else if (newState == STATE_WANDER)
	{
		shieldModel->SetEnabled(true);
		subShield->GetComponent<StaticModel>()->SetEnabled(true);
	}
	else if (newState == STATE_ATTACK)
	{
		attacked = false;
		animController->StopAll();
	}
}

void TempTemplar::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
}

void TempTemplar::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

void TempTemplar::OnShieldCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();

	if (other != node_) 
	{
		if (other->HasComponent<Actor>())
		{
			const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			Quaternion direction = Quaternion();
			direction.FromLookRotation(diff.Normalized(), Vector3::UP);
			other->GetComponent<Actor>()->KnockBack(10.0f + (12.0f / diff.LengthSquared()), direction);
		}
		else if (otherBody->GetMass() > 0)
		{
			const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			const float push = 3.0f + (12.0f / diff.LengthSquared());
			otherBody->ApplyImpulse(diff.Normalized() * Vector3(push, push, push) * otherBody->GetMass());
		}
	}
}

TempTemplar::~TempTemplar()
{
}
