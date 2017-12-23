#include "Fireball.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <iostream>

Fireball::Fireball(Context* context) : Projectile(context)
{
	//Properties are set in the MakeFireball function
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
		if (lifeTimer > 2.0f) OnHit(nullptr);
	}
	Projectile::FixedUpdate(timeStep);
}

void Fireball::Move(const float timeStep)
{
	movement = node_->GetWorldRotation() * (Vector3::FORWARD * speed * timeStep);
}

void Fireball::OnHit(Node* n)
{
	Projectile::OnHit(n);
	if (emitter)
	{
		emitter->SetEmitting(false);
	}
}

//Components can't have constructors that take more than 1 parameter for some reason...
Node* Fireball::MakeFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner)
{
	Fireball* p = new Fireball(sc->GetContext());
	p->owner = owner;
	p->speed = 30.0f;
	p->damage = 10;
	p->radius = 0.5f;

	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_fireball.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(rotation);
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
