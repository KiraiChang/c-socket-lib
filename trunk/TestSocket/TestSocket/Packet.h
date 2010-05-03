class Packet
{
public:
	char message[80];
	int size;
	Packet()
	{
		Init();
	}
	void Init(void)
	{
		ZeroMemory(message, sizeof(message));
		size = 0;
	}
};