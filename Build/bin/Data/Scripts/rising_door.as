const int STATE_CLOSED = 0;
const int STATE_OPENING = 1;
const int STATE_OPEN = 2;
const int STATE_CLOSING = 3;

class RisingDoor : ScriptObject
{
	float timer;
	int state;
	float openSpeed;
	float openTime;
	float openHeight;
	RigidBody@ body;
	Vector3 origin;
	RisingDoor()
	{
		state = STATE_CLOSED;
		openSpeed = 0.1f;
		openTime = 2.0f;
		openHeight = 4.8f;
		timer = 0;
	}
	void Start()
	{
		origin = node.worldPosition;
		body = node.CreateComponent("RigidBody");
		body.friction = 0.0f;
		body.kinematic = true;
		body.collisionLayer = 1;
		body.collisionMask = 140;
		SubscribeToEvent(node, "NodeCollision", "OnCollision");
	}
	void FixedUpdate(float timeStep)
	{
		switch(state)
		{
			case STATE_CLOSED:
				
				break;
			case STATE_OPENING:
				node.position = node.position + Vector3(0.0f, openSpeed, 0.0f);
				if (node.worldPosition.y > origin.y + openHeight)
				{
					node.worldPosition = origin + Vector3(0.0f, openHeight, 0.0f);
					state = STATE_OPEN;
					timer = openTime;
				}
				break;
			case STATE_OPEN:
				timer -= timeStep;
				if (timer <= 0.0f)
				{
					timer = 0.0f;
					state = STATE_CLOSING;
				}
				break;
			case STATE_CLOSING:
				node.position = node.position - Vector3(0.0f, openSpeed, 0.0f);
				if (node.worldPosition.y < origin.y)
				{
					node.worldPosition = origin;
					state = STATE_CLOSED;
				}
				break;
		}
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		RigidBody@ otherBody = eventData["OtherBody"].GetPtr();
		if (otherBody.collisionLayer & 128 > 0 || 
			otherBody.collisionLayer & 8 > 0 || 
			otherBody.collisionLayer & 4 > 0)
		{
			state = STATE_OPENING;
		}
	}
}