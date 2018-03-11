#include "Fireball.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <iostream>
#include "Settings.h"

Fireball::Fireball(Context* context) : Projectile(context),
	lifeTime(2.0f)
{
}

void Fireball::Start()
{
	Projectile::Start();
	if (node_->HasComponent<ParticleEmitter>())
		emitter = node_->GetComponent<ParticleEmitter>();
}

void Fireball::RegisterObject(Context* context)
{
	context->RegisterFactory<Fireball>();
}

void Fireball::FixedUpdate(float timeStep)
{
	if (hit)
	{
		if (emitter->GetNumParticles() <= 1 || deathTimer > 1.0f)
		{
			node_->Remove();
			//std::cout << "IM DEAD" << std::endl;
		}
	}
	else
	{
		if (lifeTimer > lifeTime) OnHit(PhysicsRaycastResult());
	}
	Projectile::FixedUpdate(timeStep);
}

void Fireball::Move(const float timeStep)
{
	movement = node_->GetWorldRotation() * (Vector3::FORWARD * speed * timeStep);
}

void Fireball::OnHit(PhysicsRaycastResult result)
{
	Projectile::OnHit(result);
	if (emitter)
	{
		emitter->SetEmitting(false);
	}
	//deathTimer = 1000.0f;
}

Node* Fireball::MakeFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner)
{
	Fireball* p = new Fireball(sc->GetContext());
	p->owner = owner;
	p->speed = 30.0f + Settings::ScaleWithDifficulty(0.0f, 0.0f, 10.0f);
	p->damage = 10;
	p->radius = 0.5f;

	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_fireball.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(rotation);
	if (Settings::GetDifficulty() > 1.4f) 
	{
		p->radius = 0.75f; 
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
	p->radius = 0.5f;

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
