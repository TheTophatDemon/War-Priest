#include "Projectile.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <iostream>

#include "Enemy.h"
#include "Gameplay.h"
#include "Player.h"

#define TYPE_FIREBALL 0

Projectile::Projectile(Context* context) : LogicComponent(context)
{
	projectileType = TYPE_FIREBALL;
	radius = 0.5f;
	damage = 10;
	speed = 30.0f;
	hit = false;
	movement = Vector3::ZERO;
	timer = 0;
	lifeTimer = 0;

	emitter = nullptr;
}

void Projectile::RegisterObject(Context* context)
{
	context->RegisterFactory<Projectile>();
}

void Projectile::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	if (node_->HasComponent<ParticleEmitter>())
		emitter = node_->GetComponent<ParticleEmitter>();
}

void Projectile::FixedUpdate(float timeStep)
{
	lifeTimer += timeStep;
	if (lifeTimer > 2.0f && !hit) Destroy();
	if (hit)
	{
		timer += 1;
		switch (projectileType)
		{
		case TYPE_FIREBALL:
			if (emitter->GetNumParticles() <= 1 || timer > 100)
			{
				node_->Remove();
			}
			break;
		default:
			node_->Remove();
			break;
		}
	}
	else //////////////////////////////////////////////////////////
	{
		switch (projectileType)
		{
		case TYPE_FIREBALL:
			movement = node_->GetWorldRotation() * (Vector3::FORWARD * speed * timeStep);
			break;
		}

		PhysicsRaycastResult result;
		physworld->SphereCast(result, Ray(node_->GetWorldPosition(), movement.Normalized()), radius, radius, 210);//128+64+2+16
		if (result.body_)
		{
			if (result.body_->GetNode() != owner) 
			{
				int colLayer = result.body_->GetCollisionLayer();
				if (colLayer & 128 || colLayer & 64)
				{
					VariantMap map = VariantMap();
					map.Insert(Pair<StringHash, Variant>(StringHash("perpetrator"), node_));
					map.Insert(Pair<StringHash, Variant>(StringHash("victim"), result.body_->GetNode()));
					map.Insert(Pair<StringHash, Variant>(StringHash("damage"), damage));
					SendEvent(StringHash("ProjectileHit"), map);
				}
				Destroy();
			}
		}
		
		node_->Translate(movement, TS_WORLD);
	}
}

void Projectile::Destroy()
{
	hit = true;
	if (emitter)
	{
		emitter->SetEmitting(false);
	}
	if (node_->HasComponent<StaticModel>())
		node_->RemoveComponent<StaticModel>();
	timer = 0;
}

Projectile::~Projectile()
{
	
}

Node* Projectile::MakeProjectile(Scene* sc, String name, Vector3 position, Quaternion rotation, Node* owner) //Since components can't have constructor parameters...
{
	Projectile* p = new Projectile(sc->GetContext());
	p->owner = owner;
	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_" + name + ".xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(rotation);
	
	if (name == "fireball")
	{
		p->projectileType = 0;
	}
	n->AddComponent(p, 333, LOCAL);
	return n;
}