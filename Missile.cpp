#include "Missile.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <iostream>
#include "Settings.h"
#include "TempEffect.h"
#include "Zeus.h"

Missile::Missile(Context* context) : Projectile(context),
	deltaTime(0.0f)
{
	checkCollisionsManually = false;
}

void Missile::Start()
{
	Projectile::Start();
	emitterNode = node_->GetChild("smoke");
	emitter = emitterNode->GetComponent<ParticleEmitter>();
	emitterNode->SetParent(GetScene());
	emitter->SetEmitting(true);

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Missile, OnCollision));
}

void Missile::RegisterObject(Context* context)
{
	context->RegisterFactory<Missile>();
}

void Missile::FixedUpdate(float timeStep)
{
	deltaTime = timeStep;
	if (!hit)
	{
		emitterNode->SetWorldPosition(node_->GetWorldPosition());
		if (lifeTimer > 10.0f)
		{
			node_->Remove();
			emitterNode->Remove();
			return;
		}
	}
	
	Projectile::FixedUpdate(timeStep);
}

void Missile::Move(const float timeStep)
{
	movement = node_->GetWorldRotation() * (Vector3::FORWARD * speed * timeStep);
}

void Missile::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != owner) 
	{
		if (otherBody->GetCollisionLayer() & 2 || otherBody->GetCollisionLayer() & 128
			|| otherBody->GetCollisionLayer() & 64)
		{
			if (!hit)
			{
				hit = true;

				emitter->SetEmitting(false);
				TempEffect* t = new TempEffect(context_);
				t->life = 2.0f;
				emitterNode->AddComponent(t, 1212, LOCAL);

				Zeus::MakeExplosion(scene, node_->GetWorldPosition(), 2.0f, 4.0f);
				Zeus::ApplyRadialDamage(scene, node_, 5.5f, Settings::ScaleWithDifficulty(10.0f, 12.0f, 15.0f), 132); //128 + 4

				node_->Remove();
			}
		}
		else if (otherBody->GetCollisionLayer() & 16)
		{
			node_->Rotate(Quaternion(deltaTime * 2.0f, Vector3::RIGHT), TS_LOCAL);
		}
	}
}

Node* Missile::MakeMissile(Scene* sc, Vector3 position, Quaternion rotation, Node* owner)
{
	Missile* m = new Missile(sc->GetContext());
	m->owner = owner;
	m->speed = 40.0f + Settings::ScaleWithDifficulty(-10.0f, 0.0f, 10.0f);
	m->damage = 0;
	m->radius = 0.5f;
	m->limitRange = false;

	Node* n = sc->CreateChild();
	n->LoadXML(m->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/missile.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(Quaternion(90.0f, Vector3::UP) * rotation);
	n->AddComponent(m, 333, LOCAL);

	return n;
}

Missile::~Missile()
{
}
