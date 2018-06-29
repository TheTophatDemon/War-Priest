class Rotator : ScriptObject
{
	float speed = 100.0f;
	Vector3 axis = Vector3(0.0f, 1.0f, 0.0f);
	float timer = 0.0f;
	Rotator()
	{
	
	}
	void Start()
	{
		
	}
	void FixedUpdate(float timeStep)
	{
		timer += timeStep;
		node.rotation = Quaternion(timer * speed, axis);
	}
}