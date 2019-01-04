#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>

using namespace Urho3D;

class RobeLocksBomb : public LogicComponent
{
	URHO3D_OBJECT(RobeLocksBomb, LogicComponent);
public:
	RobeLocksBomb(Context* context);
	static void RegisterObject(Context* context);
	static Node* MakeRobeLocksBomb(Scene* scene, const Vector3 position);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
	~RobeLocksBomb();
protected:
	SharedPtr<ResourceCache> cache;
	SharedPtr<SoundSource3D> tickSource;
	float tickTimer;
	float tickSpeed;
	float lifeTimer;
};

