#include "Projectile.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Physics/CollisionShape.h>

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
		/////////////////////////////////////////////////////////////////////////////////////////
		PODVector<Node*> nodes;
		scene->GetChildrenWithTag(nodes, "shootable", true);
		for (PODVector<Node*>::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
		{
			Node* otherNode = (Node*)*i;
			if (otherNode && otherNode != owner)
			{
				CollisionShape* shape = otherNode->GetComponent<CollisionShape>();
				if (shape) 
				{
					Vector3 nodePos = node_->GetWorldPosition(); nodePos.y_ = 0.0f; //Get the 2d distance first.
					Vector3 otherPos = otherNode->GetWorldPosition(); otherPos.y_ = 0.0f;
					float distance = (otherPos - nodePos).Length();
					if (distance < radius + shape->GetSize().x_)
					{
						if (node_->GetWorldPosition().y_ + radius > otherNode->GetWorldPosition().y_ && node_->GetWorldPosition().y_ - radius <= otherNode->GetWorldPosition().y_ + shape->GetSize().y_ * 2.0f)
						{
							Enemy* enm = otherNode->GetDerivedComponent<Enemy>();
							if (enm)
							{
								if (enm->state == 0) break;
								enm->OnHurt(node_, damage);
							}
							else if (otherNode == game->playerNode)
							{
								game->player->OnHurt(node_, damage);
							}
							Destroy();
						}
						break;
					}
				}
			}
		}
		PhysicsRaycastResult result;
		physworld->RaycastSingle(result, Ray(node_->GetWorldPosition(), movement.Normalized()), radius, 2);
		if (result.body_)
		{
			Destroy();
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
	timer = 0;
}

Projectile::~Projectile()
{
	
}
