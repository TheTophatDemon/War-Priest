#include "KilledKaaba.h"

#include "Gameplay.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include "Settings.h"

#define STATE_DORMANT 0
#define STATE_RISE 1
#define STATE_FLY 2

#define HEIGHT_FROM_BOTTOM 9.0f
#define RISE_SPEED 500.0f

KilledKaaba::KilledKaaba(Context* context) : LogicComponent(context),
	distanceFromPlayer(0.0f),
	stateTimer(0.0f),
	state(-1),
	moveSpeed(1.0f),
	direction(1.0f, 0.0f, 0.0f),
	targetHeight(0.0f),
	lastState(-1)
{
}

void KilledKaaba::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	moveSpeed = 50.0f * Settings::GetDifficulty();
}

void KilledKaaba::Start()
{
	moveSpeed = 50.0f * Settings::GetDifficulty();

	SetUpdateEventMask(USE_FIXEDUPDATE);
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	
	body = node_->GetComponent<RigidBody>();
	body->SetLinearVelocity(Vector3::ZERO);

	targetHeight = node_->GetWorldPosition().y_ + HEIGHT_FROM_BOTTOM;

	glowNode = node_->CreateChild();
	StaticModel* sm = glowNode->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/fireglow.xml"));

	areas = Vector<SharedPtr<Node>>();
	PODVector<Node*> areas_ptrs = scene->GetChildrenWithTag("kaaba_area", true);
	for (Node* n : areas_ptrs)
	{
		n->RemoveComponent<StaticModel>();
		areas.Push(SharedPtr<Node>(n));
		SubscribeToEvent(n, E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnAreaCollision));
	}

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(KilledKaaba, OnSettingsChange));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnCollision));

	ChangeState(STATE_DORMANT);
}

void KilledKaaba::FixedUpdate(float timeStep)
{
	Vector3 plyPos = game->playerNode->GetWorldPosition(); 
	plyPos.y_ = 0.0f;
	Vector3 ourPos = node_->GetWorldPosition(); 
	ourPos.y_ = 0.0f;
	distanceFromPlayer = (ourPos - plyPos).Length();

	const float hDiff = targetHeight - node_->GetWorldPosition().y_;

	stateTimer += timeStep;
	switch (state)
	{
	case STATE_DORMANT:
		body->SetLinearVelocity(Vector3::ZERO);
		if (distanceFromPlayer < 40.0f)
		{
			ChangeState(STATE_RISE);
		}
		break;
	case STATE_RISE:
		if (Abs(hDiff) < 1.0f)
		{
			body->SetLinearVelocity(Vector3::ZERO);
			const int dir = -Sign(body->GetAngularVelocity().y_);
			body->ApplyTorqueImpulse(Vector3(0.0f, dir * timeStep * 500.0f, 0.0f));
			if (body->GetAngularVelocity().Length() < 1.0f)
			{
				body->SetAngularVelocity(Vector3::ZERO);
				ChangeState(STATE_FLY);
			}
		}
		else
		{
			if (lastState == STATE_DORMANT)
				body->SetLinearVelocity(Vector3(0.0f, timeStep * RISE_SPEED * Sign(hDiff) * 0.1f, 0.0f));
			else
				body->SetLinearVelocity(Vector3(0.0f, timeStep * RISE_SPEED * Sign(hDiff), 0.0f));
			body->ApplyTorqueImpulse(Vector3(0.0f, 500.0f * timeStep, 0.0f));
		}
		break;
	case STATE_FLY:
		node_->Rotate(Quaternion(timeStep * 100.0f, Vector3::UP), TS_LOCAL);
		const float currentSpeed = body->GetLinearVelocity().Length();
		if (currentSpeed < moveSpeed) 
		{
			body->ApplyForce(Vector3(direction.x_ * moveSpeed, 0.0f, direction.z_ * moveSpeed));
		}
		else if (currentSpeed > moveSpeed)
		{
			body->SetLinearVelocity(body->GetLinearVelocity().Normalized() * moveSpeed);
		}
		
		if (hDiff < -1.0f) 
		{
			const int buffer = body->GetCollisionLayer();
			body->SetCollisionLayer(0);

			PhysicsRaycastResult result;
			physworld->SphereCast(result, Ray(node_->GetWorldPosition(), Vector3::DOWN), 8.0f, 10.0f, 2U);
			if (result.body_ == nullptr || result.body_ == body) ChangeState(STATE_RISE);

			body->SetCollisionLayer(buffer);
		}
		else if (hDiff > 1.0f)
		{
			ChangeState(STATE_RISE);
		}
		break;
	}
}

void KilledKaaba::OnAreaCollision(StringHash eventType, VariantMap& eventData)
{
	RigidBody* body = (RigidBody*)eventData["Body"].GetPtr();
	Node* area = body->GetNode();
	Node* otherNode = (Node*)eventData["OtherNode"].GetPtr();
	if (otherNode->GetName() == "player")
	{
		for (SharedPtr<Node> a : areas)
		{
			if (a.Get() == area)
			{
				targetHeight = a->GetWorldPosition().y_ + HEIGHT_FROM_BOTTOM;
				break;
			}
		}
	}
}

void KilledKaaba::OnCollision(StringHash eventType, VariantMap& eventData)
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
			direction += normal * -Sign(distance);
		}
		direction.y_ = 0.0f;
		direction.Normalize();
		body->ApplyImpulse(Vector3(direction.x_ * moveSpeed, 0.0f, direction.z_ * moveSpeed));
	}
}

void KilledKaaba::ChangeState(const int newState)
{
	if (newState != state)
	{
		EnterState(newState);
		lastState = state;
		LeaveState(state);
		stateTimer = 0.0f;
	}
	state = newState;
}

void KilledKaaba::EnterState(const int newState)
{
	if (newState == STATE_DORMANT)
	{
		glowNode->SetEnabled(false);
		glowNode->SetScale(0.0f);
	}
	else if (newState == STATE_FLY)
	{
		direction = Vector3(Random(-1.0f, 1.0f), Random(-1.0f, 1.0f), Random(-1.0f, 1.0f));
		direction.Normalize();
	}
	else if (newState == STATE_RISE)
	{
		body->SetAngularFactor(Vector3::UP);
		body->SetLinearFactor(Vector3::ONE);
	}
}

void KilledKaaba::LeaveState(const int oldState)
{
	if (oldState == STATE_DORMANT)
	{
		glowNode->SetEnabled(true);
		glowNode->SetScale(8.25f);

		SharedPtr<ValueAnimation> valAnim(new ValueAnimation(context_));
		valAnim->SetKeyFrame(0.0f, Vector3(8.5f, 8.5f, 8.5f));
		valAnim->SetKeyFrame(0.5f, Vector3(9.0f, 9.0f, 9.0f));
		valAnim->SetKeyFrame(1.0f, Vector3(8.5f, 8.5f, 8.5f));

		glowNode->SetAttributeAnimation("Scale", valAnim, WM_LOOP, 1.0f);
	}
	else if (oldState == STATE_RISE)
	{
		body->SetAngularFactor(Vector3::ZERO);
		body->SetLinearFactor(Vector3(1.0f, 0.0f, 1.0f));
	}
}

KilledKaaba::~KilledKaaba()
{
}

void KilledKaaba::RegisterObject(Context* context)
{
	context->RegisterFactory<KilledKaaba>();
}