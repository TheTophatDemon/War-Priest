#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Gameplay;
class Player;
class God : public LogicComponent
{
	URHO3D_OBJECT(God, LogicComponent);
public:
	static StringHash E_BEAMED;


	God(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~God();
	void SetTarget(Node* n);
protected:
	WeakPtr<Node> target;
	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<Scene> scene;
	float stateTimer = 0.0f;
	int state = 0;
	bool beamed = false;
};

