#include <string>
#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/AngelScript/Script.h>

#include "Gameplay.h"
#include "Player.h"

using namespace Urho3D;

class GunPriest : public Application
{
public:
	SharedPtr<Scene> scene_;
	Renderer* renderer;
	WeakPtr<Input> input;
	
	GunPriest(Context* context) : Application(context)
	{
		Gameplay::RegisterObject(context);
		Player::RegisterObject(context);
	}
	virtual void Setup()
	{
		engineParameters_["FullScreen"] = false;
		engineParameters_["WindowWidth"] = 1280;
		engineParameters_["WindowHeight"] = 720;
		engineParameters_["WindowResizable"] = false;
		engineParameters_["Multisample"] = 0;
		context_->RegisterSubsystem(new Script(context_));
	}
	virtual void Start()
	{
		input = WeakPtr<Input>(engine_->GetSubsystem<Input>());
		engine_->SetMaxFps(60);
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
		scene_ = new Scene(context_);
		renderer = GetSubsystem<Renderer>();

		XMLFile* mapFile = cache->GetResource<XMLFile>("Scenes/testmap.xml");
		scene_->LoadXML(mapFile->GetRoot());
		scene_->CreateComponent<Gameplay>();

		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GunPriest, Update));
	}
	void Update(StringHash eventType, VariantMap& eventData)
	{
		if (input->GetKeyDown(KEY_ESCAPE))
		{
			engine_->Exit();
			return;
		}
	}
	virtual void Stop()
	{
		
	}
};

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);