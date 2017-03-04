#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

class NPC : public LogicComponent
{
public:
	URHO3D_OBJECT(NPC, LogicComponent);
public:
	NPC(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~NPC();
protected:

};

