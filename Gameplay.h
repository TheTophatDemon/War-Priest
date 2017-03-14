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
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Audio/Audio.h>


using namespace Urho3D;

class Player;
class Gameplay : public LogicComponent
{
	URHO3D_OBJECT(Gameplay, LogicComponent);
public:
	SharedPtr<ResourceCache> cache;
	Engine* engine_;
	SharedPtr<Scene> scene_;
	WeakPtr<Node> cameraNode;
	WeakPtr<Camera> camera;
	Input* input;
	SharedPtr<Text> text;
	SharedPtr<Viewport> viewport;
	SharedPtr<Renderer> renderer;
	SharedPtr<Audio> audio;

	WeakPtr<Sprite> crosshair;

	SharedPtr<Node> playerNode;
	WeakPtr<Player> player;
	SharedPtr<Node> boulderNode;

	Gameplay(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	virtual void OnSetEnabled();
	void SetupGame();
	void GetSettings();
	void MakeHUD(int width, int height);
	void FlashScreen(Color c, float spd);
	~Gameplay();
protected:
	void AfterRenderUpdate(StringHash eventType, VariantMap& eventData);
	void GetNextFrame(Sprite* spr, int cellWidth, int cellHeight, int cellCount);
	void SetOnFloor(Node* n, Vector3 pos);
	void UpdateHUD(float timeStep);
	void SetupCrosses();
	SharedPtr<Node> skybox;
	Color flashColor;
	float flashSpeed;
	int crossCount;

	SharedPtr<UIElement> ourUI;
	SharedPtr<Sprite> crossIcon;
	SharedPtr<Text> crossCounter;
};

