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
	deltaTime(0.0f),
	state(0),
	targetOffset(0.0f,0.0f,0.0f)
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
	WeakChild::MakeWeakChild(emitterNode, node_);

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
		if (lifeTimer > 25.0f)
		{
			OnHit(PhysicsRaycastResult());
			return;
		}
	}
	
	Projectile::FixedUpdate(timeStep);
}

void Missile::Move(const float timeStep)
{
	switch (state)
	{
	case 0:
		speed -= (timeStep * 28.0f) * Sign(orgSpeed);
		if (speed <= 0.0f)
		{
			targetOffset = Vector3(Random(-1.0f, 1.0f), 2.0f, Random(-1.0f, 1.0f));
			state++;
		}
		break;
	case 1:
	{
		if (target.Get() != nullptr)
		{
			const Vector3 diff = target->GetWorldPosition() + targetOffset - node_->GetWorldPosition();
			Quaternion dest = Quaternion();
			dest.FromLookRotation(diff.Normalized(), Vector3::UP);
			node_->SetWorldRotation(node_->GetWorldRotation().Slerp(dest, 5.0f * timeStep));
			speed += (timeStep * 20.5f) * Sign(orgSpeed);
			if (fabs(speed) > fabs(orgSpeed))
			{
				speed = orgSpeed;
			}
			if (diff.Length() < 20.0f && fabs(speed) >= fabs(orgSpeed) / 2.0f)
			{
				state++;
			}
		}
		break;
	}
	case 2:
		//Just keep going until we explode
		break;
	case 3: //For when it's deflected by a shield
		speed += (timeStep * 30.0f) * Sign(orgSpeed);
		if (fabs(speed - orgSpeed) <= 0.0f) speed = orgSpeed;
		break;
	}
	movement = node_->GetWorldRotation() * (Vector3::FORWARD * speed * timeStep);
}

void Missile::OnHit(PhysicsRaycastResult result)
{
	Projectile::OnHit(result);

	emitter->SetEmitting(false);

	Node* explosion = Zeus::MakeExplosion(scene, node_->GetWorldPosition(), 2.0f, 3.5f);
	Zeus::ApplyRadialDamage(scene, node_, 4.0f, Settings::ScaleWithDifficulty(7.0f, 10.0f, 13.0f), 132); //128 + 4

	SoundSource3D* s = explosion->CreateComponent<SoundSource3D>();
	s->SetSoundType("GAMEPLAY");
	s->Play(cache->GetResource<Sound>("Sounds/env_explode.wav"));

	node_->RemoveComponent<StaticModel>(); //The smoke trail is WeakParented to it, so instead of removing it we make it empty.
	node_->RemoveComponent<RigidBody>();
	node_->RemoveComponent<CollisionShape>();

	TempEffect* t = new TempEffect(context_);
	t->life = 2.0f;
	node_->AddComponent(t, 1212, LOCAL);
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
				OnHit(PhysicsRaycastResult());
			}
		}
		else if (otherBody->GetCollisionLayer() & 16) //SHIELD!
		{
			if (other->HasTag("tempshield")) 
			{
				Vector3 diff = (node_->GetWorldPosition() - other->GetWorldPosition()).Normalized();
				Quaternion quatty = node_->GetWorldRotation();
				quatty.FromLookRotation(diff, Vector3::UP);
				node_->SetWorldRotation(node_->GetWorldRotation().Slerp(quatty, 2.0f * deltaTime));
				state = 3;
			}
		}
	}
}

Node* Missile::MakeMissile(Scene* sc, Vector3 position, Quaternion rotation, Node* owner, Node* target)
{
	Missile* m = new Missile(sc->GetContext());
	m->owner = owner;
	m->speed = 50.0f + Settings::ScaleWithDifficulty(-20.0f, 0.0f, 20.0f);
	m->damage = 0;
	m->radius = 0.5f;
	m->limitRange = false;
	m->target = WeakPtr<Node>(target);

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
