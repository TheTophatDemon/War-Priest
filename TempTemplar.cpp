#include "TempTemplar.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/DrawableEvents.h>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "Zeus.h"
#include "WeakChild.h"
#include "GunPriest.h"
#include "Settings.h"
#include "TempShield.h"
#include <iostream>

float TempTemplar::MELEE_RANGE = 3.0f;
float TempTemplar::MELEE_DAMAGE = 10.0f;

#define IDLE_ANIM "Models/enemy/temptemplar_idle.ani"
#define REVIVE_ANIM "Models/enemy/temptemplar_revive.ani"
#define WALK_ANIM "Models/enemy/temptemplar_walk.ani"
#define SWING_ANIM "Models/enemy/temptemplar_swing.ani"

#define SOUND_SWORD "Sounds/enm_sword.wav"

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

	shieldNode = TempShield::MakeTempShield(scene, node_->GetWorldPosition(), node_);
	shieldNode->SetDeepEnabled(false);
	shieldComponent = shieldNode->GetComponent<TempShield>();

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(TempTemplar, OnSettingsChange));

	SendEvent(Settings::E_SETTINGSCHANGED); //To initialize

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);
	animController->SetSpeed(WALK_ANIM, 6.0f);
}

void TempTemplar::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	actor->maxSpeed = Settings::ScaleWithDifficulty(2.5f, 5.0f, 10.0f);
}

void TempTemplar::RegisterObject(Context* context)
{
	context->RegisterFactory<TempTemplar>();
}

void TempTemplar::Execute()
{
	float targetDist = 10000.0f;
	Vector3 targetDiff = Vector3::ZERO;
	if (target)
	{
		targetDiff = (target->GetWorldPosition() - node_->GetWorldPosition());
		targetDist = targetDiff.Length();
	}
	
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
	{
		if (shieldComponent->formed)
		{
			if (targetDist < MELEE_RANGE)
			{
				ChangeState(STATE_ATTACK);
			}
		}
		const Vector3 shieldDiff = node_->GetWorldPosition() - shieldNode->GetWorldPosition();
		if (shieldDiff.Length() > shieldComponent->maxSize * 0.4f) //Stay inside the shield
		{
			walking = true;
			turnTimer = 3.0f;
			newRotation.FromLookRotation(Vector3(-shieldDiff.x_, 0.0f, -shieldDiff.z_), Vector3::UP);
			actor->SetInputFPS(true, false, false, false);
			actor->Move(deltaTime);
		}
		else if (targetDist < MELEE_RANGE * 3.0f && Settings::GetDifficulty() > Settings::UNHOLY_THRESHOLD) //Chase after the player in Unholy Mode
		{
			walking = true;
			newRotation.FromLookRotation(Vector3(targetDiff.x_, 0.0, targetDiff.z_) / targetDist, Vector3::UP);
			actor->SetInputFPS(true, false, false, false);
			actor->Move(deltaTime);
		}
		else
		{
			Wander(true);
			actor->SetInputFPS(walking, false, false, false);
			actor->Move(deltaTime);
		}

		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		break;
	}
	case STATE_ATTACK:
		stateTimer += deltaTime;

		FaceTarget();
		if (stateTimer > 0.5f && !attacked && target && targetDist < MELEE_RANGE)
		{
			attacked = true;
			VariantMap map = VariantMap();
			map.Insert(Pair<StringHash, Variant>(Projectile::P_PERPETRATOR, node_));
			map.Insert(Pair<StringHash, Variant>(Projectile::P_VICTIM, Variant(target)));
			map.Insert(Pair<StringHash, Variant>(Projectile::P_DAMAGE, MELEE_DAMAGE));
			SendEvent(Projectile::E_PROJECTILEHIT, map);
		}
		if (animController->IsAtEnd(SWING_ANIM))
		{
			ChangeState(STATE_WANDER);
		}

		actor->SetInputVec(Vector3::ZERO);
		actor->Move(deltaTime);
		break;
	}
}

void TempTemplar::OnAnimTrigger(StringHash eventType, VariantMap& eventData)
{
	const String name = eventData[AnimationTrigger::P_NAME].GetString();
	const Animation* anim = (Animation*)eventData[AnimationTrigger::P_ANIMATION].GetPtr();
	if (anim->GetName() == SWING_ANIM)
	{
		soundSource->Play(SOUND_SWORD, true);
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
		if (!shieldNode->IsEnabled()) 
		{
			shieldNode->SetWorldPosition(node_->GetWorldPosition());
			shieldNode->SetDeepEnabled(true);
		}
	}
	else if (newState == STATE_ATTACK)
	{
		attacked = false;
		animController->PlayExclusive(SWING_ANIM, 128, false, 0.2f);
		animController->SetTime(SWING_ANIM, 0.0f);
		SubscribeToEvent(modelNode, E_ANIMATIONTRIGGER, URHO3D_HANDLER(TempTemplar, OnAnimTrigger));
	}
	else if (newState == STATE_IDLE)
	{
		animController->StopAll();
		animController->PlayExclusive(IDLE_ANIM, 0, false, 0.2f);
	}
}

void TempTemplar::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_ATTACK)
	{
		animController->StopLayer(128, 0.2f);
		UnsubscribeFromEvent(modelNode, E_ANIMATIONTRIGGER);
	}
}

void TempTemplar::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

TempTemplar::~TempTemplar()
{
}
