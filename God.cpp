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
	node_->SetWorldPosition(game->playerNode->GetWorldPosition() + Vector3(0.0f, 50.0f, 0.0f));
}

void God::FixedUpdate(float timeStep)
{
	Vector3 plyPos = game->playerNode->GetWorldPosition();
	node_->SetWorldPosition(Vector3(plyPos.x_, node_->GetWorldPosition().y_, plyPos.z_));
	float diff = fabs(plyPos.y_ - node_->GetWorldPosition().y_);

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
			Zeus::MakeLightBeam(scene, Vector3(plyPos.x_, node_->GetWorldPosition().y_ - 32.5f, plyPos.z_));
			beamed = true;
			game->playerNode->SetParent(scene);
			game->playerNode->RemoveAllChildren();
			game->playerNode->Remove();
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

God::~God()
{
}
