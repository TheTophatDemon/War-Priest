#include "Fireball.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <iostream>
#include "Settings.h"
#include "Gameplay.h"

Fireball::Fireball(Context* context) : Projectile(context),
	deathTime(0.0f),
	speed(30.0f),
	damage(10)
{
}

void Fireball::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);

	Projectile::Start();
	if (node_->HasComponent<ParticleEmitter>())
		emitter = node_->GetComponent<ParticleEmitter>();

	SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Fireball, OnCollision));
}

void Fireball::RegisterObject(Context* context)
{
	context->RegisterFactory<Fireball>();
}

void Fireball::FixedUpdate(float timeStep)
{
	PreUpdate(timeStep);

	if (hit)
	{
		if (emitter->GetNumParticles() <= 1 || lifeTimer - deathTime > 1.0f)
		{
			killMe = true;
		}
	}
	else
	{
		node_->Translate(Vector3::FORWARD * speed * timeStep, TS_LOCAL);

		const float distFromPlayerSquared = (node_->GetWorldPosition() - game->playerNode->GetWorldPosition()).LengthSquared();
		if (lifeTimer > 2.0f || distFromPlayerSquared > 40000.0f)
		{
			Die();
		}
	}

	PostUpdate(timeStep);
}

void Fireball::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* otherNode = dynamic_cast<Node*>(eventData["OtherNode"].GetPtr());
	RigidBody* otherBody = dynamic_cast<RigidBody*>(eventData["OtherBody"].GetPtr());

	if (otherNode == owner) return;
	if (otherBody->GetCollisionLayer() & 4)
	{
		DoDamage(otherNode, damage);
	}
	if (otherBody->GetCollisionLayer() & 16)
	{
		ForceFieldResponse(otherNode, 5.0f);
		return;
	}

	Die();
}

void Fireball::Die()
{
	if (!hit)
	{
		deathTime = lifeTimer;
		hit = true;
		node_->RemoveTag("projectile");
		if (emitter)
		{
			emitter->SetEmitting(false);
		}
		node_->RemoveComponents<RigidBody>();
		node_->RemoveComponents<CollisionShape>();
	}
}

Node* Fireball::MakeFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner)
{
	Fireball* p = new Fireball(sc->GetContext());
	p->owner = owner;
	p->speed = 30.0f;
	p->damage = 10;

	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_fireball.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(rotation);
	if (Settings::GetDifficulty() > Settings::UNHOLY_THRESHOLD) 
	{
		n->SetScale(1.5f); 
	}
	n->AddComponent(p, 333, LOCAL);
	return n;
}

Node* Fireball::MakeBlueFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner)
{
	Fireball* p = new Fireball(sc->GetContext());
	p->owner = owner;
	p->speed = 20.0f;
	p->damage = 10;

	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_fireball_blue.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(rotation);
	n->AddComponent(p, 333, LOCAL);

	return n;
}

Fireball::~Fireball()
{
}
