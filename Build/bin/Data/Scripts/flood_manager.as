class FloodManager : ScriptObject
{
	Sound@ reviveSound;
	Sound@ quakeSound;
	SoundSource@ sounder;
	Material@ seaMat;
	Node@ seaNode;
	Gameplay@ game;
	bool triggered = false;
	float timer = 0.0f;
	float lastBlinkTime = 0.0f;
	void DelayedStart()
	{
		game = scene.GetComponent("Gameplay");
		seaNode = scene.GetChild("sea");
		StaticModel@ sm = seaNode.GetComponent("StaticModel");
		seaMat = sm.materials[0];
		
		sounder = node.CreateComponent("SoundSource");
		sounder.soundType = "GAMEPLAY";
		reviveSound = cache.GetResource("Sound", "Sounds/ply_revive_big.wav");
		quakeSound = cache.GetResource("Sound", "Sounds/env_quake.wav");
		
		SubscribeToEvent(node, "NodeCollisionStart", "OnCollision");
	}
	void FixedUpdate(float timeStep)
	{
		if (triggered)
		{
			float lastTime = timer;
			timer += timeStep;
			if (timer > 4.0f && timer < 17.0f)
			{
				float diff = timer - lastBlinkTime;
				if (diff > 1.0f)
				{
					lastBlinkTime = timer;
					game.DisplayMessage("WARNING! FLOOD IS IMMINENT!", Color(1.0f, 1.0f, 0.0f), 0.8f, 1000000);
				}
				else if (diff > 0.75f)
				{
					game.DisplayMessage(" ", Color(1.0f, 1.0f, 0.0f), 0.5f, 1000000);
				}
			}
			else if (timer > 17.0f)
			{
				if (lastTime < 17.0f) 
				{
					seaNode.AddTag("instant_kill");
					sounder.Play(quakeSound, 44100.0f, 0.5f);
				}
				seaNode.Translate(Vector3(0.0f, timeStep, 0.0f));
				game.ShakeScreen(1.0f);
			}
		}
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		if (otherNode.name == "player" && !triggered)
		{
			triggered = true;
			//Make the sea of bodies flash
			ValueAnimation@ flash = ValueAnimation();
			flash.SetKeyFrame(0.0f, Variant(Color(0.5f, 0.5f, 0.5f)));
			flash.SetKeyFrame(2.0f, Variant(Color(30.0f, 30.0f, 30.0f)));
			flash.SetKeyFrame(4.0f, Variant(Color(0.5f, 0.5f, 0.5f)));
			seaMat.SetShaderParameterAnimation("MatDiffColor", flash, WM_CLAMP, 1.0f);
			sounder.Play(reviveSound);
		}
	}
}