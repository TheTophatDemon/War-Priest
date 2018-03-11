#include "Projectile.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <iostream>
#include <algorithm>

#include "Enemy.h"
#include "Gameplay.h"
#include "Player.h"

StringHash Projectile::E_PROJECTILEHIT = StringHash("ProjectileHit");

Projectile::Projectile(Context* context) : LogicComponent(context), 
	radius(0.5f), 
	damage(10), 
	speed(30.0f), 
	hit(false),
	orgSpeed(0.0f),
	lifeTimer(0),
	deathTimer(0.0f),
	movement(Vector3::ZERO),
	checkCollisionsManually(true),
	limitRange(true)
{
}

void Projectile::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	orgSpeed = speed;
}

void Projectile::FixedUpdate(float timeStep)
{
	lifeTimer += timeStep;
	if (hit)
	{
		deathTimer += timeStep;
	}
	else
	{
		const float dist2p = (game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).Length();
		if (dist2p > 250.0f && limitRange) { OnHit(PhysicsRaycastResult()); deathTimer = 9000.0f; }

		Move(timeStep);
		if (checkCollisionsManually && movement != Vector3::ZERO) //Check collisions
		{
			PhysicsRaycastResult result;
			const float movLength = movement.Length();
			physworld->SphereCast(result, Ray(node_->GetWorldPosition(), movement / movLength), radius, movLength, 214);//128+64+2+16+4
			if (result.body_)
			{
				if (result.body_->GetNode() != owner)
				{
					int colLayer = result.body_->GetCollisionLayer();
					if (colLayer & 4)
					{
						VariantMap map = VariantMap();
						map.Insert(Pair<StringHash, Variant>(StringHash("perpetrator"), node_));
						map.Insert(Pair<StringHash, Variant>(StringHash("victim"), result.body_->GetNode()));
						map.Insert(Pair<StringHash, Variant>(StringHash("damage"), damage));
						SendEvent(E_PROJECTILEHIT, map);
						OnHit(result);
					}
					else if (colLayer & 16) //Reflect yerself inside of the shield
					{
						if (result.body_->GetNode()->HasTag("tempshield"))
						{
							speed -= 2.5f;
							if (speed == 0.0f) speed = -orgSpeed;
						}
					}
					else
					{
						OnHit(result);
					}
				}
			}
			else
			{
				if (fabs(speed) < orgSpeed) //Reset speed after going through shield
				{
					if (speed < 0) speed = -orgSpeed;
					if (speed > 0) speed = orgSpeed;
				}
			}
		}
		node_->Translate(movement, TS_WORLD);
	}
}

void Projectile::OnHit(PhysicsRaycastResult result)
{
	if (!result.body_) return;
	if (result.body_->GetNode() == owner.Get()) return;
	hit = true;
	deathTimer = 0.0f;
}

Projectile::~Projectile()
{
}