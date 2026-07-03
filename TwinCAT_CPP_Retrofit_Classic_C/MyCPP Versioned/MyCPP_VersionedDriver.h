///////////////////////////////////////////////////////////////////////////////
// MyCPP_VersionedDriver.h

#ifndef __MYCPP_VERSIONEDDRIVER_H__
#define __MYCPP_VERSIONEDDRIVER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TcBase.h"

#define MYCPP_VERSIONEDDRV_NAME        "MYCPP_VERSIONED"
#define MYCPP_VERSIONEDDRV_Major       1
#define MYCPP_VERSIONEDDRV_Minor       0

#define DEVICE_CLASS CMyCPP_VersionedDriver

#include "ObjDriver.h"

class CMyCPP_VersionedDriver : public CObjDriver
{
public:
	virtual IOSTATUS	OnLoad();
	virtual VOID		OnUnLoad();

	//////////////////////////////////////////////////////
	// VxD-Services exported by this driver
	static unsigned long	_cdecl MYCPP_VERSIONEDDRV_GetVersion();
	//////////////////////////////////////////////////////
	
};

Begin_VxD_Service_Table(MYCPP_VERSIONEDDRV)
	VxD_Service( MYCPP_VERSIONEDDRV_GetVersion )
End_VxD_Service_Table


#endif // ifndef __MYCPP_VERSIONEDDRIVER_H__