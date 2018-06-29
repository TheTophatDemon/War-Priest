class MovingPlatform : ScriptObject
{
	float timer;
	float restTime;
	Vector3 movement;
	Vector3 origin;
	bool returning;
	MovingPlatform()
	{
		returning = false;
		restTime = 1.0f;
		movement = Vector3(0.0f,0.0f,0.0f);
		timer = 0;
	}
	void Start()
	{
		origin = node.worldPosition;
		SubscribeToEvent(node, "NodeCollision", "OnCollision");
	}
	void FixedUpdate(float timeStep)
	{
		timer += timeStep;
		if (timer > restTime)
		{
			timer = 0.0f;
		}
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		RigidBody@ otherBody = eventData["OtherBody"].GetPtr();
		if (otherBody.collisionLayer & 128 > 0)
		{
			state = STATE_OPENING;
		}
	}
}