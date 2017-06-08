#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/StaticModel.h>

#include "Enemy.h"

using namespace Urho3D;

class Actor;
class DangerDeacon : public Enemy //He's dangerous. He's out of control!
{
	URHO3D_OBJECT(DangerDeacon, LogicComponent);
public:
	DangerDeacon(Context* context);
	static void RegisterObject(Context* context);
	virtual void DelayedStart();
	virtual void Revive();
	~DangerDeacon();
protected:
	float strafeAmt;
	virtual void Execute();
	virtual void Dead();
	virtual void EnterState(const int newState);
	virtual void LeaveState(const int oldState);
	SharedPtr<Node> orbThing;
	SharedPtr<StaticModel> orbModel;
};

