//
//  platforms.hpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#ifndef platforms_hpp
#define platforms_hpp

#include "zgolf.hpp"


struct PlatFillInfo
{
	PlatFillInfo ()
	{
		type = FillType::colorDev;
		arg = ColorDevInfo(Color(207, 163, 0), 14, 1);
	}
	
	PlatFillInfo (string fname) : arg(fname) { type = FillType::imagePixs; }
	
	PlatFillInfo (const Color& c, int dev = 0, int reps = 1)
		: arg(ColorDevInfo(c, dev, reps))
	{
		type = FillType::colorDev;
	}
	
	enum class FillType { imagePixs, colorDev };
	
	struct ColorDevInfo
	{
		ColorDevInfo (const Color& col, int d = 0, int reps = 1)
			: c(col)
			, dev(d)
			, blurRepetitions(reps)
		{ }
		
		Color c;
		int dev;
		int blurRepetitions;
	};
	
	FillType type;
	std::variant<string, ColorDevInfo> arg;
};



struct Platform
{
	vector<GroundSegment>	segs;
	TransformableVxArray 	va {LineStrip};
	// If dynamic elements added, Platforms may be drawn individually
	// again instead of being amalgamated into one static sprite
	//	Sprite 					s;
	//	Texture 				tx;
	Color					fillColor {Color(207, 163, 0)};
	PlatFillInfo			fillInfo;
};



struct EditorPlatform
{
	EditorPlatform () { verts.reserve(2000); }
	
	pair<Vert*, Vert*> recomputeSpline ();
	
	pair<Vert*, Vert*> updateSegs () ;
	
	TransformableVxArray	splVa {LineStrip};
	vector<Vert> 			verts;
	vector<Vert>			saveVerts;
	vector<vecf>			pts;
	//	vector<EditorGroundSeg> segs;  //in Vert
	PlatFillInfo			fillInfo;
	string					fillTboxStr = "207 163 0 255 14 1";
	bool					isComplete = false;
};

#endif /* platforms_hpp */
