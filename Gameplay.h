#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D\Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D\Graphics/DebugRenderer.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Audio/Audio.h>
#include "CompassScene.h"

using namespace Urho3D;

class GunPriest;
class Player;
class Gameplay : public LogicComponent
{
	URHO3D_OBJECT(Gameplay, LogicComponent);
public:
	GunPriest* gunPriest;
	
	SharedPtr<CompassScene> compassScene;

	WeakPtr<Node> cameraNode;
	WeakPtr<Camera> camera;

	SharedPtr<Node> playerNode;
	SharedPtr<Player> player;
	SharedPtr<Node> mapNode;

	Gameplay(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);

	void SetupGame();
	void MakeHUD();
	void FlashScreen(Color c, float spd);
	void DisplayMessage(String msg, Color col, float time);
	void Lose();
	void Win();

	~Gameplay();
	int restartTimer = 0;
	bool initialized = false;
	SharedPtr<UIElement> ourUI;

	int projectileCount = 0;
	int enemyCount = 0;
	int winState = 0;

protected:
	void GetNextFrame(Sprite* spr, int cellWidth, int cellHeight, int cellCount);
	void UpdateHUD(float timeStep);
	void SetupLighting();
	
	void SetupEnemy();
	void SetupProps();
	void ExtractLiquidsFromMap();

	SharedPtr<Node> skybox;
	SharedPtr<Node> exitNode;
	
	SharedPtr<Text> text;
	SharedPtr<Text> messageText;
	SharedPtr<Text> projectileCounter;
	SharedPtr<Sprite> healthMeter;
	SharedPtr<Sprite> compass1;

	SharedPtr<ResourceCache> cache;
	Engine* engine_;
	SharedPtr<Scene> scene_;
	Input* input;
	SharedPtr<Viewport> viewport;
	SharedPtr<Renderer> renderer;
	SharedPtr<Audio> audio;
	SharedPtr<PhysicsWorld> physworld;

	Color flashColor;
	float flashSpeed;
	float oldHealth;
	float messageTimer = 0.0f;
};

