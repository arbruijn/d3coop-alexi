// 
// copies of Dallas functions; taken from "dallasfuncs.cpp"

#include "dallas.h"
#include "../osiris/osiris_import.h"
#include <string.h>
//#include <stdarg.h>
//#include <float.h>
#include <stdlib.h>
#include "../osiris/osiris_vector.h"


/*
$$ACTION
Weather
Create lighting between [o:Object1] and [o:Object2]; with [f:Lifetime=1.0], [f:Thickness=1.0], [i:NumTiles=1], [u:Texture], [f:SlideTime=0], [i:TimesDrawn=1], and color=[i:Red=255],[i:Green=255],[i:Blue=255], AutoTile=[b:AutoTile=false]
aLightningCreate
Create lighting between two objects
  Creates a lighting effect between two specified obejcts

Parameters:
  Object1, Object2:  Where the lighting is created
  Lifetime: How long the lighting lasts
  Thickness: How thick the lightning is
  NumTiles: How many times to tile the texture within the bolt
  Texture: The texture to be used for the lighting
  SlideTime: The time it will take to slide the entire bolt once
  TimesDrawn: The number of times to draw the bolt (saturation)
  Red, Green, Blue: The color of the lighting (0-255)
  AutoTile - For automatic UV tiling based on the length of the bolt
$$END
*/
void Dallas_LightningCreate (int objhandle1, int objhandle2, float lifetime, float thickness, int numtiles, int texture_id, float slidetime, int timesdrawn, int red, int green, int blue, bool autotile)
{
	msafe_struct mstruct;
	int type;

	Obj_Value(objhandle1, VF_GET, OBJV_I_TYPE, &type, 0);
	if(type == OBJ_NONE)
		return;

	Obj_Value(objhandle2, VF_GET, OBJV_I_TYPE, &type, 0);
	if(type == OBJ_NONE)
		return;

	Obj_Value(objhandle1, VF_GET, OBJV_I_ROOMNUM, &mstruct.roomnum, 0);
	Obj_Value(objhandle1, VF_GET, OBJV_V_POS, &mstruct.pos, 0);
	Obj_Value(objhandle2, VF_GET, OBJV_V_POS, &mstruct.pos2, 0);

	mstruct.objhandle=objhandle1;
	mstruct.ithandle=objhandle2;
	mstruct.lifetime = lifetime;
	mstruct.size = thickness;

	mstruct.interval=slidetime;
	mstruct.count=timesdrawn;
	mstruct.index=numtiles;
	mstruct.texnum=texture_id;
	mstruct.color=((red>>3)<<10)|((green>>3)<<5)|(blue>>3);

	if (autotile)
		mstruct.state=1;
	else
		mstruct.state=0;

	mstruct.flags=0;
	
	MSafe_CallFunction(MSAFE_WEATHER_LIGHTNING_BOLT,&mstruct);
}

