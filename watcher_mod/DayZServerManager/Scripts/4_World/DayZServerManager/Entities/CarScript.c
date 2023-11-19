modded class CarScript
{
	void CarScript()
	{
		if(this != NULL)
			DayZServerManagerContainer.registerVehicle(this);
	}

    void ~CarScript()
	{
		if(this != NULL)
			DayZServerManagerContainer.unregisterVehicle(this);
    }
}
