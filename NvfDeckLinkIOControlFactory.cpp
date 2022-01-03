//==========================================================================;
//
// (c) Copyright China Digital Technology CO.,Ltd., 2005. All rights reserved. 
//
// This code and information is provided "as is" without warranty of any kind, 
// either expressed or implied, including but not limited to the implied 
// warranties of merchantability and/or fitness for a particular purpose.
//
//--------------------------------------------------------------------------;
//----------------------------------------------------------------
//   Birth Date:       June 24. 2013
//   Operating System: Win7
//   Author:           CDV DX Team
//----------------------------------------------------------------
//   
//   Beijing China. 
//----------------------------------------------------------------
111111122
#include "NvfDeckLinkIOControlFactory.h"
#include "NvfDeckLinkIOPlaybackControl.h"
#include "NvfDeckLinkIOCaptureControl.h"

fsfafa
extern TNvcSmartPtr<CDeckLinkReferenceClock> poReferenceClock;
extern SNvcResolutionInfo g_sInputResolution;
extern SNvcResolutionInfo g_sOutputResolution;

extern CNvcCriticalSection g_ocsFactory;

CFactoryTemplate * g_Templates;

int g_cTemplates = 0;

CNvcFactoryTemplate g_BvfaoTemplates[] =
{
	{ L"DeckLink IO Control Factory", &CLSID_NvfDeckLinkIOControlFactory, CNvfDeckLinkIOControlFactory::CreateInstance, NULL, NULL }
};


BOOL WINAPI DllMain(HINSTANCE hInst,
					ULONG ul_reason_for_call,
					LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
		NvcCOMEntryPoint(hInst, ul_reason_for_call, lpReserved,
			g_BvfaoTemplates, sizeof(g_BvfaoTemplates) / sizeof(g_BvfaoTemplates[0]));
		break;
	}

	return TRUE;
}

extern "C" int NVCAPI CheckHardwareIOCardExist()
{
	HRESULT hr = NVC_NOERROR;

	IDeckLinkIterator *pDeckLinkIterator = NULL;
	hr = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_INPROC_SERVER, IID_IDeckLinkIterator, (void**)&pDeckLinkIterator);

	if (FAILED(hr) || pDeckLinkIterator == NULL)
		return -1;

	uint32_t ui32DeckLinkCount = 0;
	IDeckLink *pDeckLink = NULL;

	while (pDeckLinkIterator->Next(&pDeckLink) == S_OK)
		ui32DeckLinkCount ++;

	if (ui32DeckLinkCount == 0)
	{
		pDeckLinkIterator->Release();
		return -1;
	}

	pDeckLinkIterator->Release();

	return 1;
}

extern "C" int NVCAPI GetPluginObjectCount()
{
	return sizeof(g_BvfaoTemplates) / sizeof(g_BvfaoTemplates[0]);
}


extern "C" GUID NVCAPI GetPluginObjectID(int nIdx)
{
	int nFactoryCount = GetPluginObjectCount();

	for (int i = 0; i < nFactoryCount; i++) {
		if(nIdx == i)
			return *g_BvfaoTemplates[i].m_pClsID;
	}

	return GUID_NULL;
}

extern "C" int NVCAPI GetPluginObjectType(const GUID gObjectID)
{

	if (gObjectID == CLSID_NvfDeckLinkIOControlFactory)
		return keNvfIOPluginObjectType_IODevice;

	return keNvfIOPluginObjectType_Unkonwn;
}
extern "C" HRESULT NVCAPI CreatePluginInstance(REFCLSID in_rsClsID,
											   IUnknown *in_pUnkOuter,
											   REFIID in_rsIID,
											   void **out_ppv)
{
	HRESULT hr = NVC_NOERROR;

	int nFactoryCount = GetPluginObjectCount();
	for (int i = 0; i < nFactoryCount; i++) {
		if (in_rsClsID == *g_BvfaoTemplates[i].m_pClsID) {
			if (g_BvfaoTemplates[i].m_pfnNew == NULL) {
				assert(false);
				return NVC_E_INVALID_PARAMETER;
			}

			// create this object
			CNvcUnknown *pUnk = NULL;
			pUnk = g_BvfaoTemplates[i].m_pfnNew(in_pUnkOuter, &hr);
			if (FAILED(hr)) {
				assert(pUnk == NULL);
				return hr;
			}

			assert(pUnk != NULL);
			hr = pUnk->NonDelegatingQueryInterface(in_rsIID, out_ppv);
			if (FAILED(hr)) {
				assert(false);
				delete pUnk;
			}

			return hr;
		}
	}

	return NVC_E_NOT_FOUND;
}

CNvfDeckLinkIOControlFactory::CNvfDeckLinkIOControlFactory
	(
	LPUNKNOWN in_poUnknown,
	HRESULT * out_pHresult
	): CNvcUnknown(L"CNvfDeckLinkIOControlFactory", in_poUnknown, out_pHresult)
{
	HRESULT hr = NVC_NOERROR;

	m_deckLinkDeviceCount = 0;
//	m_deckLinkIterator = NULL;
	m_deckLink = NULL;

	IDeckLinkIterator *deckLinkIterator = NULL;
	hr = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&deckLinkIterator);
	if (FAILED(hr))
	{
		*out_pHresult = NVC_E_IGNORE_OBJECT_CREATION;
		return ;
	}

	IDeckLink *deckLink = NULL;

	while (deckLinkIterator->Next(&deckLink) == S_OK)
	{
		m_vecDeckLink.push_back(deckLink);

		m_deckLinkDeviceCount ++;

		if (m_deckLinkDeviceCount == 1)
			m_deckLink = deckLink;
	}

	if (m_deckLinkDeviceCount == 0)
	{
		deckLinkIterator->Release();
		*out_pHresult = NVC_E_IGNORE_OBJECT_CREATION;
		return ;
	}

	deckLinkIterator->Release();
}

CNvfDeckLinkIOControlFactory::~CNvfDeckLinkIOControlFactory()
{
	poReferenceClock = NULL;

	for (int i = 0; i < m_vecDeckLink.size(); i++)
	{
		m_vecDeckLink[i]->Release();
		m_vecDeckLink[i] = NULL;

		m_vecDeckLink.clear();
	}
 
// 	if (m_deckLink != NULL)
// 	{
// 		m_deckLink->Release();
// 		m_deckLink = NULL;
// 	}
}

STDMETHODIMP CNvfDeckLinkIOControlFactory::NonDelegatingQueryInterface(REFIID  in_rIID, void ** out_ppv)
{
	if (in_rIID == IID_INvfIOControlFactory)
	{
		return NvcGetInterface(static_cast<INvfIOControlFactory*>(this), out_ppv);
	}

	return CNvcUnknown::NonDelegatingQueryInterface(in_rIID, out_ppv);
}

CNvcUnknown* WINAPI CNvfDeckLinkIOControlFactory::CreateInstance(IUnknown *in_pUnkOuter, HRESULT *io_phr)
{
	CNvcUnknown* punk = new CNvfDeckLinkIOControlFactory(in_pUnkOuter, io_phr);
	if (punk == NULL) 
	{
		*io_phr = NVC_E_OUT_OF_MEMORY;    
	}

	if(FAILED(*io_phr) && punk != NULL)
	{
		delete punk;
		punk = NULL;
	}

	return punk;
}

//////////////////////////////////////////////////////////////////////////

//INvfIOControlFactory interface implement

//////////////////////////////////////////////////////////////////////////

HRESULT __stdcall CNvfDeckLinkIOControlFactory::GetCurrentResolutionInfo
	(
	bool				in_bInput, 
	int					in_nChannels, 
	SNvcResolutionInfo* out_psResolutionInfo
	)
{
	UNREFERENCED_PARAMETER(in_nChannels);

	if (m_deckLinkDeviceCount <= 0)
		return NVC_E_IGNORE_OBJECT_CREATION;
	
	if (in_bInput)
	{
		*out_psResolutionInfo = g_sInputResolution;
	}
	else
	{
		*out_psResolutionInfo = g_sOutputResolution;
	}
	
	return NVC_NOERROR;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::IsResolutionSupport
	(
	bool                    in_bInput,
	uint32_t				in_ui32Width, 
	uint32_t				in_ui32Hight,
	ENvcFrameRate			in_eFrameRate
 	)
{
	if (m_deckLinkDeviceCount <= 0)
		return NVC_E_IGNORE_OBJECT_CREATION;

	wchar_t *deviceModleName = NULL;
	m_deckLink->GetModelName(&deviceModleName);
	
	char szCharName[NVC_MAX_FILENAME];
	NvcWideCharToChar(deviceModleName, szCharName, NVC_MAX_FILENAME);

	std::string str(szCharName);

	enum 
	{
		keDeckLinkStandType_SD = 0,			//720 * 576
		keDeckLinkStandType_HD_720,			//1280 * 720 
		keDeckLinkStandType_HD_1440,		//1440 * 1080
		keDeckLinkStandType_HD,				//1920 * 1080
		keDeckLinkStandType_2K,				//2048 * 1556
		keDeckLinkStandType_QuadHD_3840,	//3840 * 2160
		keDeckLinkStandType_4K				//4096 * 2160
	};

	int iStandType = -1; 
	if (in_ui32Width == 720)
		iStandType = keDeckLinkStandType_SD;
	else if (in_ui32Hight == 720)
		iStandType = keDeckLinkStandType_HD_720;
	else if (in_ui32Width == 1440)
		iStandType = keDeckLinkStandType_HD_1440;
	else if (in_ui32Width == 1920)
		iStandType = keDeckLinkStandType_HD;
	else if (in_ui32Width == 2048)
		iStandType = keDeckLinkStandType_2K;
	else if (in_ui32Width == 3840)
		iStandType = keDeckLinkStandType_QuadHD_3840;
	else if (in_ui32Width == 4096)
		iStandType = keDeckLinkStandType_4K;
	else
		return NVC_E_UNSUPPORTED_INPUT_WIDTH_HEIGHT;

	if (iStandType == keDeckLinkStandType_2K)
	{
		if (in_bInput)
			return NVC_E_UNSUPPORTED_INPUT_WIDTH_HEIGHT;
		else 
			return NVC_NOERROR;
	}

	if ( ((str == "DeckLink Studio 4K") || (str == "DeckLink 4K Pro") || (str == "DeckLink 4K Extreme 12G")) && iStandType == keDeckLinkStandType_QuadHD_3840)
	{
		return NVC_NOERROR;
	}

	if (iStandType == keDeckLinkStandType_QuadHD_3840 || iStandType == keDeckLinkStandType_4K)
		return NVC_E_UNSUPPORTED_INPUT_WIDTH_HEIGHT;

	if (str == "UltraStudio SDI")
	{
		if (iStandType == keDeckLinkStandType_HD)
			if (in_eFrameRate == keNvcFrameRate50 || in_eFrameRate == keNvcFrameRate60M ||in_eFrameRate == keNvcFrameRate60)
				return NVC_E_NOT_SUPPORTED;

		if (iStandType == keDeckLinkStandType_HD_720)
		{
			if (in_eFrameRate == keNvcFrameRate50 || in_eFrameRate == keNvcFrameRate60M ||in_eFrameRate == keNvcFrameRate60)
				return NVC_NOERROR;
		}
	}

	return NVC_NOERROR;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::IsBitDepthSupport(uint32_t in_ui32ComponentBitCount)
{
	if (m_deckLinkDeviceCount <= 0)
		return NVC_E_IGNORE_OBJECT_CREATION;

	if((in_ui32ComponentBitCount != 8) && (in_ui32ComponentBitCount != 10))
		return NVC_E_UNSUPPORTED_BIT_DEPTH;

	return NVC_NOERROR;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::IsAudioFormatSupport(SNvcWaveFormatInfo *in_psWaveFormatInfo)
{
    if(in_psWaveFormatInfo == NULL)
    {
        ASSERT(false);
        return NVC_E_INVALID_POINTER;
    }

    if(in_psWaveFormatInfo->ui32SamplesPerSec != 48000)
        return NVC_E_NOT_SUPPORTED;

    if(in_psWaveFormatInfo->ui32ValidBitsPerSample == 24 && in_psWaveFormatInfo->ui32BitsPerSample == 32)
        return NVC_NOERROR;
// 	else if (in_psWaveFormatInfo->ui32ValidBitsPerSample == 16 && in_psWaveFormatInfo->ui32BitsPerSample == 16)
// 		return NVC_NOERROR;

    return NVC_E_NOT_SUPPORTED;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::GetIOControlCount(uint32_t*	out_ui32IOControlCount)
{
	HRESULT hr = NVC_NOERROR;

	if(out_ui32IOControlCount != NULL)
	{
//		*out_ui32IOControlCount = m_deckLinkDeviceCount;
		*out_ui32IOControlCount = 1;
	}
	else
	{
		ASSERT(FALSE);
		hr = NVC_E_INVALID_PARAMETER;
	}

	return hr;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::GetIOControlInfo(SNvfIOControlInfo io_psIOInfo[], uint32_t &io_IOInfoCount)
{
	HRESULT hr = NVC_NOERROR;

// 	uint32_t ui32DeviceCount = 0;
// 
// 	for (int i = 0; i < m_vecDeckLink.size(); i++)
// 	{
// 		wchar_t *deviceModleName = NULL;
// 
// 		hr = m_vecDeckLink[i]->GetModelName(&deviceModleName);
// 		ASSERT(SUCCEEDED(hr));
// 
// 		wcscpy(io_psIOInfo[ui32DeviceCount].wszIODeviceName, deviceModleName);
// 
// 		GUID guid;
// 		NvcCreateGuid(&guid);
// 
// 		io_psIOInfo[ui32DeviceCount].guidIODeviceID = guid;
// 
// 		io_psIOInfo[ui32DeviceCount].eDeviceType = keNvfDeviceTypeHardWare;
// 
// 		IDeckLinkAttributes *deckLinkAttributes = NULL;
// 		hr = m_vecDeckLink[i]->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes); 
// 		ASSERT(SUCCEEDED(hr));
// 
// 		int64_t deckLinkNumberOfSubDevices = 0;
// 		hr = deckLinkAttributes->GetInt(BMDDeckLinkNumberOfSubDevices, &deckLinkNumberOfSubDevices);
// 		ASSERT(SUCCEEDED(hr));
// 
// 		io_psIOInfo[ui32DeviceCount].ui32NumOfCaptureCtrl = (uint32_t)deckLinkNumberOfSubDevices;
// 		io_psIOInfo[ui32DeviceCount].ui32NumOfPlayBackCtrl = (uint32_t)deckLinkNumberOfSubDevices;
// 		io_psIOInfo[ui32DeviceCount].ui32NumOfAudioStreamPerCaptureControl = (uint32_t)deckLinkNumberOfSubDevices;
// 		io_psIOInfo[ui32DeviceCount].ui32NumOfAudioStreamPerPlayBackControl = (uint32_t)deckLinkNumberOfSubDevices;
// 		io_psIOInfo[ui32DeviceCount].nIOControlInfoFlags = keNvfIOControlFlags_SupportVCR | keNvfIOControlFlags_SupportDeviceSetting;
// 
// 		ui32DeviceCount ++;
// 
// 		hr = deckLinkAttributes->Release();
// 		ASSERT(SUCCEEDED(hr));
// 	}
// 
// 	ASSERT(ui32DeviceCount == m_deckLinkDeviceCount);
// 
// 	io_IOInfoCount = m_deckLinkDeviceCount;

	wchar_t *deviceModleName = NULL;

	hr = m_deckLink->GetModelName(&deviceModleName);
	ASSERT(SUCCEEDED(hr));

	wcscpy(io_psIOInfo[0].wszIODeviceName, deviceModleName);

	io_psIOInfo[0].guidIODeviceID = CLSID_DeckLinkControl;

	io_psIOInfo[0].eDeviceType = keNvfDeviceTypeHardWare;

	IDeckLinkProfileAttributes *deckLinkProfileAttributes = NULL;
	hr = m_deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&deckLinkProfileAttributes);
	ASSERT(SUCCEEDED(hr));

	int64_t deckLinkNumberOfSubDevices = 0;
	hr = deckLinkProfileAttributes->GetInt(BMDDeckLinkNumberOfSubDevices, &deckLinkNumberOfSubDevices);
	ASSERT(SUCCEEDED(hr));

	io_psIOInfo[0].ui32NumOfCaptureCtrl = (uint32_t)deckLinkNumberOfSubDevices;
	io_psIOInfo[0].ui32NumOfPlayBackCtrl = (uint32_t)deckLinkNumberOfSubDevices;
	for (uint32_t i = 0; i < io_psIOInfo[0].ui32NumOfCaptureCtrl; i++)
		io_psIOInfo[0].aCaptureCtrlChannel[i] = (ENvfVideoIOChannel)(1 << i);

	for (uint32_t i = 0; i < io_psIOInfo[0].ui32NumOfPlayBackCtrl; i++)
		io_psIOInfo[0].aPlayBackCtrlChannel[i] = (ENvfVideoIOChannel)(1 << i);

	io_psIOInfo[0].ui32NumOfAudioStreamPerCaptureControl = (uint32_t)deckLinkNumberOfSubDevices;
	io_psIOInfo[0].ui32NumOfAudioStreamPerPlayBackControl = (uint32_t)deckLinkNumberOfSubDevices;
	io_psIOInfo[0].nIOControlInfoFlags = keNvfIOControlFlags_SupportVCR | keNvfIOControlFlags_SupportDeviceSetting | keNvfIOControlFlags_MutexInputAndOutput;

// 	char szCharName[NVC_MAX_FILENAME];
// 	NvcWideCharToChar(deviceModleName, szCharName, NVC_MAX_FILENAME);
// 	
// 	std::string str(szCharName);
// 
// 	if (str == "DeckLink Studio 2")
// 		io_psIOInfo[0].nIOControlInfoFlags |= keNvfIOControlFlags_MutexInputAndOutput;

	hr = deckLinkProfileAttributes->Release();
	ASSERT(SUCCEEDED(hr));

	io_IOInfoCount = 1;

	return hr;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::CreateIOPlayBackControl
	(
	GUID					in_gIOControlID, 
	uint32_t				in_ui32PlayBackChannels,
	INvfPlaybackControl**	out_ppIPlaybackControl
	)
{
	HRESULT hr = NVC_NOERROR;

	CNvfDeckLinkIOPlaybackControl *pPlayBackControl = new CNvfDeckLinkIOPlaybackControl(static_cast<INvfIOControlFactory*>(this), m_deckLink, in_ui32PlayBackChannels, NULL, &hr);

	if((pPlayBackControl == NULL) || FAILED(hr))
	{
		ASSERT(FALSE);
		hr = NVC_E_OUT_OF_MEMORY;
		return hr;
	}

	*out_ppIPlaybackControl = pPlayBackControl;

	return hr;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::CreateIOCaptureControl
	(
	GUID					in_gIOControlID, 
	uint32_t				in_ui32CaptureChannels,
	INvfCaptureControl**	out_ppICaptureControl
	)
{
	HRESULT hr = NVC_NOERROR;

	CNvfDeckLinkIOCaptureControl* pCaptureControl = new CNvfDeckLinkIOCaptureControl(static_cast<INvfIOControlFactory*>(this), m_deckLink, in_ui32CaptureChannels, NULL, &hr);

	if((pCaptureControl == NULL) || FAILED(hr))
	{
		ASSERT(FALSE);
		hr = NVC_E_OUT_OF_MEMORY;
		return hr;
	}

	*out_ppICaptureControl = pCaptureControl;

	return hr;
}

HRESULT __stdcall CNvfDeckLinkIOControlFactory::CreateVcrControl(GUID in_gIOControlID, INvfVcrControl**	  out_ppIVcrControl)
{
	HRESULT hr = NVC_NOERROR;

	hr = NvcCreateInstance(CLSID_NvfDeviceRS422, 
		NULL, 
		CLSCTX_INPROC, 
		IID_INvfVcrControl,
		(void**)out_ppIVcrControl);

	ASSERT(SUCCEEDED(hr));

	TNvcSmartPtr<IReferenceClock> pJReferenceClock;
	hr = poReferenceClock->QueryInterface(IID_IReferenceClock, (void**)&pJReferenceClock);
	ASSERT(SUCCEEDED(hr));

	TNvcSmartPtr<IUnknown> pJUnkClock;
	hr = pJReferenceClock->QueryInterface(IID_IUnknown, (void**)&pJUnkClock);
	ASSERT(SUCCEEDED(hr));

	hr = (*out_ppIVcrControl)->SetClock(pJUnkClock);
	ASSERT(SUCCEEDED(hr));

	return NVC_NOERROR;
}

