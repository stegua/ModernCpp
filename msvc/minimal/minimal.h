class minimal
{
private:
	static int num_instances;

public:
	minimal()
	{
		++num_instances;
	}
	~minimal()
	{
		--num_instances;
	}

	void print_num_instances();
};