#include "PostalPope.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "Debris.h"
#include "Zeus.h"
#include "WeakChild.h"
#include "GunPriest.h"
#include "Settings.h"
#include <iostream>

#define IDLE_ANIM "Models/enemy/postalpope_idle.ani"
#define WALK_ANIM "Models/enemy/postalpope_walk.ani"
#define SUMMON_ANIM "Models/enemy/postalpope_summon.ani"
#define THROW_ANIM "Models/enemy/postalpope_throw.ani"
#define REVIVE_ANIM "Models/enemy/postalpope_revive.ani"

PostalPope::PostalPope(Context* context) : Enemy(context), 
	summoned(false)
{
	debris = Vector<Node*>();
}

void PostalPope::DelayedStart()
{
	//Load resources in advance to prevent lag spikes
	cache->GetResource<Animation>(IDLE_ANIM);
	cache->GetResource<Animation>(WALK_ANIM);
	cache->GetResource<Animation>(SUMMON_ANIM);
	cache->GetResource<Animation>(THROW_ANIM);
	cache->GetResource<Animation>(REVIVE_ANIM);
	cache->GetResource<Material>("Materials/telekinesis.xml");
	cache->GetResource<XMLFile>("Objects/debris.xml");
	cache->GetResource<Model>("Models/props/rock.mdl");
	cache->GetResource<Material>("Materials/palpan_cliff.xml");

	Enemy::DelayedStart();

	weeoo = node_->CreateComponent<SoundSource3D>();
	weeoo->SetSoundType("GAMEPLAY");

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);

	actor->maxSpeed = 6.0f;
	spinner = scene->CreateChild();
	WeakChild::MakeWeakChild(spinner, node_);
	
	SharedPtr<ValueAnimation> spinAnim(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion::IDENTITY);
	spinner->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);
	
}

void PostalPope::RegisterObject(Context* context)
{
	context->RegisterFactory<PostalPope>();
}

void PostalPope::Execute()
{
	weeoo->SetGain(Settings::GetSoundVolume());
	const Vector3 upperBody = node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f);
	spinner->SetWorldPosition(upperBody);
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;


	case STATE_WANDER:
		actor->gravity = true;
		
		Wander(false, 0.9f, 8.0f);
		actor->SetInputFPS(walking, false, false, false);
		actor->Move(deltaTime);

		stateTimer += deltaTime;
		if (stateTimer > 2.0f)
		{
			ChangeState(STATE_SUMMON);
		}

		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		break;


	case STATE_SUMMON:
		animController->PlayExclusive(SUMMON_ANIM, 0, false, 0.2f);
		FaceTarget();
		stateTimer += deltaTime;
		actor->gravity = false;
		if (CanSummon()) 
		{
			if (!summoned) SummonDebris();
			actor->SetInputVec(Vector3::ZERO);
			actor->Move(deltaTime);
			if (stateTimer > 1.0f)
			{
				ChangeState(STATE_THROW);
			}
			if (debris.Size() == 0)
			{
				ChangeState(STATE_WANDER);
			}
		}
		else
		{
			actor->SetInputVec(Vector3::ZERO);
			actor->Move(deltaTime);
			node_->Translate(Vector3::UP * deltaTime * 3.0f, TS_LOCAL);
			if (stateTimer > 5.0f)
			{
				ChangeState(STATE_WANDER);
			}
		}
		break;


	case STATE_THROW:
		FaceTarget();
		stateTimer += deltaTime;
		if (stateTimer > 0.5f)
		{
			stateTimer = 0.0f;
			for (Vector<Node*>::Iterator i = debris.Begin(); i != debris.End(); ++i)
			{
				Node* n = dynamic_cast<Node*>(*i);
				if (n)
				{
					Vector3 tDiff = (target->GetWorldPosition() - n->GetWorldPosition()).Normalized();
					Vector3 pDiff = (n->GetWorldPosition() - upperBody);
					pDiff.y_ = 0.0f;
					pDiff.Normalize();
					if (pDiff.DotProduct(node_->GetWorldDirection()) >= 0.5f)
					{
						n->SetParent(scene);
						Debris* debs = n->CreateComponent<Debris>();
						debs->damage = Settings::ScaleWithDifficulty(10.0f, 15.0f, 20.0f);
						debs->owner = node_;
						
						RigidBody* rb = n->GetComponent<RigidBody>();
						rb->SetLinearFactor(Vector3::ONE);
						rb->SetRestitution(1.0f);
						rb->ApplyImpulse(tDiff * rb->GetMass() * 50.0f);
						rb->ApplyImpulse(Vector3::UP * rb->GetMass() * 6.0f);
						rb->SetUseGravity(true);
						
						debris.Remove(n);
						soundSource->Play("Sounds/enm_telethrow.wav");
						break;
					}
				}
			}
		}
		actor->SetInputVec(Vector3::ZERO);
		actor->Move(deltaTime);
		if (debris.Size() <= 0)
		{
			ChangeState(STATE_WANDER);
		}
		break;


	}
}

void PostalPope::Dead()
{
	Enemy::Dead();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.0f);
	if (animController->GetTime(REVIVE_ANIM) >= animController->GetLength(REVIVE_ANIM) * 0.9f)
	{
		ChangeState(STATE_WANDER);
	}
}

void PostalPope::EnterState(const int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_SUMMON)
	{
		summoned = false;
	}
	else if (newState == STATE_THROW)
	{
		animController->PlayExclusive(THROW_ANIM, 0, false, 0.2f);
		FaceTarget();
	}
	else if (newState == STATE_IDLE)
	{
		animController->StopAll();
		animController->PlayExclusive(IDLE_ANIM, 0, false, 0.2f);
	}
}

void PostalPope::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_THROW)
	{
		weeoo->Stop();
	}
}

void PostalPope::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

bool PostalPope::CanSummon()
{
	PODVector<RigidBody*> result;
	const Vector3 sz = Vector3(4.5f, 0.5f, 4.5f);
	const Vector3 h = Vector3(0.0f, 3.0f, 0.0f);
	physworld->GetRigidBodies(result, BoundingBox(node_->GetWorldPosition() - sz + h, node_->GetWorldPosition() + sz + h), 2);
	if (result.Size() > 0) 
	{
		return false;
	}
	return true;
}

void PostalPope::SummonDebris()
{
	summoned = true;
	soundSource->Play("Sounds/enm_summon.wav");
	weeoo->Play(cache->GetResource<Sound>("Sounds/enm_telekinesis.wav"));
	if (debris.Size() > 0)
	{
		for (PODVector<Node*>::Iterator i = debris.Begin(); i != debris.End(); ++i)
		{
			Node* n = dynamic_cast<Node*>(*i);
			if (n)
			{
				n->Remove();
			}
		}
		debris.Clear();
	}
	int count = 5;
	const float difficulty = Settings::GetDifficulty();
	if (difficulty < Settings::UNHOLY_THRESHOLD && difficulty >= 0.75f)
		count = 5;
	if (difficulty >= Settings::UNHOLY_THRESHOLD)
		count = 7;
	else if (difficulty >= 1.25f)
		count = 6;
	if (difficulty < 0.55f)
		count = 3;
	else if (difficulty < 0.75f)
		count = 4;
	

	for (int i = 0; i < count; ++i) //Summon more debris
	{
		const float angle = (M_PI / (count / 2.0f)) * i;
		Node* n = spinner->CreateChild();
		n->LoadXML(cache->GetResource<XMLFile>("Objects/debris.xml")->GetRoot());
		n->SetPosition(Vector3(cosf(angle) * 4.0f, 0.0f, sinf(angle) * 4.0f));
		n->GetComponent<RigidBody>()->SetLinearFactor(Vector3::ZERO);

		Zeus::PuffOfSmoke(scene, n->GetWorldPosition(), 2.0f);

		debris.Push(n);
	}
}

PostalPope::~PostalPope()
{
}
