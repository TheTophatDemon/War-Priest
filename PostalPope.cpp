#include "PostalPope.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "Debris.h"
#include "Zeus.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SUMMON 32
#define STATE_THROW 33

#define IDLE_ANIM "Models/enemy/postalpope_idle.ani"
#define WALK_ANIM "Models/enemy/postalpope_walk.ani"
#define SUMMON_ANIM "Models/enemy/postalpope_summon.ani"
#define THROW_ANIM "Models/enemy/postalpope_throw.ani"
#define REVIVE_ANIM "Models/enemy/postalpope_revive.ani"

PostalPope::PostalPope(Context* context) : Enemy(context)
{
	deltaTime = 0.0f;
	debris = Vector<Node*>();
}

void PostalPope::DelayedStart()
{
	Enemy::DelayedStart();

	if (modelNode->HasComponent<AnimationController>())
		animController = modelNode->GetComponent<AnimationController>();
	else
		animController = modelNode->CreateComponent<AnimationController>();

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);

	actor->maxspeed = 6.0f;
	spinner = scene->CreateChild();
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
	Vector3 upperBody = node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f);
	spinner->SetWorldPosition(upperBody);
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		Wander();
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
		
		actor->SetMovement(Vector3::ZERO);
		actor->Move(deltaTime);
		if (stateTimer > 1.0f)
		{
			ChangeState(STATE_THROW);
		}
		if (debris.Size() == 0)
		{
			ChangeState(STATE_WANDER);
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
						
						RigidBody* rb = n->GetComponent<RigidBody>();
						rb->SetLinearFactor(Vector3::ONE);
						rb->SetRestitution(1.0f);
						rb->ApplyImpulse(tDiff * rb->GetMass() * 50.0f);
						rb->ApplyImpulse(Vector3::UP * rb->GetMass() * 10.0f);
						rb->SetUseGravity(true);
						
						debris.Remove(n);
						break;
					}
				}
			}
		}
		actor->SetMovement(Vector3::ZERO);
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

void PostalPope::EnterState(int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_SUMMON)
	{
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
		for (int i = 0; i < 5; ++i) //Summon more debris
		{
			float angle = (M_PI / 2.5f) * i;
			Node* n = spinner->CreateChild();
			n->LoadXML(cache->GetResource<XMLFile>("Objects/rock.xml")->GetRoot());
			n->SetPosition(Vector3(cosf(angle) * 4.0f,0.0f,sinf(angle) * 4.0f));
			Debris* debs = new Debris(context_);
			debs->damage = 15;
			n->AddComponent(debs, 666, LOCAL);
			n->GetComponent<RigidBody>()->SetLinearFactor(Vector3::ZERO);
			
			Zeus::PuffOfSmoke(scene, n->GetWorldPosition(), 2.0f);

			debris.Push(n);
		}
	}
	else if (newState == STATE_THROW)
	{
		animController->PlayExclusive(THROW_ANIM, 0, false, 0.2f);
		FaceTarget();
	}
}

void PostalPope::LeaveState(int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_THROW)
	{
		
	}
}

void PostalPope::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

PostalPope::~PostalPope()
{
}
