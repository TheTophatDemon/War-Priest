#include "Blackstone.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "Gameplay.h"

#define STATE_BOUNCE 0
#define STATE_CHASE 1
#define STATE_STAGNATE 2

Blackstone::Blackstone(Context* context) : Projectile(context),
	state(STATE_BOUNCE),
	stateTimer(0.0f),
	moveDir(Vector3::ZERO)
{
	checkCollisionsManually = false;
}

void Blackstone::Start()
{
	Projectile::Start();
	target = game->playerNode;
	assert(node_->HasComponent<ParticleEmitter>());
	emitter = node_->GetComponent<ParticleEmitter>();
	assert(node_->HasComponent<RigidBody>());
	body = node_->GetComponent<RigidBody>();

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Blackstone, OnPhysicsCollision));
}

void Blackstone::RegisterObject(Context* context)
{
	context->RegisterFactory<Blackstone>();
}

void Blackstone::FixedUpdate(float timeStep)
{
	Projectile::FixedUpdate(timeStep);
	switch (state) 
	{
	case STATE_BOUNCE:
		if (lifeTimer > 3.0f)
		{
			ChangeState(STATE_CHASE);
			body->SetLinearVelocity(Vector3::ZERO);
			body->SetEnabled(false);
			//UnsubscribeFromEvent(GetNode(), E_NODECOLLISION);
			checkCollisionsManually = true;
		}
		break;
	case STATE_STAGNATE:
		if (speed < 0.0f) speed = orgSpeed;
		stateTimer += timeStep;
		if (stateTimer > 0.5f)
		{
			ChangeState(STATE_CHASE);
		}
		break;
	case STATE_CHASE:
		stateTimer += timeStep;
		if (stateTimer > 1.0f)
		{
			ChangeState(STATE_STAGNATE);
		}
		break;
	}
	
	if (lifeTimer > 24.0f)
	{
		node_->Remove();
	}
}

void Blackstone::Move(const float timeStep)
{
	if (state == STATE_CHASE) 
	{
		if (target.Get())
			moveDir = (target->GetWorldPosition() + Vector3(0.0f, 3.5f, 0.0f) - node_->GetWorldPosition()).Normalized();
		movement = moveDir * speed * timeStep;
	}
	else
	{
		movement = Vector3::ZERO;
	}
}

void Blackstone::ChangeState(const int newState)
{
	state = newState;
	stateTimer = 0;
}

void Blackstone::OnHit(Node* n)
{
	if (n->HasComponent<RigidBody>()) 
	{
		RigidBody* rb = n->GetComponent<RigidBody>();
		if (state != STATE_BOUNCE && 
			(rb->GetCollisionLayer() & 2 || rb->GetCollisionLayer() & 128))
		{
			lifeTimer = 100.0f;
		}
	}
}

void Blackstone::OnPhysicsCollision(StringHash eventType, VariantMap& eventData)
{
	if (state == STATE_BOUNCE) 
	{
		Node* other = (Node*)eventData["OtherNode"].GetPtr();
		RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
		if (otherBody->GetCollisionLayer() & 2)
		{
			VectorBuffer contacts = eventData["Contacts"].GetBuffer();
			while (!contacts.IsEof())
			{
				Vector3 position = contacts.ReadVector3();
				Vector3 normal = contacts.ReadVector3();
				float distance = contacts.ReadFloat();
				float impulse = contacts.ReadFloat();
				body->ApplyImpulse(normal * body->GetMass() * 3.5f);
			}
		}
	}
}

Node* Blackstone::MakeBlackstone(Scene* sc, Vector3 position, Vector3 impulse, Node* owner)
{
	Blackstone* p = new Blackstone(sc->GetContext());
	p->owner = owner;
	p->speed = 15.0f;
	p->damage = 20;
	p->radius = 0.5f;

	Node* n = sc->CreateChild();
	n->LoadXML(p->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_blackstone.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
	n->AddComponent(p, 333, LOCAL);

	n->GetComponent<RigidBody>()->ApplyImpulse(impulse);
	return n;
}

Blackstone::~Blackstone()
{
}
