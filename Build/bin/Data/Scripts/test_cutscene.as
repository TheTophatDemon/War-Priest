int numTriggers = 0;
class TestCutscene : ScriptObject
{
	Gameplay@ game;
	Sound@ music;
	Node@ playerNode;
	Node@ modelNode;
	AnimationController@ animController;
	Actor@ actor;
	int eventNo;
	bool triggered = false;
	void DelayedStart()
	{
		playerNode = scene.GetChild("player");
		modelNode = cast<Player@>(playerNode.GetComponent("Player")).modelNode;
		animController = modelNode.GetComponent("AnimationController");
		actor = playerNode.GetComponent("Actor");
		game = scene.GetComponent("Gameplay");
		
		music = cache.GetResource("Sound", "Music/frownofthelord.ogg");
		music.looped = true;
		
		SubscribeToEvent(node, "NodeCollision", "OnCollision");
	}
	void Trigger()
	{
		SendEvent("CutsceneStart");
		triggered = true;
		numTriggers++;
		
		Print("This cutscene has been triggered " + numTriggers + " times.");
		if (numTriggers > 1) //Don't play the cutscene more than once
		{
			DoEvent(999);
			return;
		}
		
		actor.SetInputFPS(false, false, false, false);
		
		DoEvent(0);
		ScheduleEvent(1.0f, 1);
		ScheduleEvent(2.0f, 2);
		ScheduleEvent(3.0f, 3);
		ScheduleEvent(4.0f, 999);
	}
	void ScheduleEvent(float secs, int eventNo)
	{
		Array<Variant> param;
		param.Push(eventNo);
		DelayedExecute(secs, false, "void DoEvent(int)", param);
	}
	void DoEvent(int number)
	{
		eventNo = number;
		//Print(number);
		switch (number)
		{
			case 0:
				animController.PlayExclusive("Models/grungle_walk.ani", 0, true, 0.2f);
				break;
			case 1:
				animController.PlayExclusive("Models/grungle_idle.ani", 0, true, 0.2f);
				break;
			case 2:
				animController.PlayExclusive("Models/grungle_drown.ani", 0, true, 0.2f);
				break;
			case 3:
				animController.PlayExclusive("Models/enemy/temptemplar_revive.ani", 0, true, 0.2f);
				game.DisplayMessage("I DARE SAY WHAT THE DEVIL DID YOU JUST AUDACIOUSLY PROCLAIM ABOUT MY WELL-BEING, YOU TROLLOP? I'LL HAVE YOU KNOW I GRADUATED TOP OF MY CLASS IN THE GENTLEMEN'S SOCIETY OF SOPHISTICATED PERSONS AND HAVE ENGAGED IN NUMEROUS ENDEAVOURS UPON THE RUFFIANS DOWN THE STREET FROM MY ABODE, MIGHT I ALSO I ADD THAT I HAVE ACQUIRED OVER 300 PIECES OF ANTIQUE FURNITURE?", Color(1.0f, 0, 1.0f), 10.0f, 10000);
				break;
			case 999:
				SendEvent("CutsceneEnd");
				//Change music
				game.musicSource.Play(music);
				//Reset everything
				triggered = false;
				break;
		}
	}
	void FixedUpdate(float timeStep)
	{
		if (triggered)
		{
			switch(eventNo)
			{
				case 0:
				case 2:
					actor.SetInputFPS(true, false, false, false);
					break;
				case 1:
				case 3:
					actor.SetInputFPS(false, true, false, false);
					break;
			}
			actor.Move(timeStep);
		}
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		RigidBody@ otherBody = eventData["OtherBody"].GetPtr();
		if (otherNode.name == "player" && !triggered)
		{
			Trigger();
			UnsubscribeFromEvent(node, "NodeCollision");
		}
	}
}