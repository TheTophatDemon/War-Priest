#include "God.h"
#include "Gameplay.h"
#include "Player.h"
#include "Zeus.h"

#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Core/Context.h>
#include <iostream>

using namespace Urho3D;

#define STATE_DROP 0
#define STATE_BEAM 1
#define STATE_RISE 2

StringHash God::E_BEAMED = StringHash("GodBeamed");

God::God(Context* context) : LogicComponent(context)
{
	
}

void God::RegisterObject(Context* context)
{
	context->RegisterFactory<God>();
}

void God::Start()
{
	scene = GetScene();
	cache = GetSubsystem<ResourceCache>();
	game = scene->GetComponent<Gameplay>();
}

void God::SetTarget(Node* n)
{
	target = n;
	node_->SetWorldPosition(target->GetWorldPosition() + Vector3(0.0f, 50.0f, 0.0f));
	stateTimer = 0.0f;
	state = STATE_DROP;
}

void God::FixedUpdate(float timeStep)
{
	if (target.Get()) 
	{
		Vector3 targetPos = target->GetWorldPosition();
		node_->SetWorldPosition(Vector3(targetPos.x_, node_->GetWorldPosition().y_, targetPos.z_));
		float diff = fabs(targetPos.y_ - node_->GetWorldPosition().y_);

		node_->Rotate(Quaternion(timeStep * 200.0f, Vector3::UP), TS_LOCAL);

		switch (state)
		{
		case STATE_DROP:
			stateTimer += timeStep;
			if (diff > 10.0f)
			{
				node_->Translate(Vector3(0.0f, -timeStep * 30.0f, 0.0f), TS_LOCAL);
			}
			else
			{
				state = STATE_BEAM;
				stateTimer = 0.0f;
			}
			break;
		case STATE_BEAM:
			stateTimer += timeStep;
			if (!beamed && stateTimer > 0.5f)
			{
				Node* beam = Zeus::MakeLightBeam(scene, Vector3(targetPos.x_, node_->GetWorldPosition().y_ - 513.0f, targetPos.z_));
				//Max out the beam's volume so you can hear it from all the way down there.
				beam->GetComponent<SoundSource3D>()->SetDistanceAttenuation(1.0f, 1000.0f, 0.5f);
				beamed = true;
				VariantMap map = VariantMap();
				map.Insert(Pair<StringHash, Variant>(StringHash("target"), target.Get()));
				SendEvent(E_BEAMED, map);
			}
			if (stateTimer > 2.0f)
			{
				stateTimer = 0.0f;
				state = STATE_RISE;
			}
			break;
		case STATE_RISE:
			stateTimer += timeStep;
			node_->Translate(Vector3(0.0f, stateTimer * 2.0f, 0.0f), TS_LOCAL);
			break;
		}
	}
}

God::~God()
{
}
