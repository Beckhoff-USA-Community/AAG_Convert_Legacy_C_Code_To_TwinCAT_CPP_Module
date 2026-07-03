///////////////////////////////////////////////////////////////////////////////
// MyCPP_VersionedDriver.cpp
#include "TcPch.h"
#pragma hdrstop

#include "MyCPP_VersionedDriver.h"
#include "MyCPP_VersionedClassFactory.h"

DECLARE_GENERIC_DEVICE(MYCPP_VERSIONEDDRV)

IOSTATUS CMyCPP_VersionedDriver::OnLoad( )
{
	TRACE(_T("CObjClassFactory::OnLoad()\n") );
	m_pObjClassFactory = new CMyCPP_VersionedClassFactory();

	return IOSTATUS_SUCCESS;
}

VOID CMyCPP_VersionedDriver::OnUnLoad( )
{
	delete m_pObjClassFactory;
}

unsigned long _cdecl CMyCPP_VersionedDriver::MYCPP_VERSIONEDDRV_GetVersion( )
{
	return( (MYCPP_VERSIONEDDRV_Major << 8) | MYCPP_VERSIONEDDRV_Minor );
}

