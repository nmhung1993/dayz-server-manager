class DZSMDumpEntry : Managed
{
	string classNameTemp;
	string sourceTemp;

	ref TStringArray parents;

	void ~DZSMDumpEntry() {
		delete parents;
	}

	void Init(string classNameTemp, string sourceTemp)
	{
		this.classNameTemp = classNameTemp;
		this.sourceTemp = sourceTemp;

		parents = new TStringArray;
		string child = classNameTemp;
		string parent;
		while (GetGame().ConfigGetBaseName(sourceTemp + " " + child, parent))
		{
			if (parent && child != parent)
			{
				parents.Insert(parent);
			}
			else
			{
				break;
			}
			child = parent;
		}

	}
}

class DZSMBaseDumpEntry : DZSMDumpEntry
{
	string displayName;
	float hitPoints;
	
	float weight;
	ref TIntArray size;

	ref TIntArray repairableWithKits;
	ref TFloatArray repairCosts;

	ref TStringArray inventorySlot;
	string lootCategory;
	ref TStringArray lootTag;
	ref TStringArray itemInfo;

	void ~DZSMBaseDumpEntry() {
		delete size;
		delete repairableWithKits;
		delete repairCosts;
		delete inventorySlot;
		delete lootTag;
		delete itemInfo;
	}

	override void Init(string classNameTemp, string sourceTemp)
	{
		super.Init(classNameTemp, sourceTemp);

		displayName = GetGame().ConfigGetTextOut( sourceTemp + " " + classNameTemp + " displayName" );
		hitPoints = GetGame().ConfigGetFloat( sourceTemp + " " + classNameTemp + " DamageSystem GlobalHealth Health hitpoints" );

		weight = GetGame().ConfigGetFloat( sourceTemp +" " + classNameTemp + " weight" );
		size = new TIntArray;
		GetGame().ConfigGetIntArray( sourceTemp + " " + classNameTemp + " itemSize", size );

		repairableWithKits = new TIntArray;
		GetGame().ConfigGetIntArray( sourceTemp + " " + classNameTemp + " repairableWithKits", repairableWithKits );
		repairCosts = new TFloatArray;
		GetGame().ConfigGetFloatArray( sourceTemp + " " + classNameTemp + " repairCosts", repairCosts );

		inventorySlot = new TStringArray;
		GetGame().ConfigGetTextArray( sourceTemp + " " + classNameTemp + " inventorySlot", inventorySlot );
		
		lootCategory = GetGame().ConfigGetTextOut( sourceTemp + " " + classNameTemp + " lootCategory" );
		lootTag = new TStringArray;
		GetGame().ConfigGetTextArray( sourceTemp + " " + classNameTemp + " lootTag", lootTag );

		itemInfo = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgVehicles " + classNameTemp + " itemInfo", itemInfo);
	}
}


class DZSMAmmoDumpEntry : DZSMDumpEntry
{
	string displayName;
	string projectile;

	string simulation;

	float hit;
	float indirectHit;
	float indirectHitRange;

	float initSpeed;
	float typicalSpeed;
	float airFriction;
	
	bool tracer;
	bool explosive;
	float ttl;

	float weight;
	float caliber;
	float projectilesCount;
	float deflecting;
	
	float noiseHit;

	// ref TFloatArray damageOverride;
	float damageHP;
	float damageBlood;
	float damageShock;
	float damageArmor;


	void DZSMAmmoDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgMagazines");

		displayName = GetGame().ConfigGetTextOut( "cfgMagazines " + classNameTemp + " displayName" );
		projectile = GetGame().ConfigGetTextOut( "cfgMagazines " + classNameTemp + " ammo" );
		
		simulation = GetGame().ConfigGetTextOut( "cfgAmmo " + projectile + " simulation" );

		hit = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " hit" );
		indirectHit = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " indirectHit" );
		indirectHitRange = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " indirectHitRange" );
		initSpeed = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " initSpeed" );
		typicalSpeed = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " typicalSpeed" );
		airFriction = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " airFriction" );
		
		tracer = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " tracerStartTime" ) > -1.0;
		explosive = GetGame().ConfigGetInt( "cfgAmmo " + projectile + " explosive" ) > 0.0;
		ttl = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " timeToLive" );
		
		weight = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " weight" );
		caliber = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " caliber" );
		projectilesCount = Math.Max(1.0, GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " projectilesCount" ));
		deflecting = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " deflecting" );
		
		noiseHit = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " NoiseHit strength" );
		
		// damageOverride = GetGame().ConfigGetTextOut( "cfgAmmo " + projectile + " DamageApplied defaultDamageOverride" );
		// damageOverride = new TFloatArray;
		// GetGame().ConfigGetFloatArray( "cfgAmmo " + projectile + " DamageApplied defaultDamageOverride 0", damageOverride );
		
		damageArmor = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " DamageApplied Health armorDamage" );
		damageHP = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " DamageApplied Health damage" );
		damageBlood = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " DamageApplied Blood damage" );
		damageShock = GetGame().ConfigGetFloat( "cfgAmmo " + projectile + " DamageApplied DamageShock damage" );
	}
}

static void DZSMAmmoDump()
{
    string filepath = "$profile:dzsm-ammodump.json";
	if (FileExist(filepath))
	{
		return;
	}
	array<ref DZSMAmmoDumpEntry> list = new array<ref DZSMAmmoDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgMagazines" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgMagazines", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "Ammunition_Base") && GetGame().ConfigGetInt( "cfgMagazines " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMAmmoDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMAmmoDumpEntry>>.JsonSaveFile(filepath, list);
}

class DZSMMagDumpEntry : DZSMDumpEntry
{
	string displayName;
	string projectile;

	float weight;
	float capacity;
	float weightPerQuantityUnit;

	ref TIntArray size;
	ref TStringArray ammo;
	
	void DZSMMagDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgMagazines");

		displayName = GetGame().ConfigGetTextOut( "cfgMagazines " + classNameTemp + " displayName" );
		projectile = GetGame().ConfigGetTextOut( "cfgMagazines " + classNameTemp + " ammo" );

		weight = GetGame().ConfigGetFloat( "cfgMagazines " + classNameTemp + " weight" );
		weightPerQuantityUnit = GetGame().ConfigGetFloat( "cfgMagazines " + classNameTemp + " weightPerQuantityUnit" );
		capacity = GetGame().ConfigGetFloat( "cfgMagazines " + classNameTemp + " count" );
		
		size = new TIntArray;
		GetGame().ConfigGetIntArray( "cfgMagazines " + classNameTemp + " itemSize", size);
		ammo = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgMagazines " + classNameTemp + " ammoItems", ammo);
	}

	void ~DZSMMagDumpEntry()
	{
		delete size;
		delete ammo;
	}
}

static void DZSMMagDump()
{
    string filepath = "$profile:dzsm-magdump.json";
	if (FileExist(filepath))
	{
		return;
	}
	array<ref DZSMMagDumpEntry> list = new array<ref DZSMMagDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgMagazines" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgMagazines", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "Magazine_Base") && GetGame().ConfigGetInt( "cfgMagazines " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMMagDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMMagDumpEntry>>.JsonSaveFile(filepath, list);
}

class DZSMWeaponModeDumpEntry : Managed
{
	string nameTemp;
	float rpmTemp;
	float dispersionTemp;
	float roundsTemp;

	void DZSMWeaponModeDumpEntry(string nameTemp, float rpmTemp, float dispersionTemp, float roundsTemp)
	{
		this.nameTemp = nameTemp;
		this.rpmTemp = rpmTemp;
		this.dispersionTemp = dispersionTemp;
		if (roundsTemp)
		{
			this.roundsTemp = roundsTemp;
		}
		else
		{
			this.roundsTemp = 1;
		}
	}
}

class DZSMWeaponDumpEntry : DZSMBaseDumpEntry
{
	static ref array< string > m_ItemsThatCrash =
	{
		"itemoptics",
		"quickiebow",
		"m203",
		"gp25",
		"gp25_standalone",
		"gp25_base",
		"m203_base",
		"m203_standalone",
		"archery_base"
	};
	
	float noise;
	float magazineSwitchTime;
	float initSpeedMultiplier;

    float opticsDistanceZoomMin;
	float opticsDistanceZoomMax;
	ref TFloatArray opticsDiscreteDistance;

	float recoilMouseOffsetRangeMin;
	float recoilMouseOffsetRangeMax;
	float recoilMouseOffsetDistance;
	float recoilMouseOffsetRelativeTime;

	float recoilCamOffsetDistance;
	float recoilCamOffsetRelativeTime;
	
	ref TFloatArray recoilModifier;
	ref TFloatArray swayModifier;

	int chamberSize;
	int barrels;
	
    string color;

	ref TStringArray ammo;
	ref TStringArray mags;
	ref TStringArray attachments;

	ref array<ref DZSMWeaponModeDumpEntry> modes;
	
	void DZSMWeaponDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgWeapons");

		noise = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " NoiseShoot strength" );
		magazineSwitchTime = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " magazineSwitchTime" );
		initSpeedMultiplier = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " initSpeedMultiplier" );
		
		ammo = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgWeapons " + classNameTemp + " chamberableFrom", ammo);
		mags = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgWeapons " + classNameTemp + " magazines", mags);
		attachments = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgWeapons " + classNameTemp + " attachments", attachments);
		
		chamberSize = GetGame().ConfigGetInt( "cfgWeapons " + classNameTemp + " chamberSize" );
		TStringArray muzzles = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgWeapons " + classNameTemp + " muzzles", muzzles);
		barrels = muzzles.Count();
		delete muzzles;
		
        color = GetGame().ConfigGetTextOut( "cfgWeapons " + classNameTemp + " color" );
		
		modes = new array<ref DZSMWeaponModeDumpEntry>;
		
		TStringArray modesList = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgWeapons " + classNameTemp + " modes", modesList);
		for ( int i = 0; i < modesList.Count(); i++ )
		{
			float reloadTime = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " " + modesList[i] + " reloadTime" );
			if (reloadTime)
			{
				float rpmTemp = 60.0 / reloadTime;
			}
			float dispersionTemp = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " " + modesList[i] + " dispersionTemp" );
			float roundsTemp = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " " + modesList[i] + " burst" );
			modes.Insert(new DZSMWeaponModeDumpEntry(modesList[i], rpmTemp, dispersionTemp, roundsTemp));
		}

		recoilModifier = new TFloatArray;
		GetGame().ConfigGetFloatArray( "cfgWeapons " + classNameTemp + " recoilModifier", recoilModifier);
		swayModifier = new TFloatArray;
		GetGame().ConfigGetFloatArray( "cfgWeapons " + classNameTemp + " swayModifier", swayModifier);

        if (GetGame().ConfigIsExisting( "cfgWeapons " + classNameTemp + " OpticsInfo distanceZoomMin" ))
		{
			opticsDistanceZoomMin = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " OpticsInfo distanceZoomMin" );
			opticsDistanceZoomMax = GetGame().ConfigGetFloat( "cfgWeapons " + classNameTemp + " OpticsInfo distanceZoomMax" );
			opticsDiscreteDistance = new TFloatArray;
			GetGame().ConfigGetFloatArray( "cfgWeapons " + classNameTemp + " OpticsInfo discreteDistance", opticsDiscreteDistance );
		}

		if (!CheckItemCrash(classNameTemp))
		{
			Weapon_Base ent;
			if ( !Class.CastTo( ent, GetGame().CreateObjectEx( classNameTemp, "0 0 0", ECE_CREATEPHYSICS ) ) )
				return;
			
			RecoilBase recoil = ent.SpawnRecoilObject();

			recoilMouseOffsetRangeMin = recoil.m_MouseOffsetRangeMin;
			recoilMouseOffsetRangeMax = recoil.m_MouseOffsetRangeMax;
			recoilMouseOffsetDistance = recoil.m_MouseOffsetDistance;
			recoilMouseOffsetRelativeTime = recoil.m_MouseOffsetRelativeTime;
		
			recoilCamOffsetDistance = recoil.m_CamOffsetDistance;
			recoilCamOffsetRelativeTime = recoil.m_CamOffsetRelativeTime;

			GetGame().ObjectDelete( ent );
		}
	}

	void ~DZSMWeaponDumpEntry()
	{
		delete ammo;
		delete mags;
		delete attachments;
		delete modes;
		delete recoilModifier;
		if (opticsDiscreteDistance)
		{
			delete opticsDiscreteDistance;
		}
		if (swayModifier)
		{
			delete swayModifier;
		}
		if (recoilModifier)
		{
			delete recoilModifier;
		}
	}

	private bool CheckItemCrash( string nameTemp )
	{
		for (int i = 0; i < m_ItemsThatCrash.Count(); i++)
		{
			if ( m_ItemsThatCrash[i] == nameTemp )
			{
				return true;
			}
		}
		return false;
	}
}

static void DZSMWeaponDump()
{
    string filepath = "$profile:dzsm-weapondump.json";
	if (FileExist(filepath))
	{
		return;
	}
	array<ref DZSMWeaponDumpEntry> list = new array<ref DZSMWeaponDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgWeapons" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgWeapons", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "Weapon_Base") && GetGame().ConfigGetInt( "cfgWeapons " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMWeaponDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMWeaponDumpEntry>>.JsonSaveFile(filepath, list);
}

class DZSMClothingDumpEntry : DZSMBaseDumpEntry
{
	float heatIsolation;
	float visibilityModifier;
	float quickBarBonus;
	float durability;
	
	float armorProjectileHP;
	float armorProjectileBlood;
	float armorProjectileShock;
	
	float armorMeleeHP;
	float armorMeleeBlood;
	float armorMeleeShock;
	
	float armorFragHP;
	float armorFragBlood;
	float armorFragShock;
	
	float armorInfectedHP;
	float armorInfectedBlood;
	float armorInfectedShock;

	ref TIntArray cargoSize;
	
	ref TStringArray attachments;
	
	void DZSMClothingDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgVehicles");

		heatIsolation = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " heatIsolation" );
		visibilityModifier = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " visibilityModifier" );
		quickBarBonus = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " quickBarBonus" );
		durability = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " durability" );
	
		armorProjectileHP = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Projectile Health damage" );
		armorProjectileBlood = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Projectile Blood damage" );
		armorProjectileShock = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Projectile Shock damage" );
	
		armorMeleeHP = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Melee Health damage" );
		armorMeleeBlood = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Melee Blood damage" );
		armorMeleeShock = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Melee Shock damage" );
	
		armorFragHP = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor FragGrenade Health damage" );
		armorFragBlood = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor FragGrenade Blood damage" );
		armorFragShock = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor FragGrenade Shock damage" );
	
		armorInfectedHP = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Infected Health damage" );
		armorInfectedBlood = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Infected Blood damage" );
		armorInfectedShock = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " DamageSystem GlobalArmor Infected Shock damage" );

		cargoSize = new TIntArray;
		GetGame().ConfigGetIntArray( "cfgVehicles " + classNameTemp + " itemscargoSize", cargoSize);
	
		attachments = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgVehicles " + classNameTemp + " attachments", attachments);
	}

	void ~DZSMClothingDumpEntry()
	{
		delete cargoSize;
		delete attachments;
	}
}

static void DZSMClothingDump()
{
	string filepath = "$profile:dzsm-clothingdump.json";
	if (FileExist(filepath))
	{
		return;
	}

	array<ref DZSMClothingDumpEntry> list = new array<ref DZSMClothingDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgVehicles" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgVehicles", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "Clothing") && GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMClothingDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMClothingDumpEntry>>.JsonSaveFile(filepath, list);
}

class DZSMNutritionDumpEntry : Managed
{
	float fullnessIndex = 1;
	float energy = -10;
	float water = 1;
	float nutritionalIndex = 55;
	float toxicity = 0.3;
	float digestibility = 2;
	float agents = 16;
}

class DZSMMedicineDumpEntry : Managed
{
	float prevention = 0.75;
	float treatment = 0.5;
	float diseaseExit = 0;
}

class DZSMItemDumpEntry : DZSMBaseDumpEntry
{
	bool isMeleeWeapon;

	int repairKitType;
	
	ref DZSMNutritionDumpEntry nutrition;
	ref DZSMMedicineDumpEntry medicine;

	ref TIntArray cargoSize;
	
	ref TStringArray attachments;

	ref TFloatArray recoilModifier;
	ref TFloatArray swayModifier;
	float noiseShootModifier;
	float dispersionModifier;

	float opticsDistanceZoomMin;
	float opticsDistanceZoomMax;
	ref TFloatArray opticsDiscreteDistance;
	
	void DZSMItemDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgVehicles");

		isMeleeWeapon = GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " isMeleeWeapon" ) == 1;
		repairKitType = GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " repairKitType" );

		cargoSize = new TIntArray;
		GetGame().ConfigGetIntArray( "cfgVehicles " + classNameTemp + " itemscargoSize", cargoSize);
	
		attachments = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgVehicles " + classNameTemp + " attachments", attachments);

		recoilModifier = new TFloatArray;
		GetGame().ConfigGetFloatArray( "cfgVehicles " + classNameTemp + " recoilModifier", recoilModifier);
		swayModifier = new TFloatArray;
		GetGame().ConfigGetFloatArray( "cfgVehicles " + classNameTemp + " swayModifier", swayModifier);
		noiseShootModifier = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " noiseShootModifier");
		dispersionModifier = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " dispersionModifier");
		
		if (GetGame().ConfigIsExisting( "cfgVehicles " + classNameTemp + " OpticsInfo distanceZoomMin" ))
		{
			opticsDistanceZoomMin = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " OpticsInfo distanceZoomMin" );
			opticsDistanceZoomMax = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " OpticsInfo distanceZoomMax" );
			opticsDiscreteDistance = new TFloatArray;
			GetGame().ConfigGetFloatArray( "cfgVehicles " + classNameTemp + " OpticsInfo discreteDistance", opticsDiscreteDistance );
		}

		if (GetGame().ConfigIsExisting("cfgVehicles " + classNameTemp + " Nutrition fullnessIndex"))
		{
			nutrition = new DZSMNutritionDumpEntry;
			nutrition.fullnessIndex = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition fullnessIndex" );
			nutrition.energy = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition energy" );
			nutrition.water = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition water" );
			nutrition.nutritionalIndex = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition nutritionalIndex" );
			nutrition.toxicity = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition toxicity" );
			nutrition.digestibility = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition digestibility" );
			nutrition.agents = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Nutrition agents" );
		}

		if (GetGame().ConfigIsExisting("cfgVehicles " + classNameTemp + " Medicine prevention"))
		{
			medicine = new DZSMMedicineDumpEntry;
			medicine.prevention = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Medicine prevention" );
			medicine.treatment = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Medicine treatment" );
			medicine.diseaseExit = GetGame().ConfigGetFloat( "cfgVehicles " + classNameTemp + " Medicine diseaseExit" );
		}
	}

	void ~DZSMItemDumpEntry()
	{
		if (nutrition)
		{
			delete nutrition;
		}
		if (medicine)
		{
			delete medicine;
		}
		if (cargoSize)
		{
			delete cargoSize;
		}
		if (attachments)
		{
			delete attachments;
		}
		if (recoilModifier)
		{
			delete recoilModifier;
		}
		if (swayModifier)
		{
			delete swayModifier;
		}
		if (opticsDiscreteDistance)
		{
			delete opticsDiscreteDistance;
		}
	}
}

static void DZSMItemDump()
{
	string filepath = "$profile:dzsm-itemdump.json";
	if (FileExist(filepath))
	{
		return;
	}

	array<ref DZSMItemDumpEntry> list = new array<ref DZSMItemDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgVehicles" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgVehicles", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "Inventory_Base") && !GetGame().IsKindOf(classNameTemp, "Clothing") && GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMItemDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMItemDumpEntry>>.JsonSaveFile(filepath, list);
}

class DZSMContainerDumpEntry : DZSMBaseDumpEntry
{

	int canBeDigged;
	int heavyItem;
	ref TIntArray cargoSize;
	
	ref TStringArray attachments;
	
	void DZSMContainerDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgVehicles");

		canBeDigged = GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " canBeDigged" );
		heavyItem = GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " heavyItem" );

		cargoSize = new TIntArray;
		if (GetGame().ConfigIsExisting( "cfgVehicles " + classNameTemp + " Cargo itemscargoSize" ))
		{
			GetGame().ConfigGetIntArray( "cfgVehicles " + classNameTemp + " Cargo itemscargoSize", cargoSize);
		}
		else
		{
			GetGame().ConfigGetIntArray( "cfgVehicles " + classNameTemp + " itemscargoSize", cargoSize);
		}
	
		attachments = new TStringArray;
		GetGame().ConfigGetTextArray( "cfgVehicles " + classNameTemp + " attachments", attachments);
	}

	void ~DZSMContainerDumpEntry()
	{
		if (cargoSize)
		{
			delete cargoSize;
		}
		if (attachments)
		{
			delete attachments;
		}
	}
}

static void DZSMContainerDump()
{
	string filepath = "$profile:dzsm-containerdump.json";
	if (FileExist(filepath))
	{
		return;
	}

	array<ref DZSMContainerDumpEntry> list = new array<ref DZSMContainerDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgVehicles" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgVehicles", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "Container_Base") && GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMContainerDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMContainerDumpEntry>>.JsonSaveFile(filepath, list);
}

class DZSMZombieDumpEntry : DZSMDumpEntry
{
	
	void DZSMZombieDumpEntry(string classNameTemp)
	{
		Init(classNameTemp, "cfgVehicles");

	}

	void ~DZSMZombieDumpEntry()
	{
		
	}
}

static void DZSMZombieDump()
{
	string filepath = "$profile:dzsm-zombiedump.json";
	if (FileExist(filepath))
	{
		return;
	}

	array<ref DZSMZombieDumpEntry> list = new array<ref DZSMZombieDumpEntry>;
    int nClasses = GetGame().ConfigGetChildrenCount( "cfgVehicles" );
    for ( int nClass = 0; nClass < nClasses; ++nClass )
	{
    	string classNameTemp;
    	GetGame().ConfigGetChildName( "cfgVehicles", nClass, classNameTemp );
		if (GetGame().IsKindOf(classNameTemp, "ZombieBase") && GetGame().ConfigGetInt( "cfgVehicles " + classNameTemp + " scope" ) == 2) {
			list.Insert(new DZSMZombieDumpEntry(classNameTemp));
		}
    }
	JsonFileLoader<array<ref DZSMZombieDumpEntry>>.JsonSaveFile(filepath, list);
}

class ServerManagerCallback: RestCallback
{	
	override void OnSuccess(string data, int dataSize)
	{
		#ifdef DZSM_DEBUG
		Print("DZSM ~ OnSuccess Data: " + data);
		#endif
	}
	
	override void OnError(int errorCode)
	{
		#ifdef DZSM_DEBUG
		Print("DZSM ~ OnError: " + errorCode);
		#endif
	}
	
	override void OnTimeout()
	{
		#ifdef DZSM_DEBUG
		Print("DZSM ~ OnTimeout");
		#endif
	}
};

class ServerManagerEntry
{
	string entryType;
	string type;
	string category;
	string nameTemp;
	int id;
	string position;	
	string speed;
	float damage;
}


class ServerManagerEntryContainer
{
	ref array<ref ServerManagerEntry> players = new array<ref ServerManagerEntry>;
	ref array<ref ServerManagerEntry> vehicles = new array<ref ServerManagerEntry>;

	void ServerManagerEntryContainer()
	{
	}

	void ~ServerManagerEntryContainer()
	{
		int i = 0;
		for (i = 0; i < players.Count(); i++)
		{
			delete players.Get(i);
		}
		delete players;

		for (i = 0; i < vehicles.Count(); i++)
		{
			delete vehicles.Get(i);
		}
		delete vehicles;
	}

}

class DayZServerManagerWatcher
{
    private ref Timer m_Timer;
	private ref Timer m_InitTimer;

	private ref JsonSerializer m_jsonSerializer = new JsonSerializer;
	
	private RestApi m_RestApi;
    private RestContext m_RestContext;

    void DayZServerManagerWatcher()
    {
		#ifdef DZSM_DEBUG
		Print("DZSM ~ DayZServerManagerWatcher()");
		#endif

        m_InitTimer = new Timer(CALL_CATEGORY_GAMEPLAY);
		m_InitTimer.Run(2.0 * 60.0, this, "init", null, false);
    }

	void init()
	{
		#ifdef DZSM_DEBUG
		Print("DZSM ~ DayZServerManagerWatcher() - INIT");
		#endif

		m_RestApi = CreateRestApi();
        m_RestContext = m_RestApi.GetRestContext(GetDZSMApiOptions().host);
		m_RestContext.SetHeader("application/json");
        m_RestApi.EnableDebug(false);
		
		StartLoop();
		#ifdef DZSM_DEBUG
		Print("DZSM ~ DayZServerManagerWatcher() - INIT DONE");
		#endif

		if (GetDZSMApiOptions().dataDump)
		{
			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - DATA DUMP");
			#endif

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - AMMO DUMP");
			#endif
			DZSMAmmoDump();

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - MAG DUMP");
			#endif
			DZSMMagDump();

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - WEAPON DUMP");
			#endif
			DZSMWeaponDump();

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - CLOTHING DUMP");
			#endif
			DZSMClothingDump();

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - ITEM DUMP");
			#endif
			DZSMItemDump();
			
			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - CONTAINER DUMP");
			#endif
			DZSMContainerDump();

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - ZOMBIE DUMP");
			#endif
			DZSMZombieDump();

			#ifdef DZSM_DEBUG
			Print("DZSM ~ DayZServerManagerWatcher() - DATA DUMP DONE");
			#endif
		}
	}

    float GetInterval()
	{
		return GetDZSMApiOptions().reportInterval;
	}

    void StartLoop()
	{
		if (!m_Timer)
		{
			m_Timer = new Timer(CALL_CATEGORY_GAMEPLAY);
		}
		
		m_Timer.Run(GetInterval(), this, "Tick", null, true);
	}
	
	void StopLoop()
	{
		if (m_Timer)
		{
			m_Timer.Stop();
		}
	}

	void Tick()
	{
		#ifdef DZSM_DEBUG
		Print("DZSM ~ TICK");
		#endif
		int i;
		
		ref ServerManagerEntryContainer container = new ServerManagerEntryContainer;
		
		array<EntityAI> allVehicles;
		DayZServerManagerContainer.GetVehicles(allVehicles);
		if (allVehicles)
		{
			for (i = 0; i < allVehicles.Count(); i++)
			{
				EntityAI itrCar = allVehicles.Get(i);
				
				ref ServerManagerEntry entry = new ServerManagerEntry();
				
				entry.entryType = "VEHICLE";
				entry.nameTemp = itrCar.GetName();
				entry.damage = itrCar.GetDamage();
				entry.type = itrCar.GetType();
				entry.id = itrCar.GetID();
				entry.speed = itrCar.GetSpeed().ToString(false);
				entry.position = itrCar.GetPosition().ToString(false);

			#ifdef DZSM_DEBUG
			//Print("DZSM ~ [Tick] ~ entry.type="+entry.type+" --- entry.type.Contains(RFFS)="+entry.type.Contains("RFFS")+" --- entry.type.Contains(RFWC)="+entry.type.Contains("RFWC"));
			#endif

				if ((itrCar.IsKindOf("ExpansionHelicopterScript")) || (entry.type.Contains("RFFS")))
				{
					entry.category = "AIR";
				}
				else if ((itrCar.IsKindOf("ExpansionBoatScript")) || (entry.type.Contains("RFWC")))
				{
					entry.category = "SEA";
				}
				else
				{
					entry.category = "GROUND";
				}
				
				
				
				container.vehicles.Insert(entry);
			}
		}
		
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		if (players)
		{
			for (i = 0; i < players.Count(); i++)
			{
				Man player = players.Get(i);
				
				ref ServerManagerEntry playerEntry = new ServerManagerEntry();
				
				playerEntry.entryType = "PLAYER";
				playerEntry.category = "MAN";

				playerEntry.nameTemp = player.GetIdentity().GetName();
				// player.GetDisplayName();
				playerEntry.damage = player.GetDamage();
				playerEntry.type = player.GetType();
				playerEntry.id = player.GetID();
				playerEntry.speed = player.GetSpeed().ToString(false);
				playerEntry.position = player.GetPosition().ToString(false);

				container.players.Insert(playerEntry);
			}
		}

		DZSMApiOptions apiOptions = GetDZSMApiOptions();
		if (apiOptions.useApiForReport)
		{
			#ifdef DZSM_DEBUG
			Print("DZSM ~ API TICK");
			#endif

			// RestContext restContext = GetRestApi().GetRestContext(apiOptions.host);
			// restContext.SetHeader("application/json");
			// restContext.POST_now("/ingamereport?key=" + apiOptions.key, JsonFileLoader<ref ServerManagerEntryContainer>.JsonMakeData(container));
			m_RestContext.POST(new ServerManagerCallback(), string.Format("/ingamereport?key=%1", apiOptions.key), JsonFileLoader<ref ServerManagerEntryContainer>.JsonMakeData(container));
		}
		else
		{
			JsonFileLoader<ref ServerManagerEntryContainer>.JsonSaveFile("$profile:DZSM-TICK.json", container);
		}

		#ifdef DZSM_DEBUG
		Print("DZSM ~ Cleanup");
		#endif
		delete container;
		#ifdef DZSM_DEBUG
		Print("DZSM ~ Cleanup Done");
		#endif
	}

}

modded class MissionServer
{
    private ref DayZServerManagerWatcher m_dayZServerManagerWatcher;

    void MissionServer()
    {
		#ifdef DZSM_DEBUG
		Print("DZSM ~ MissionServer");
		#endif
    }

	override void OnInit()
	{
		super.OnInit();
		
		#ifdef DZSM_DEBUG
		Print("DZSM ~ MissionServer.OnInit");
		#endif
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();
		#ifdef DZSM_DEBUG
		Print("DZSM ~ MissionServer.OnMissionStart");
		#endif

		if (!GetGame().IsClient())
		{
        	m_dayZServerManagerWatcher = new DayZServerManagerWatcher();
		}
	}
};