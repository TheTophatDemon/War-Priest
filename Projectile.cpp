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
}

void Projectile::FixedUpdate(float timeStep)
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
			float distance = (otherNode->GetWorldPosition() - node_->GetWorldPosition()).Length();
			if (distance < radius)
			{
				if (otherNode->HasComponent<Enemy>())
					otherNode->GetComponent<Enemy>()->OnHurt(node_, damage);
				else if (otherNode == game->playerNode)
					game->player->OnHurt(node_, damage);
				Destroy();
				break;
			}
		}
	}
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition(), movement.Normalized()), radius, 2);
	if (result.body_)
	{
		Destroy();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	node_->Translate(movement, TS_WORLD);
	if (hit)
	{
		node_->Remove();
	}
}

void Projectile::Destroy()
{
	hit = true;
}

Projectile::~Projectile()
{
}
