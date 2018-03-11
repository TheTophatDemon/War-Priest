#include "BouncyFireball.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <iostream>
#include "Settings.h"

BouncyFireball::BouncyFireball(Context* context) : Projectile(context)
{
}

void BouncyFireball::Start()
{
	Projectile::Start();
	if (node_->HasComponent<ParticleEmitter>())
		emitter = node_->GetComponent<ParticleEmitter>();
}

void BouncyFireball::RegisterObject(Context* context)
{
	context->RegisterFactory<BouncyFireball>();
}

void BouncyFireball::FixedUpdate(float timeStep)
{
	if (hit)
	{
		if (emitter->GetNumParticles() <= 1 || deathTimer > 1.0f)
		{
			node_->Remove();
		}
	}
	else
	{
		if (lifeTimer > 10.0f) OnHit(PhysicsRaycastResult());
	}
	Projectile::FixedUpdate(timeStep);
}

void BouncyFireball::Move(const float timeStep)
{
	movement = node_->GetWorldRotation() * ( (Vector3::FORWARD * speed * timeStep) + Vector3(cosf(lifeTimer * 10.0f) * 0.5f, 0.0f, 0.0f) );
}

void BouncyFireball::OnHit(PhysicsRaycastResult result)
{
	node_->SetWorldRotation(Quaternion(Vector3::ZERO, result.normal_));
}

BouncyFireball::~BouncyFireball()
{
}

Node* BouncyFireball::MakeBouncyFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner)
{
	BouncyFireball* p = new BouncyFireball(sc->GetContext());
	p->owner = owner;
	p->speed = 40.0f + Settings::ScaleWithDifficulty(-10.0f, 0.0f, 10.0f);
	p->damage = 20;
	p->radius = 2.5f;
	p->limitRange = false;

	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_fireball_green.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(rotation);
	n->AddComponent(p, 333, LOCAL);

	return n;
}