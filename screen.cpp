// by Alexander Ilyin
// original belongs at Descent 3 Co-op Mod project

#include "screen.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "types.h"


#ifdef WIN32
void RecreateGammaRamp (HDC hdc)
{
	ushort gamma_ramp[3][256];
	BOOL result;
	int j;

	for (j = 0; j < 256; j++)
	{
		short val = j << 8;

		gamma_ramp[0][j] = val;
		gamma_ramp[1][j] = val;
		gamma_ramp[2][j] = val;
	}

	result = SetDeviceGammaRamp (hdc, gamma_ramp);
}
#endif

// remove any possible garbage from the screen :  wrong resolution, black covering rectangle, wrong gamma
void Screen_Restore ()
{
#ifdef WIN32
	int i_r;
	HDC h_dc;

// restore screen; now, after a D3 exception handlers were removed, has become necessary
	ChangeDisplaySettings (0, 0);
// remove the layer bug
	SetWindowPos (GetActiveWindow (), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER);
	h_dc = GetDC (0);
	if (h_dc)
	{
		RecreateGammaRamp (h_dc);
		i_r = ReleaseDC (0, h_dc);
	}
#endif
}

