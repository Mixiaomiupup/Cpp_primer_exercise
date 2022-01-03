#include "NvfDeckLinkBuffer.h"
#include "NvcTime.h"
11
CNvfDeckLinkBufferAllocator::CNvfDeckLinkBufferAllocator(IDeckLink* pIDeckLink, bool bOutput, int nWidth, int nRowPitch, int nHeight, BMDPixelFormat ePixelFormat, int nChannelCount, int nBitDepth,  HRESULT* io_phr ) 
	:  CNvcLightUnknown( NULL , NULL )
{
	*io_phr = NVC_NOERROR;

	m_pJDeckLink = pIDeckLink;
	m_bOutput = bOutput;

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nRowPitch = nRowPitch;
	m_ePixelFormat = ePixelFormat;

	m_nAudioChannelCount = nChannelCount;
	m_nBitDepth = nBitDepth;
}
//
what'r'sajjsjj
//==========================================================================;
hahaha
HRESULT CNvfDeckLinkBufferAllocator::NonDelegatingQueryInterface( REFIID in_rIID, void * * out_ppv )
{
	if (in_rIID == IID_INvcPoolAllocator)
	{
		return NvcGetInterface((INvcPoolAllocator *) this, out_ppv);
	}
	else
	{
		return CNvcLightUnknown::NonDelegatingQueryInterface(in_rIID, out_ppv);
	}
}

HRESULT CNvfDeckLinkBufferAllocator::Allocate( IUnknown ** io_ppIElement )
{
	HRESULT hr = NVC_NOERROR;

	CNvfDeckLinkBuffer* poElement = new CNvfDeckLinkBuffer( );
	if ( poElement == NULL )
	{
		hr = NVC_E_OUT_OF_MEMORY;
	}

	TNvcSmartPtr<IDeckLinkMutableVideoFrame> pJVideoFrame;
	if(SUCCEEDED(hr))
	{
		if(m_bOutput)
		{
			TNvcSmartPtr<IDeckLinkOutput> pJDeckLinOutput;

			hr = m_pJDeckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&pJDeckLinOutput);
			ASSERT(SUCCEEDED(hr));

			if(SUCCEEDED(hr))
				hr = pJDeckLinOutput->CreateVideoFrame(m_nWidth, m_nHeight, m_nRowPitch, m_ePixelFormat, bmdFrameFlagDefault, &pJVideoFrame);
		}
	}

	if(SUCCEEDED(hr))
		hr = poElement->AllocVideoBuffer(pJVideoFrame);

	if(SUCCEEDED(hr))
		hr = poElement->AllocAudioBuffer(m_nAudioChannelCount, m_nBitDepth);

	if ( SUCCEEDED( hr ) )
	{
		hr = poElement->QueryInterface( IID_IUnknown, reinterpret_cast<void**>(io_ppIElement));
	}
	// release the extra ref count
	poElement->Release();

	ASSERT( SUCCEEDED( hr ) );

	return hr;
}

#define NVC_MAX_AUDIO_SAMPLE_COUNT_IN_FRAME  2002
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

CNvfDeckLinkBuffer::CNvfDeckLinkBuffer()
	: CNvcPoolElement(L"CNvfDeckLinkBuffer", NULL)
{
	m_pAudioDataBuffer = NULL;
	m_nAudioChannelCount = 0;
	m_nBitDepth = 0;
	m_nAudioDataSize = 0;

	m_displayTime = (BMDTimeValue)(-1);
}

CNvfDeckLinkBuffer::~CNvfDeckLinkBuffer()
{
	if(m_pAudioDataBuffer != NULL)
		free(m_pAudioDataBuffer);

	m_pAudioDataBuffer = NULL;
	m_nAudioChannelCount = 0;
	m_nBitDepth = 0;
	m_nAudioDataSize = 0;
}

HRESULT CNvfDeckLinkBuffer::NonDelegatingQueryInterface( REFIID in_rIID, void * * out_ppv )
{
	if (in_rIID == IID_IDeckLinkMutableVideoFrame)
	{
		return NvcGetInterface((IDeckLinkMutableVideoFrame *) this, out_ppv);
	}
	else if (in_rIID == IID_INvcCOMPoolElement)
	{
		return NvcGetInterface((INvcCOMPoolElement *) this, out_ppv);
	}
	else
	{
		return CNvcLightUnknown::NonDelegatingQueryInterface(in_rIID, out_ppv);
	}
}

long CNvfDeckLinkBuffer::GetWidth()
{
	if(m_pJVideoFrameBuffer == NULL)
		return 0;

	return m_pJVideoFrameBuffer->GetWidth();
}

long CNvfDeckLinkBuffer::GetHeight()
{
	if(m_pJVideoFrameBuffer == NULL)
		return 0;

	return m_pJVideoFrameBuffer->GetHeight();
}

long CNvfDeckLinkBuffer::GetRowBytes()
{
	if(m_pJVideoFrameBuffer == NULL)
		return 0;

	return m_pJVideoFrameBuffer->GetRowBytes();
}

BMDPixelFormat CNvfDeckLinkBuffer::GetPixelFormat()
{
	if(m_pJVideoFrameBuffer == NULL)
		return bmdFormat8BitYUV;

	return m_pJVideoFrameBuffer->GetPixelFormat();
}

BMDFrameFlags CNvfDeckLinkBuffer::GetFlags()
{
	if(m_pJVideoFrameBuffer == NULL)
		return bmdFrameFlagDefault;

	return m_pJVideoFrameBuffer->GetPixelFormat();
}

HRESULT CNvfDeckLinkBuffer::GetBytes(void **buffer)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->GetBytes(buffer);
}

HRESULT CNvfDeckLinkBuffer::GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->GetTimecode(format, timecode);
}

HRESULT CNvfDeckLinkBuffer::GetAncillaryData(IDeckLinkVideoFrameAncillary **ancillary)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->GetAncillaryData(ancillary);
}

HRESULT CNvfDeckLinkBuffer::SetFlags(BMDFrameFlags newFlags)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->SetFlags(newFlags);
}

HRESULT CNvfDeckLinkBuffer::SetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode *timecode)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->SetTimecode(format, timecode);
}


HRESULT CNvfDeckLinkBuffer::SetAncillaryData(IDeckLinkVideoFrameAncillary *ancillary)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->SetAncillaryData(ancillary);
}

HRESULT CNvfDeckLinkBuffer::SetTimecodeFromComponents(BMDTimecodeFormat format, 
	unsigned char hours,
	unsigned char minutes,
	unsigned char seconds,
	unsigned char frames,
	BMDTimecodeFlags flags)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->SetTimecodeFromComponents(format, hours, minutes, seconds, frames, flags);
}

HRESULT CNvfDeckLinkBuffer::SetTimecodeUserBits(BMDTimecodeFormat format, BMDTimecodeUserBits userBits)
{
	if(m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	return m_pJVideoFrameBuffer->SetTimecodeUserBits(format, userBits);
}

HRESULT CNvfDeckLinkBuffer::AllocVideoBuffer(IDeckLinkMutableVideoFrame* pIVideoFrameBuffer)
{
	m_pJVideoFrameBuffer = pIVideoFrameBuffer;

	return NVC_NOERROR;
}

HRESULT CNvfDeckLinkBuffer::AllocAudioBuffer(int nChannelCount, int nBitDepth)
{
	if(m_pAudioDataBuffer != NULL)
	{
		free(m_pAudioDataBuffer);
		m_pAudioDataBuffer = NULL;
	}

	m_nAudioChannelCount = nChannelCount;
	m_nBitDepth = nBitDepth;

	m_nAudioDataSize = 0;

	int nAudioSamplesSize = nChannelCount * (nBitDepth / 8) * NVC_MAX_AUDIO_SAMPLE_COUNT_IN_FRAME;

	m_pAudioDataBuffer = malloc(nAudioSamplesSize);

	return NVC_NOERROR;
}

HRESULT CNvfDeckLinkBuffer::GetAudioSampleBytes(void **buffer, int* out_pnSampleCount)
{
	if(buffer != NULL)
		*buffer = m_pAudioDataBuffer;

	if(out_pnSampleCount != NULL)
		*out_pnSampleCount = m_nAudioDataSize / (m_nAudioChannelCount * (m_nBitDepth / 8));

	return NVC_NOERROR;
}

HRESULT CNvfDeckLinkBuffer::AddVideoSurface(INvfSurface* in_pInputSurface)
{
	if(in_pInputSurface == NULL || m_pJVideoFrameBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	HRESULT hr = NVC_NOERROR;

	SNvfSurfaceDescription sSurfaceDesc = { sizeof(SNvfSurfaceDescription)};

	hr = in_pInputSurface->GetSurfaceDescription(&sSurfaceDesc);

	SNvfLockSurfaceDescription sLockDesc = { sizeof(SNvfLockSurfaceDescription)};
	hr = in_pInputSurface->Lock(0, keNvfFaceTypeFront, &sLockDesc);

	long width = m_pJVideoFrameBuffer->GetWidth();
	long height = m_pJVideoFrameBuffer->GetHeight();
	long pitch = m_pJVideoFrameBuffer->GetRowBytes();

	unsigned char* pDestBuffer = NULL;
	m_pJVideoFrameBuffer->GetBytes((void **)&pDestBuffer);
	uint32_t nLineSize = width * 2;

// 	uint32_t ui32Pitch = 0;
// 	uint32_t nLineSize = 0;
// 	NvfGetLineSizeInByte(sSurfaceDesc.eFormat, sSurfaceDesc.ui32Width, sSurfaceDesc.ui32ComponentBitCount, &nLineSize, &ui32Pitch);

// 	if(SUCCEEDED(hr))
// 		NvcCopySurface(sLockDesc.pBuffer, sLockDesc.ui32RowPitchInBytes, pDestBuffer, picth, nLineSize, height);

	long srcLine = height - 1;

	for (long i = 0; i < height; i++)
	{
		unsigned char *pSrcBuffer = (unsigned char*)sLockDesc.pBuffer + sLockDesc.ui32RowPitchInBytes * srcLine;

		memcpy(pDestBuffer + pitch * i, pSrcBuffer, pitch);

		srcLine --;
	}
	auto funWriteEncodeStream = [](unsigned char* buf, int size, char* filePath)->int
	{
		FILE* pFile = fopen(filePath, "wb+");
		if (!pFile)
		{
			return -1;
		}
		fwrite(buf, 1, size, pFile);
		fflush(pFile);
		fclose(pFile);
		return 1;
	};
	char sre[1024] = "E://1.YUV";
	funWriteEncodeStream((unsigned char*)(char*)pDestBuffer, 7680 * 2160, sre);
	in_pInputSurface->Unlock(0, keNvfFaceTypeFront);


	TNvcSmartPtr<INvfAVContent> pJAVContent;
	in_pInputSurface->QueryInterface(IID_INvfAVContent, (void**)&pJAVContent);
	if(pJAVContent == NULL)
		return NVC_E_INVALID_POINTER;

	pJAVContent->GetTimeStampInfo((uint64_t *)&m_displayTime);

	pJAVContent->GetStreamPosition(&m_ui64StreamTime);

	m_pJSurface = in_pInputSurface;

	return hr;
}

HRESULT CNvfDeckLinkBuffer::AddAudioSamples(INvfAudioSamples* in_paIInputSamples[], int nArraySize)
{
	if(m_pAudioDataBuffer == NULL || nArraySize == 0)
		return NVC_E_INVALID_POINTER;

	HRESULT hr = NVC_NOERROR;
	std::vector<SNvfAudioSamplesDescription> vecAudioSampleDesc;
	std::vector<void*> vecAudioInputBuffer;

	int nSampleCount = 0;

	in_paIInputSamples[0]->GetValidBufferLengthInSamples((uint32_t*)&nSampleCount);
	ASSERT(nSampleCount > 0);

	uint8_t* paInputBufferPointer[kui32MaxAudioStreamCount];

	int nMaxInputChannelCount = 0;

	for (int i = 0; i < nArraySize; i++)
	{
		paInputBufferPointer[i] = NULL;

		uint32_t ui32BufferLength = 0;
		hr = in_paIInputSamples[i]->GetBufferAndLength((void**)&paInputBufferPointer[i], &ui32BufferLength);
		ASSERT(SUCCEEDED(hr));

		SNvfAudioSamplesDescription sAudioDesc = { sizeof(SNvfAudioSamplesDescription)};
		hr = in_paIInputSamples[i]->GetAudioSamplesDescription(&sAudioDesc);
		ASSERT(SUCCEEDED(hr));

		nMaxInputChannelCount += sAudioDesc.sWaveFormat.eChannelType;

		vecAudioSampleDesc.push_back(sAudioDesc);

		m_vecAudioSamples.push_back(in_paIInputSamples[i]);
	}

	ASSERT((int)vecAudioSampleDesc.size() == nArraySize);

	uint8_t* ui8DstBufferPointer = (uint8_t*)m_pAudioDataBuffer;
	int nSizePerSample = m_nAudioChannelCount * m_nBitDepth / 8;

	for (int i = 0; i < nSampleCount; i++)
	{
		if(nMaxInputChannelCount < m_nAudioChannelCount)
			memset(ui8DstBufferPointer, 0, nSizePerSample);

		for (int n = 0; n < nArraySize; n++)
		{
			if(n * vecAudioSampleDesc[n].sWaveFormat.eChannelType > m_nAudioChannelCount)
				continue;

			int nOneSampleSize = vecAudioSampleDesc[n].sWaveFormat.eChannelType * (vecAudioSampleDesc[n].sWaveFormat.ui32BitsPerSample / 8);

			memcpy(ui8DstBufferPointer + nOneSampleSize * n, paInputBufferPointer[n], nOneSampleSize);

			paInputBufferPointer[n] += nOneSampleSize;
		}

		ui8DstBufferPointer += nSizePerSample;
	}

	m_nAudioDataSize = m_nAudioChannelCount * (m_nBitDepth / 8) * nSampleCount;

	return hr;
}

void CNvfDeckLinkBuffer::SetTimeStampInfo(BMDTimeValue btvTime)
{
	m_displayTime = btvTime;
}

BMDTimeValue CNvfDeckLinkBuffer::GetTimeStampInfo()
{
	if(m_displayTime == (BMDTimeValue)(-1))
		return 0;

	return m_displayTime;
}

uint64_t CNvfDeckLinkBuffer::GetStreamPosition()
{
	return m_ui64StreamTime;
}

HRESULT CNvfDeckLinkBuffer::FlushAllInputInfo()
{
	HRESULT hr = NVC_NOERROR;

	if(m_pJSurface != NULL)
		SignalSurfaceReadCompletion(m_pJSurface);
	m_pJSurface = NULL;

	for (int i = 0; i < (int)m_vecAudioSamples.size(); i++)
		SignalAudioSamplesReadCompletion(m_vecAudioSamples[i]);

	m_vecAudioSamples.clear();

	return hr;
}

HRESULT CNvfDeckLinkBuffer::ClearVideoAudioBuffer(int nSampleCount)
{
	if(m_pJVideoFrameBuffer == NULL || m_pAudioDataBuffer == NULL)
		return NVC_E_INVALID_POINTER;

	HRESULT hr = NVC_NOERROR;
	//clear video;
	unsigned char *buffer = NULL;
	hr = m_pJVideoFrameBuffer->GetBytes((void**)&buffer);
	ASSERT(SUCCEEDED(hr));

	BMDPixelFormat ePixelFormat = m_pJVideoFrameBuffer->GetPixelFormat();
	int width = m_pJVideoFrameBuffer->GetWidth();
	int pitch = m_pJVideoFrameBuffer->GetRowBytes();
	int height = m_pJVideoFrameBuffer->GetHeight();

	if(ePixelFormat == bmdFormat8BitYUV)
	{
		 sNvcBGRA_8Bit sColor;
		 sColor.a =1;
		 sColor.b = 0;
		 sColor.g = 0;
		 sColor.r = 0;
		sNvcYUV_8Bit sYUV;
		Nvc_sNvcBGRA_8Bit_To_sNvcYUV_8Bit(&sColor, &sYUV, NvcGetColorimetry(NvfGetDefaltColorimetyByWidth(width)));
		NvcClear_8Bit_UYVY(buffer, pitch, width, height, &sYUV, 1);

//		NvcClear_8Bit_RGBA(buffer, pitch, width, height, &sColor, 1);
	}
	else
	{
		ASSERT(false);
	}

	m_nAudioDataSize = m_nAudioChannelCount * (m_nBitDepth / 8) * nSampleCount;

	memset(m_pAudioDataBuffer, 0, m_nAudioDataSize);

	return hr;
}
