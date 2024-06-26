#include "stdafx.h"
#include "build.h"
#include "xrPhase_MergeLM_Rect.h"
#include "../xrlc_light/xrdeflector.h"

static	BYTE	surface			[c_LMAP_size*c_LMAP_size];
const	u32		alpha_ref		= 254-BORDER;

// Initialization
void _InitSurface	()
{
	FillMemory		(surface,c_LMAP_size*c_LMAP_size,0);
}

// Rendering of rect
void _rect_register	(L_rect &R, lm_layer* D, BOOL bRotate)
{
	u8*		lm	= &*(D->marker.begin());
	u32		s_x	= D->width+2*BORDER;
	u32		s_y = D->height+2*BORDER;
	
	if (!bRotate) {
		// Normal (and fastest way)
		for (u32 y=0; y<s_y; y++)
		{
			BYTE*	P = surface+(y+R.a.y)*c_LMAP_size+R.a.x;	// destination scan-line
			u8*		S = lm + y*s_x;
			for (u32 x=0; x<s_x; x++,P++,S++) 
				if (*S >= alpha_ref)			*P	= 255;
		}
	} else {
		// Rotated :(
		for (u32 y=0; y<s_x; y++)
		{
			BYTE*	P = surface+(y+R.a.y)*c_LMAP_size+R.a.x;	// destination scan-line
			for (u32 x=0; x<s_y; x++,P++)
				if (lm[x*s_x+y] >= alpha_ref)	*P	= 255;
		}
	}
}

// Test of per-pixel intersection (surface test)
bool Place_Perpixel	(L_rect& R, lm_layer* D, BOOL bRotate)
{
	u8*	lm			= &*(D->marker.begin());
	u32	s_x			= D->width	+2*BORDER;
	u32	s_y			= D->height +2*BORDER;
	
	if (!bRotate) {
		// Normal (and fastest way)
		for (u32 y=0; y<s_y; y++)
		{
			BYTE*	P = surface+(y+R.a.y)*c_LMAP_size+R.a.x;	// destination scan-line
			u8*		S = lm + y*s_x;
			for (u32 x=0; x<s_x; x++,P++,S++) 
				if ((*P)&&(*S>=alpha_ref))			return false;	// overlap
		}
	} else {
		// Rotated :(
		for (u32 y=0; y<s_x; y++)
		{
			BYTE*	P = surface+(y+R.a.y)*c_LMAP_size+R.a.x;	// destination scan-line
			for (u32 x=0; x<s_y; x++,P++)
				if ((*P)&&(lm[x*s_x+y]>=alpha_ref))	return false;	// overlap
		}
	}
	
	// It's OK to place it
	return true;
}

// Check for intersection
bool _rect_place(L_rect &r, lm_layer* D, bool& rotated)
{
	L_rect R;

	// Normal
	{
		u32 x_max = c_LMAP_size-r.b.x; 
		u32 y_max = c_LMAP_size-r.b.y; 
		for (u32 _Y=0; _Y<y_max; _Y++)
		{
			for (u32 _X=0; _X<x_max; _X++)
			{
				if (surface[_Y*c_LMAP_size+_X]) continue;
				R.init(_X,_Y,_X+r.b.x,_Y+r.b.y);
				if (Place_Perpixel	(R,D,FALSE)) {
					r.set			(R);
					rotated			= false;
					return true;
				}
			}
		}
	}
	
	// Rotated
	{
		u32 x_max = c_LMAP_size-r.b.y; 
		u32 y_max = c_LMAP_size-r.b.x; 
		for (u32 _Y=0; _Y<y_max; _Y++)
		{
			for (u32 _X=0; _X<x_max; _X++)
			{
				if (surface[_Y*c_LMAP_size+_X]) continue;
				
				R.init(_X,_Y,_X+r.b.y,_Y+r.b.x);
				if (Place_Perpixel	(R,D,TRUE)) {
					r.set			(R);
					rotated			= true;
					return true;
				}
			}
		}
	}
	
	return false;
};

bool rectPlaceY(u32 y, L_rect& r, lm_layer* D, bool& rotated)
{
    L_rect R;

    // Normal
    u32 y_max = c_LMAP_size - r.b.y;
    if (y < y_max) {
        u32 x_max = c_LMAP_size - r.b.x;
        for (u32 _X = 0; _X < x_max; _X++) {
            if (surface[y * c_LMAP_size + _X])
                continue;
            R.init(_X, y, _X + r.b.x, y + r.b.y);
            if (Place_Perpixel(R, D, FALSE)) {
                r.set(R);
                rotated = false;
                return true;
            }
        }
    }

    // Rotated
    y_max = c_LMAP_size - r.b.x;
    if (y < y_max) {
        u32 x_max = c_LMAP_size - r.b.y;
        for (u32 _X = 0; _X < x_max; _X++) {
            if (surface[y * c_LMAP_size + _X])
                continue;

            R.init(_X, y, _X + r.b.y, y + r.b.x);
            if (Place_Perpixel(R, D, TRUE)) {
                r.set(R);
                rotated = true;
                return true;
            }
        }
    }

    return false;
}

void fillSpace(BYTE* begin, BYTE* end)
{
    for (auto it = begin; it != end; ++it)
        *it = 255;
}

bool checkFreeRect(BYTE* p, u32 size)
{
    for (u32 y = 0; y != size; y++) {
        for (u32 x = 0; x != size; x++)
            if (p[x] != 0)
                return false;
        p += c_LMAP_size;
    }
    return true;
}

bool checkFreeSpace(u32 minCell)
{
    for (auto bLine = std::begin(surface); bLine != std::end(surface) - minCell * c_LMAP_size; bLine += c_LMAP_size) {
        auto eLine = bLine + c_LMAP_size - minCell;
        for (auto bFree = bLine; bFree != eLine;) {
            bFree = std::find(bLine, eLine, 0);
            auto eFree = std::find(bFree, eLine, 255);
            if (eFree - bFree < minCell)
                fillSpace(bFree, eFree);
            else {
                for (auto it = bFree; it != eFree; ++it)
                    if (checkFreeRect(it, minCell))
                        return true;
                    else
                        fillSpace(it, it + 1);
            }
            bFree = eFree;
        }
    }
    return false;
}