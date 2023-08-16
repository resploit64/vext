namespace settings
{
	bool menu_enabled = false;

	namespace esp
	{
		enum class BoxType
		{
			Full,
			Corner
		};
		bool head_circle = false;
		bool health_bar = true;
		bool shield_bar = false;
		bool name = true;
		bool gun = true;
		bool distance = false;
		bool boxes = false;
		BoxType box_type;
	}
}