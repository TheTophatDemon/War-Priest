#pragma once

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
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>

#include <time.h>

#include "Gameplay.h"
#include "Player.h"
#include "Actor.h"
#include "TempEffect.h"
#include "Enemy.h"
#include "TitleScreen.h"
#include "PyroPastor.h"
#include "Fireball.h"
#include "God.h"
#include "Lift.h"
#include "PostalPope.h"
#include "Debris.h"
#include "Water.h"
#include "WeakChild.h"
#include "SoundSounder.h"
#include "DangerDeacon.h"
#include "Launchpad.h"
#include "CompassScene.h"
#include "TempTemplar.h"
#include "Statue.h"
#include "ChaosCaliph.h"
#include "Blackstone.h"
#include "Bonus.h"
#include "KilledKaaba.h"
#include "BouncyFireball.h"
#include "MissileFinder.h"
#include "TempShield.h"

class GunPriest : public Application
{
	friend class Gameplay;
public:
	SharedPtr<DebugHud> debugHud;
	SharedPtr<DebugRenderer> debugRenderer;
	
	static int STATE_GAME;
	static int STATE_TITLE;

	GunPriest(Context* context);
	void StartGame(String path = "Scenes/palisadepantheon.xml");
	void VideoSetup();

	virtual void Setup();
	virtual void Start();
	void ChangeState(int newState);
	void Update(StringHash eventType, VariantMap& eventData);
	virtual void Stop();
protected:
	void AfterRenderUpdate(StringHash eventType, VariantMap& eventData);

	SharedPtr<Text> loadingText;
	SharedPtr<Gameplay> game;
	SharedPtr<TitleScreen> titleScreen;
	SharedPtr<Viewport> viewport;
	SharedPtr<Scene> scene_;
	SharedPtr<Renderer> renderer;
	SharedPtr<Input> input;
	SharedPtr<ResourceCache> cache;
	SharedPtr<UI> ui;
	SharedPtr<Audio> audio;

	int state = 1;
	bool pause;
};

