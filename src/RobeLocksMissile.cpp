#include "RobeLocksMissile.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>

#include "Settings.h"
#include "Zeus.h"
#include "Gameplay.h"

#define LOCK_ON_FREQUENCY 1.0f

RobeLocksMissile::RobeLocksMissile(Context* context) : Projectile(context),
	speed(20.0f),
	lockOnTimer(0.0f)
{
}

void RobeLocksMissile::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);

	Projectile::Start();

	SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(RobeLocksMissile, OnCollision));
}

void RobeLocksMissile::FixedUpdate(float timeStep)
{
	PreUpdate(timeStep);

	if (lockOnTimer > 0.0f)
	{
		lockOnTimer -= timeStep;
		if (lockOnTimer <= 0.0f)
		{
			lockOnTimer = LOCK_ON_FREQUENCY;
			if (target.Get())
			{
				Quaternion newRot;
				newRot.FromLookRotation((target->GetWorldPosition() - node_->GetWorldPosition()).Normalized());
				node_->SetWorldRotation(newRot);
			}
		}
	}

	node_->Translate(Vector3::FORWARD * speed * timeStep, TS_LOCAL);

	const float distFromPlayerSquared = (node_->GetWorldPosition() - game->playerNode->GetWorldPosition()).LengthSquared();
	if (lifeTimer > 20.0f || distFromPlayerSquared > 40000.0f) Die();

	PostUpdate(timeStep);
}

void RobeLocksMissile::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* otherNode = dynamic_cast<Node*>(eventData["OtherNode"].GetPtr());
	RigidBody* otherBody = dynamic_cast<RigidBody*>(eventData["OtherBody"].GetPtr());

	if (otherNode == owner) return;
	if (otherBody->GetCollisionLayer() & 16)
	{
		ForceFieldResponse(otherNode, 5.0f);
		return;
	}

	Die();
}

void RobeLocksMissile::Die()
{
	if (!hit)
	{
		hit = true;

		Node* explosion = Zeus::MakeExplosion(scene, node_->GetWorldPosition(), 2.0f, 3.5f);
		Zeus::ApplyRadialDamage(scene, node_, 4.5f, Settings::ScaleWithDifficulty(7.0f, 10.0f, 13.0f), 132); //128 + 4

		SoundSource3D* s = explosion->CreateComponent<SoundSource3D>();
		s->SetSoundType("GAMEPLAY");
		s->Play(cache->GetResource<Sound>("Sounds/env_explode.wav"));

		killMe = true;
	}
}

Node* RobeLocksMissile::MakeRobeLocksMissile(Scene* scene, const Vector3 position, const Quaternion rotation, Node* owner, Node* target)
{
	RobeLocksMissile* r = new RobeLocksMissile(scene->GetContext());
	r->owner = owner;
	r->speed = 20.0f + Settings::ScaleWithDifficulty(-10.0f, 0.0f, 10.0f);
	if (Settings::GetDifficulty() > Settings::UNHOLY_THRESHOLD)
	{
		r->lockOnTimer = LOCK_ON_FREQUENCY;
	}
	r->target = target;
	
	Node* n = scene->InstantiateXML(scene->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_robelocks_missile.xml")->GetRoot(),
		position, rotation, LOCAL);
	n->AddComponent(r, 666, LOCAL);

	return n;
}

void RobeLocksMissile::RegisterObject(Context* context)
{
	context->RegisterFactory<RobeLocksMissile>();
}

RobeLocksMissile::~RobeLocksMissile()
{
}
