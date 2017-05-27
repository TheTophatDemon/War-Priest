#include "PostalPope.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "Debris.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SUMMON 32
#define STATE_THROW 33

PostalPope::PostalPope(Context* context) : Enemy(context)
{
	deltaTime = 0.0f;
	debris = PODVector<RigidBody*>();
}

void PostalPope::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 6.0f;
	spinner = node_->CreateChild();
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
		break;
	case STATE_SUMMON:
		stateTimer += deltaTime;
		for (PODVector<RigidBody*>::Iterator i = debris.Begin(); i != debris.End(); ++i)
		{
			RigidBody* rb = (RigidBody*)*i;
			if (rb)
			{
				Vector3 diff = (upperBody - rb->GetNode()->GetWorldPosition());
				if (diff.LengthSquared() > 16.0f)
				{
					rb->ApplyImpulse(diff.Normalized() * rb->GetMass() * deltaTime * 50.0f);
				}
				else
				{
					rb->SetLinearVelocity(Vector3::ZERO);
					rb->SetLinearFactor(Vector3(1.0f, 0.1f, 1.0f));
				}
			}
		}
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
		stateTimer += deltaTime;
		if (stateTimer > 0.5f)
		{
			stateTimer = 0.0f;
			for (PODVector<RigidBody*>::Iterator i = debris.Begin(); i != debris.End(); ++i)
			{
				RigidBody* rb = (RigidBody*)*i;
				if (rb)
				{
					Node* n = rb->GetNode();
					Vector3 wp = n->GetWorldPosition();
					Vector3 tDiff = (target->GetWorldPosition() - wp).Normalized();
					Vector3 pDiff = (node_->GetWorldPosition() - wp).Normalized();
					if (pDiff.DotProduct(node_->GetWorldDirection()) >= 0.0f)
					{
						n->SetParent(scene);
						rb->SetRestitution(1.0f);
						rb->SetLinearFactor(Vector3::ONE);
						rb->ApplyImpulse(tDiff * rb->GetMass() * 50.0f);
						Debris* debs = new Debris(context_);
						debs->damage = 15;
						n->AddComponent(debs, 666, LOCAL);
						debris.Remove(rb);
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
}

void PostalPope::EnterState(int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_SUMMON)
	{
		debris.Clear();
		PODVector<RigidBody*> surroundings;
		physworld->GetRigidBodies(surroundings, Sphere(node_->GetWorldPosition(), 8.0f), 16);
		for (PODVector<RigidBody*>::Iterator i = surroundings.Begin(); i != surroundings.End(); ++i)
		{
			RigidBody* rb = (RigidBody*)*i;
			if (rb)
			{
				Node* n = rb->GetNode();
				if (n->HasTag("debris"))
				{
					debris.Push(rb);
					n->SetParent(spinner);
				}
			}
		}
	}
}

void PostalPope::LeaveState(int oldState)
{
	Enemy::LeaveState(oldState);
}

void PostalPope::Revive()
{
	Enemy::Revive();
	ChangeState(STATE_WANDER);
}

PostalPope::~PostalPope()
{
}
