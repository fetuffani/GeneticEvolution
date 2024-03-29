/* FAudio - XAudio Reimplementation for FNA
 *
 * Copyright (c) 2011-2020 Ethan Lee, Luigi Auriemma, and the MonoGame Team
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Ethan "flibitijibibo" Lee <flibitijibibo@flibitijibibo.com>
 *
 */

#include "FAudio_internal.h"

#define MAKE_SUBFORMAT_GUID(guid, fmt) \
	FAudioGUID DATAFORMAT_SUBTYPE_##guid = \
	{ \
		(uint16_t) (fmt), \
		0x0000, \
		0x0010, \
		{ \
			0x80, \
			0x00, \
			0x00, \
			0xAA, \
			0x00, \
			0x38, \
			0x9B, \
			0x71 \
		} \
	}
MAKE_SUBFORMAT_GUID(PCM, 1);
MAKE_SUBFORMAT_GUID(ADPCM, 2);
MAKE_SUBFORMAT_GUID(IEEE_FLOAT, 3);
MAKE_SUBFORMAT_GUID(XMAUDIO2, FAUDIO_FORMAT_XMAUDIO2);
MAKE_SUBFORMAT_GUID(WMAUDIO2, FAUDIO_FORMAT_WMAUDIO2);
MAKE_SUBFORMAT_GUID(WMAUDIO3, FAUDIO_FORMAT_WMAUDIO3);
MAKE_SUBFORMAT_GUID(WMAUDIO_LOSSLESS, FAUDIO_FORMAT_WMAUDIO_LOSSLESS);
#undef MAKE_SUBFORMAT_GUID

/* FAudio Version */

uint32_t FAudioLinkedVersion(void)
{
	return FAUDIO_COMPILED_VERSION;
}

/* FAudio Interface */

uint32_t FAudioCreate(
	FAudio **ppFAudio,
	uint32_t Flags,
	FAudioProcessor XAudio2Processor
) {
	FAudioCOMConstructEXT(ppFAudio, FAUDIO_TARGET_VERSION);
	FAudio_Initialize(*ppFAudio, Flags, XAudio2Processor);
	return 0;
}

uint32_t FAudioCOMConstructEXT(FAudio **ppFAudio, uint8_t version)
{
	return FAudioCOMConstructWithCustomAllocatorEXT(
		ppFAudio,
		version,
		FAudio_malloc,
		FAudio_free,
		FAudio_realloc
	);
}

uint32_t FAudioCreateWithCustomAllocatorEXT(
	FAudio **ppFAudio,
	uint32_t Flags,
	FAudioProcessor XAudio2Processor,
	FAudioMallocFunc customMalloc,
	FAudioFreeFunc customFree,
	FAudioReallocFunc customRealloc
) {
	FAudioCOMConstructWithCustomAllocatorEXT(
		ppFAudio,
		FAUDIO_TARGET_VERSION,
		customMalloc,
		customFree,
		customRealloc
	);
	FAudio_Initialize(*ppFAudio, Flags, XAudio2Processor);
	return 0;
}

uint32_t FAudioCOMConstructWithCustomAllocatorEXT(
	FAudio **ppFAudio,
	uint8_t version,
	FAudioMallocFunc customMalloc,
	FAudioFreeFunc customFree,
	FAudioReallocFunc customRealloc
) {
#ifndef FAUDIO_DISABLE_DEBUGCONFIGURATION
	FAudioDebugConfiguration debugInit = {0};
#endif /* FAUDIO_DISABLE_DEBUGCONFIGURATION */
	FAudio_PlatformAddRef();
	*ppFAudio = (FAudio*) customMalloc(sizeof(FAudio));
	FAudio_zero(*ppFAudio, sizeof(FAudio));
	(*ppFAudio)->version = version;
#ifndef FAUDIO_DISABLE_DEBUGCONFIGURATION
	FAudio_SetDebugConfiguration(*ppFAudio, &debugInit, NULL);
#endif /* FAUDIO_DISABLE_DEBUGCONFIGURATION */
	(*ppFAudio)->sourceLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE((*ppFAudio), (*ppFAudio)->sourceLock)
	(*ppFAudio)->submixLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE((*ppFAudio), (*ppFAudio)->submixLock)
	(*ppFAudio)->callbackLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE((*ppFAudio), (*ppFAudio)->callbackLock)
	(*ppFAudio)->operationLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE((*ppFAudio), (*ppFAudio)->operationLock)
	(*ppFAudio)->pMalloc = customMalloc;
	(*ppFAudio)->pFree = customFree;
	(*ppFAudio)->pRealloc = customRealloc;
	(*ppFAudio)->refcount = 1;
	return 0;
}

uint32_t FAudio_AddRef(FAudio *audio)
{
	LOG_API_ENTER(audio)
	audio->refcount += 1;
	LOG_API_EXIT(audio)
	return audio->refcount;
}

uint32_t FAudio_Release(FAudio *audio)
{
	uint32_t refcount;
	LOG_API_ENTER(audio)
	audio->refcount -= 1;
	refcount = audio->refcount;
	if (audio->refcount == 0)
	{
		FAudio_OPERATIONSET_ClearAll(audio);
		FAudio_StopEngine(audio);
		audio->pFree(audio->decodeCache);
		audio->pFree(audio->resampleCache);
		audio->pFree(audio->effectChainCache);
		LOG_MUTEX_DESTROY(audio, audio->sourceLock)
		FAudio_PlatformDestroyMutex(audio->sourceLock);
		LOG_MUTEX_DESTROY(audio, audio->submixLock)
		FAudio_PlatformDestroyMutex(audio->submixLock);
		LOG_MUTEX_DESTROY(audio, audio->callbackLock)
		FAudio_PlatformDestroyMutex(audio->callbackLock);
		LOG_MUTEX_DESTROY(audio, audio->operationLock)
		FAudio_PlatformDestroyMutex(audio->operationLock);
		audio->pFree(audio);
		FAudio_PlatformRelease();
	}
	else
	{
		LOG_API_EXIT(audio)
	}
	return refcount;
}

uint32_t FAudio_GetDeviceCount(FAudio *audio, uint32_t *pCount)
{
	LOG_API_ENTER(audio)
	*pCount = FAudio_PlatformGetDeviceCount();
	LOG_API_EXIT(audio)
	return 0;
}

uint32_t FAudio_GetDeviceDetails(
	FAudio *audio,
	uint32_t Index,
	FAudioDeviceDetails *pDeviceDetails
) {
	uint32_t result;
	LOG_API_ENTER(audio)
	result = FAudio_PlatformGetDeviceDetails(Index, pDeviceDetails);
	LOG_API_EXIT(audio)
	return result;
}

uint32_t FAudio_Initialize(
	FAudio *audio,
	uint32_t Flags,
	FAudioProcessor XAudio2Processor
) {
	LOG_API_ENTER(audio)
	FAudio_assert(Flags == 0);
	FAudio_assert(XAudio2Processor == FAUDIO_DEFAULT_PROCESSOR);

	audio->initFlags = Flags;

	/* FIXME: This is lazy... */
	audio->decodeCache = (float*) audio->pMalloc(sizeof(float));
	audio->resampleCache = (float*) audio->pMalloc(sizeof(float));
	audio->decodeSamples = 1;
	audio->resampleSamples = 1;

	FAudio_StartEngine(audio);
	LOG_API_EXIT(audio)
	return 0;
}

uint32_t FAudio_RegisterForCallbacks(
	FAudio *audio,
	FAudioEngineCallback *pCallback
) {
	LOG_API_ENTER(audio)
	LinkedList_AddEntry(
		&audio->callbacks,
		pCallback,
		audio->callbackLock,
		audio->pMalloc
	);
	LOG_API_EXIT(audio)
	return 0;
}

void FAudio_UnregisterForCallbacks(
	FAudio *audio,
	FAudioEngineCallback *pCallback
) {
	LOG_API_ENTER(audio)
	LinkedList_RemoveEntry(
		&audio->callbacks,
		pCallback,
		audio->callbackLock,
		audio->pFree
	);
	LOG_API_EXIT(audio)
}

uint32_t FAudio_CreateSourceVoice(
	FAudio *audio,
	FAudioSourceVoice **ppSourceVoice,
	const FAudioWaveFormatEx *pSourceFormat,
	uint32_t Flags,
	float MaxFrequencyRatio,
	FAudioVoiceCallback *pCallback,
	const FAudioVoiceSends *pSendList,
	const FAudioEffectChain *pEffectChain
) {
	uint32_t i;

	LOG_API_ENTER(audio)
	LOG_FORMAT(audio, pSourceFormat)

	*ppSourceVoice = (FAudioSourceVoice*) audio->pMalloc(sizeof(FAudioVoice));
	FAudio_zero(*ppSourceVoice, sizeof(FAudioSourceVoice));
	(*ppSourceVoice)->audio = audio;
	(*ppSourceVoice)->type = FAUDIO_VOICE_SOURCE;
	(*ppSourceVoice)->flags = Flags;
	(*ppSourceVoice)->filter.Type = FAUDIO_DEFAULT_FILTER_TYPE;
	(*ppSourceVoice)->filter.Frequency = FAUDIO_DEFAULT_FILTER_FREQUENCY;
	(*ppSourceVoice)->filter.OneOverQ = FAUDIO_DEFAULT_FILTER_ONEOVERQ;
	(*ppSourceVoice)->sendLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSourceVoice)->sendLock)
	(*ppSourceVoice)->effectLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSourceVoice)->effectLock)
	(*ppSourceVoice)->filterLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSourceVoice)->filterLock)
	(*ppSourceVoice)->volumeLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSourceVoice)->volumeLock)

	/* Source Properties */
	FAudio_assert(MaxFrequencyRatio <= FAUDIO_MAX_FREQ_RATIO);
	(*ppSourceVoice)->src.maxFreqRatio = MaxFrequencyRatio;

	if (	pSourceFormat->wFormatTag == FAUDIO_FORMAT_PCM ||
		pSourceFormat->wFormatTag == FAUDIO_FORMAT_IEEE_FLOAT ||
		pSourceFormat->wFormatTag == FAUDIO_FORMAT_XMAUDIO2 ||
		pSourceFormat->wFormatTag == FAUDIO_FORMAT_WMAUDIO2	)
	{
		FAudioWaveFormatExtensible *fmtex = (FAudioWaveFormatExtensible*) audio->pMalloc(
			sizeof(FAudioWaveFormatExtensible)
		);
		/* convert PCM to EXTENSIBLE */
		fmtex->Format.wFormatTag = FAUDIO_FORMAT_EXTENSIBLE;
		fmtex->Format.nChannels = pSourceFormat->nChannels;
		fmtex->Format.nSamplesPerSec = pSourceFormat->nSamplesPerSec;
		fmtex->Format.nAvgBytesPerSec = pSourceFormat->nAvgBytesPerSec;
		fmtex->Format.nBlockAlign = pSourceFormat->nBlockAlign;
		fmtex->Format.wBitsPerSample = pSourceFormat->wBitsPerSample;
		fmtex->Format.cbSize = sizeof(FAudioWaveFormatExtensible) - sizeof(FAudioWaveFormatEx);
		fmtex->Samples.wValidBitsPerSample = pSourceFormat->wBitsPerSample;
		fmtex->dwChannelMask = 0;
		if (pSourceFormat->wFormatTag == FAUDIO_FORMAT_PCM)
		{
			FAudio_memcpy(&fmtex->SubFormat, &DATAFORMAT_SUBTYPE_PCM, sizeof(FAudioGUID));
		}
		else if (pSourceFormat->wFormatTag == FAUDIO_FORMAT_IEEE_FLOAT)
		{
			FAudio_memcpy(&fmtex->SubFormat, &DATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(FAudioGUID));
		}
		else if (pSourceFormat->wFormatTag == FAUDIO_FORMAT_XMAUDIO2)
		{
			FAudio_memcpy(&fmtex->SubFormat, &DATAFORMAT_SUBTYPE_XMAUDIO2, sizeof(FAudioGUID));
		}
		else if (pSourceFormat->wFormatTag == FAUDIO_FORMAT_WMAUDIO2)
		{
			FAudio_memcpy(&fmtex->SubFormat, &DATAFORMAT_SUBTYPE_WMAUDIO2, sizeof(FAudioGUID));
		}
		(*ppSourceVoice)->src.format = &fmtex->Format;
	}
	else if (pSourceFormat->wFormatTag == FAUDIO_FORMAT_MSADPCM)
	{
		FAudioADPCMWaveFormat *fmtex = (FAudioADPCMWaveFormat*) audio->pMalloc(
			sizeof(FAudioADPCMWaveFormat)
		);

		/* Copy what we can, ideally the sizes match! */
		size_t cbSize = sizeof(FAudioWaveFormatEx) + pSourceFormat->cbSize;
		FAudio_memcpy(
			fmtex,
			pSourceFormat,
			FAudio_min(cbSize, sizeof(FAudioADPCMWaveFormat))
		);
		if (cbSize < sizeof(FAudioADPCMWaveFormat))
		{
			FAudio_zero(
				((uint8_t*) fmtex) + cbSize,
				sizeof(FAudioADPCMWaveFormat) - cbSize
			);
		}

		/* XAudio2 does not validate this input! */
		fmtex->wfx.cbSize = sizeof(FAudioADPCMWaveFormat) - sizeof(FAudioWaveFormatEx);
		fmtex->wSamplesPerBlock = ((
			fmtex->wfx.nBlockAlign / fmtex->wfx.nChannels
		) - 6) * 2;
		(*ppSourceVoice)->src.format = &fmtex->wfx;
	}
	else
	{
		/* direct copy anything else */
		(*ppSourceVoice)->src.format = (FAudioWaveFormatEx*) audio->pMalloc(
			sizeof(FAudioWaveFormatEx) + pSourceFormat->cbSize
		);
		FAudio_memcpy(
			(*ppSourceVoice)->src.format,
			pSourceFormat,
			sizeof(FAudioWaveFormatEx) + pSourceFormat->cbSize
		);
	}

	(*ppSourceVoice)->src.callback = pCallback;
	(*ppSourceVoice)->src.active = 0;
	(*ppSourceVoice)->src.freqRatio = 1.0f;
	(*ppSourceVoice)->src.totalSamples = 0;
	(*ppSourceVoice)->src.bufferList = NULL;
	(*ppSourceVoice)->src.bufferLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSourceVoice)->src.bufferLock)

	if ((*ppSourceVoice)->src.format->wFormatTag == FAUDIO_FORMAT_EXTENSIBLE)
	{
		FAudioWaveFormatExtensible *fmtex = (FAudioWaveFormatExtensible*) (*ppSourceVoice)->src.format;

		#define COMPARE_GUID(type) \
			(FAudio_memcmp( \
				&fmtex->SubFormat, \
				&DATAFORMAT_SUBTYPE_##type, \
				sizeof(FAudioGUID) \
			) == 0)
		if (COMPARE_GUID(PCM))
		{
			#define DECODER(bit) \
				if (fmtex->Format.wBitsPerSample == bit) \
				{ \
					(*ppSourceVoice)->src.decode = FAudio_INTERNAL_DecodePCM##bit; \
				}
			DECODER(16)
			else DECODER(8)
			else DECODER(24)
			else DECODER(32)
			else
			{
				LOG_ERROR(
					audio,
					"Unrecognized wBitsPerSample: %d",
					fmtex->Format.wBitsPerSample
				)
				FAudio_assert(0 && "Unrecognized wBitsPerSample!");
			}
			#undef DECODER
		}
		else if (COMPARE_GUID(IEEE_FLOAT))
		{
			/* FIXME: Weird behavior!
			 * Prototype creates a source with the IEEE_FLOAT tag,
			 * but it's actually PCM16. It seems to prioritize
			 * wBitsPerSample over the format tag. Not sure if we
			 * should fold this section into the section above...?
			 * -flibit
			 */
			if (fmtex->Format.wBitsPerSample == 16)
			{
				(*ppSourceVoice)->src.decode = FAudio_INTERNAL_DecodePCM16;
			}
			else
			{
				(*ppSourceVoice)->src.decode = FAudio_INTERNAL_DecodePCM32F;
			}
		}
		else if (	COMPARE_GUID(WMAUDIO2) ||
				COMPARE_GUID(WMAUDIO3) ||
				COMPARE_GUID(WMAUDIO_LOSSLESS) ||
				COMPARE_GUID(XMAUDIO2)	)
		{
#ifdef HAVE_FFMPEG
			if (FAudio_FFMPEG_init(*ppSourceVoice, fmtex->SubFormat.Data1) != 0)
			{
				(*ppSourceVoice)->src.decode = FAudio_INTERNAL_DecodeWMAERROR;
			}
#else
			FAudio_assert(0 && "xWMA is not supported!");
			(*ppSourceVoice)->src.decode = FAudio_INTERNAL_DecodeWMAERROR;
#endif /* HAVE_FFMPEG */
		}
		else
		{
			FAudio_assert(0 && "Unsupported WAVEFORMATEXTENSIBLE subtype!");
		}
		#undef COMPARE_GUID
	}
	else if ((*ppSourceVoice)->src.format->wFormatTag == FAUDIO_FORMAT_MSADPCM)
	{
		(*ppSourceVoice)->src.decode = ((*ppSourceVoice)->src.format->nChannels == 2) ?
			FAudio_INTERNAL_DecodeStereoMSADPCM :
			FAudio_INTERNAL_DecodeMonoMSADPCM;
	}
	else
	{
		FAudio_assert(0 && "Unsupported format tag!");
	}

	if ((*ppSourceVoice)->src.format->nChannels == 1)
	{
		(*ppSourceVoice)->src.resample = FAudio_INTERNAL_ResampleMono;
	}
	else if ((*ppSourceVoice)->src.format->nChannels == 2)
	{
		(*ppSourceVoice)->src.resample = FAudio_INTERNAL_ResampleStereo;
	}
	else
	{
		(*ppSourceVoice)->src.resample = FAudio_INTERNAL_ResampleGeneric;
	}

	(*ppSourceVoice)->src.curBufferOffset = 0;

	/* Sends/Effects */
	FAudioVoice_SetEffectChain(*ppSourceVoice, pEffectChain);
	FAudioVoice_SetOutputVoices(*ppSourceVoice, pSendList);

	/* Default Levels */
	(*ppSourceVoice)->volume = 1.0f;
	(*ppSourceVoice)->channelVolume = (float*) audio->pMalloc(
		sizeof(float) * (*ppSourceVoice)->outputChannels
	);
	for (i = 0; i < (*ppSourceVoice)->outputChannels; i += 1)
	{
		(*ppSourceVoice)->channelVolume[i] = 1.0f;
	}

	/* Filters */
	if (Flags & FAUDIO_VOICE_USEFILTER)
	{
		(*ppSourceVoice)->filterState = (FAudioFilterState*) audio->pMalloc(
			sizeof(FAudioFilterState) * (*ppSourceVoice)->src.format->nChannels
		);
		FAudio_zero(
			(*ppSourceVoice)->filterState,
			sizeof(FAudioFilterState) * (*ppSourceVoice)->src.format->nChannels
		);
	}

	/* Sample Storage */
	(*ppSourceVoice)->src.decodeSamples = (uint32_t) (FAudio_ceil(
		(double) audio->updateSize *
		(double) MaxFrequencyRatio *
		(double) (*ppSourceVoice)->src.format->nSamplesPerSec /
		(double) audio->master->master.inputSampleRate
	)) + EXTRA_DECODE_PADDING * (*ppSourceVoice)->src.format->nChannels;
	FAudio_INTERNAL_ResizeDecodeCache(
		audio,
		((*ppSourceVoice)->src.decodeSamples + EXTRA_DECODE_PADDING) * (*ppSourceVoice)->src.format->nChannels
	);

	LOG_INFO(audio, "-> %p", (void*) (*ppSourceVoice))

	/* Add to list, finally. */
	LinkedList_PrependEntry(
		&audio->sources,
		*ppSourceVoice,
		audio->sourceLock,
		audio->pMalloc
	);
	FAudio_AddRef(audio);

	LOG_API_EXIT(audio)
	return 0;
}

uint32_t FAudio_CreateSubmixVoice(
	FAudio *audio,
	FAudioSubmixVoice **ppSubmixVoice,
	uint32_t InputChannels,
	uint32_t InputSampleRate,
	uint32_t Flags,
	uint32_t ProcessingStage,
	const FAudioVoiceSends *pSendList,
	const FAudioEffectChain *pEffectChain
) {
	uint32_t i;

	LOG_API_ENTER(audio)

	*ppSubmixVoice = (FAudioSubmixVoice*) audio->pMalloc(sizeof(FAudioVoice));
	FAudio_zero(*ppSubmixVoice, sizeof(FAudioSubmixVoice));
	(*ppSubmixVoice)->audio = audio;
	(*ppSubmixVoice)->type = FAUDIO_VOICE_SUBMIX;
	(*ppSubmixVoice)->flags = Flags;
	(*ppSubmixVoice)->filter.Type = FAUDIO_DEFAULT_FILTER_TYPE;
	(*ppSubmixVoice)->filter.Frequency = FAUDIO_DEFAULT_FILTER_FREQUENCY;
	(*ppSubmixVoice)->filter.OneOverQ = FAUDIO_DEFAULT_FILTER_ONEOVERQ;
	(*ppSubmixVoice)->sendLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSubmixVoice)->sendLock)
	(*ppSubmixVoice)->effectLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSubmixVoice)->effectLock)
	(*ppSubmixVoice)->filterLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSubmixVoice)->filterLock)
	(*ppSubmixVoice)->volumeLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppSubmixVoice)->volumeLock)

	/* Submix Properties */
	(*ppSubmixVoice)->mix.inputChannels = InputChannels;
	(*ppSubmixVoice)->mix.inputSampleRate = InputSampleRate;
	(*ppSubmixVoice)->mix.processingStage = ProcessingStage;

	/* Sample Storage */
	(*ppSubmixVoice)->mix.inputSamples = ((uint32_t) FAudio_ceil(
		audio->updateSize *
		(double) InputSampleRate /
		(double) audio->master->master.inputSampleRate
	) + EXTRA_DECODE_PADDING) * InputChannels;
	(*ppSubmixVoice)->mix.inputCache = (float*) audio->pMalloc(
		sizeof(float) * (*ppSubmixVoice)->mix.inputSamples
	);
	FAudio_zero( /* Zero this now, for the first update */
		(*ppSubmixVoice)->mix.inputCache,
		sizeof(float) * (*ppSubmixVoice)->mix.inputSamples
	);

	/* Sends/Effects */
	FAudioVoice_SetEffectChain(*ppSubmixVoice, pEffectChain);
	FAudioVoice_SetOutputVoices(*ppSubmixVoice, pSendList);

	/* Default Levels */
	(*ppSubmixVoice)->volume = 1.0f;
	(*ppSubmixVoice)->channelVolume = (float*) audio->pMalloc(
		sizeof(float) * (*ppSubmixVoice)->outputChannels
	);
	for (i = 0; i < (*ppSubmixVoice)->outputChannels; i += 1)
	{
		(*ppSubmixVoice)->channelVolume[i] = 1.0f;
	}

	/* Filters */
	if (Flags & FAUDIO_VOICE_USEFILTER)
	{
		(*ppSubmixVoice)->filterState = (FAudioFilterState*) audio->pMalloc(
			sizeof(FAudioFilterState) * InputChannels
		);
		FAudio_zero(
			(*ppSubmixVoice)->filterState,
			sizeof(FAudioFilterState) * InputChannels
		);
	}

	/* Add to list, finally. */
	FAudio_INTERNAL_InsertSubmixSorted(
		&audio->submixes,
		*ppSubmixVoice,
		audio->submixLock,
		audio->pMalloc
	);
	FAudio_AddRef(audio);

	LOG_API_EXIT(audio)
	return 0;
}

uint32_t FAudio_CreateMasteringVoice(
	FAudio *audio,
	FAudioMasteringVoice **ppMasteringVoice,
	uint32_t InputChannels,
	uint32_t InputSampleRate,
	uint32_t Flags,
	uint32_t DeviceIndex,
	const FAudioEffectChain *pEffectChain
) {
	FAudioDeviceDetails details;

	LOG_API_ENTER(audio)

	/* For now we only support one allocated master voice at a time */
	FAudio_assert(audio->master == NULL);

	if (FAudio_GetDeviceDetails(audio, DeviceIndex, &details) != 0)
	{
		return FAUDIO_E_INVALID_CALL;
	}

	*ppMasteringVoice = (FAudioMasteringVoice*) audio->pMalloc(sizeof(FAudioVoice));
	FAudio_zero(*ppMasteringVoice, sizeof(FAudioMasteringVoice));
	(*ppMasteringVoice)->audio = audio;
	(*ppMasteringVoice)->type = FAUDIO_VOICE_MASTER;
	(*ppMasteringVoice)->flags = Flags;
	(*ppMasteringVoice)->effectLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppMasteringVoice)->effectLock)
	(*ppMasteringVoice)->volumeLock = FAudio_PlatformCreateMutex();
	LOG_MUTEX_CREATE(audio, (*ppMasteringVoice)->volumeLock)

	/* Default Levels */
	(*ppMasteringVoice)->volume = 1.0f;

	/* Master Properties */
	(*ppMasteringVoice)->master.inputChannels = (InputChannels == FAUDIO_DEFAULT_CHANNELS) ?
		details.OutputFormat.Format.nChannels :
		InputChannels;
	(*ppMasteringVoice)->master.inputSampleRate = (InputSampleRate == FAUDIO_DEFAULT_SAMPLERATE) ?
		details.OutputFormat.Format.nSamplesPerSec :
		InputSampleRate;

	/* Sends/Effects */
	FAudio_zero(&(*ppMasteringVoice)->sends, sizeof(FAudioVoiceSends));
	FAudioVoice_SetEffectChain(*ppMasteringVoice, pEffectChain);

	/* This is now safe enough to assign */
	audio->master = *ppMasteringVoice;

	/* Build the device format.
	 * The most unintuitive part of this is the use of outputChannels
	 * instead of master.inputChannels. Bizarrely, the effect chain can
	 * dictate the _actual_ output channel count, and when the channel count
	 * mismatches, we have to add a staging buffer for effects to process on
	 * before ultimately copying the final result to the device. ARGH.
	 */
	WriteWaveFormatExtensible(
		&audio->mixFormat,
		audio->master->outputChannels,
		audio->master->master.inputSampleRate
	);

	/* Platform Device */
	FAudio_AddRef(audio);
	FAudio_PlatformInit(
		audio,
		audio->initFlags,
		DeviceIndex,
		&audio->mixFormat,
		&audio->updateSize,
		&audio->platform
	);
	if (audio->platform == NULL)
	{
		FAudioVoice_DestroyVoice(*ppMasteringVoice);
		*ppMasteringVoice = NULL;

		/* Not the best code, but it's probably true? */
		return FAUDIO_E_DEVICE_INVALIDATED;
	}
	audio->master->outputChannels = audio->mixFormat.Format.nChannels;
	audio->master->master.inputSampleRate = audio->mixFormat.Format.nSamplesPerSec;

	/* Effect Chain Cache */
	if ((*ppMasteringVoice)->master.inputChannels != (*ppMasteringVoice)->outputChannels)
	{
		(*ppMasteringVoice)->master.effectCache = (float*) audio->pMalloc(
			sizeof(float) *
			audio->updateSize *
			(*ppMasteringVoice)->master.inputChannels
		);
	}

	LOG_API_EXIT(audio)
	return 0;
}

uint32_t FAudio_CreateMasteringVoice8(
	FAudio *audio,
	FAudioMasteringVoice **ppMasteringVoice,
	uint32_t InputChannels,
	uint32_t InputSampleRate,
	uint32_t Flags,
	uint16_t *szDeviceId,
	const FAudioEffectChain *pEffectChain,
	FAudioStreamCategory StreamCategory
) {
	uint32_t DeviceIndex, retval;

	LOG_API_ENTER(audio)

	/* Eventually, we'll want the old CreateMastering to call the new one.
	 * That will depend on us being able to use DeviceID though.
	 * For now, use our little ID hack to turn szDeviceId into DeviceIndex.
	 * -flibit
	 */
	if (szDeviceId == NULL || szDeviceId[0] == 0)
	{
		DeviceIndex = 0;
	}
	else
	{
		DeviceIndex = szDeviceId[0] - L'0';
		if (DeviceIndex > FAudio_PlatformGetDeviceCount())
		{
			DeviceIndex = 0;
		}
	}

	/* Note that StreamCategory is ignored! */
	retval = FAudio_CreateMasteringVoice(
		audio,
		ppMasteringVoice,
		InputChannels,
		InputSampleRate,
		Flags,
		DeviceIndex,
		pEffectChain
	);

	LOG_API_EXIT(audio)
	return retval;
}

void FAudio_SetEngineProcedureEXT(
	FAudio *audio,
	FAudioEngineProcedureEXT clientEngineProc,
	void *user
) {
	LOG_API_ENTER(audio)
	audio->pClientEngineProc = clientEngineProc;
	audio->clientEngineUser = user;
	LOG_API_EXIT(audio)
}

uint32_t FAudio_StartEngine(FAudio *audio)
{
	LOG_API_ENTER(audio)
	audio->active = 1;
	LOG_API_EXIT(audio)
	return 0;
}

void FAudio_StopEngine(FAudio *audio)
{
	LOG_API_ENTER(audio)
	audio->active = 0;
	FAudio_OPERATIONSET_CommitAll(audio);
	FAudio_OPERATIONSET_Execute(audio);
	LOG_API_EXIT(audio)
}

uint32_t FAudio_CommitOperationSet(FAudio *audio, uint32_t OperationSet)
{
	LOG_API_ENTER(audio)
	if (OperationSet == FAUDIO_COMMIT_ALL)
	{
		FAudio_OPERATIONSET_CommitAll(audio);
	}
	else
	{
		FAudio_OPERATIONSET_Commit(audio, OperationSet);
	}
	LOG_API_EXIT(audio)
	return 0;
}

uint32_t FAudio_CommitChanges(FAudio *audio)
{
	FAudio_Log(
		"IF YOU CAN READ THIS, YOUR PROGRAM IS ABOUT TO BREAK!"
		"\n\nEither you or somebody else is using FAudio_CommitChanges,"
		"\nwhen they should be using FAudio_CommitOperationSet instead."
		"\n\nIf your program calls this, move to CommitOperationSet."
		"\n\nIf somebody else is calling this, find out who it is and"
		"\nfile a bug report with them ASAP."
	);

	/* Seriously, this is like the worst possible thing short of no-oping.
	 * For the love-a Pete, just migrate, do it, what is wrong with you
	 */
	return FAudio_CommitOperationSet(audio, FAUDIO_COMMIT_ALL);
}

void FAudio_GetPerformanceData(
	FAudio *audio,
	FAudioPerformanceData *pPerfData
) {
	LinkedList *list;
	FAudioSourceVoice *source;

	LOG_API_ENTER(audio)

	FAudio_zero(pPerfData, sizeof(FAudioPerformanceData));

	FAudio_PlatformLockMutex(audio->sourceLock);
	LOG_MUTEX_LOCK(audio, audio->sourceLock)
	list = audio->sources;
	while (list != NULL)
	{
		source = (FAudioSourceVoice*) list->entry;
		pPerfData->TotalSourceVoiceCount += 1;
		if (source->src.active)
		{
			pPerfData->ActiveSourceVoiceCount += 1;
		}
		list = list->next;
	}
	FAudio_PlatformUnlockMutex(audio->sourceLock);
	LOG_MUTEX_UNLOCK(audio, audio->sourceLock)

	FAudio_PlatformLockMutex(audio->submixLock);
	LOG_MUTEX_LOCK(audio, audio->submixLock)
	list = audio->submixes;
	while (list != NULL)
	{
		pPerfData->ActiveSubmixVoiceCount += 1;
		list = list->next;
	}
	FAudio_PlatformUnlockMutex(audio->submixLock);
	LOG_MUTEX_UNLOCK(audio, audio->submixLock)

	if (audio->master != NULL)
	{
		/* estimate, should use real latency from platform */
		pPerfData->CurrentLatencyInSamples = 2 * audio->updateSize;
	}

	LOG_API_EXIT(audio)
}

void FAudio_SetDebugConfiguration(
	FAudio *audio,
	FAudioDebugConfiguration *pDebugConfiguration,
	void* pReserved
) {
#ifndef FAUDIO_DISABLE_DEBUGCONFIGURATION
	char *env;

	LOG_API_ENTER(audio)

	FAudio_memcpy(
		&audio->debug,
		pDebugConfiguration,
		sizeof(FAudioDebugConfiguration)
	);

	env = FAudio_getenv("FAUDIO_LOG_EVERYTHING");
	if (env != NULL && *env == '1')
	{
		audio->debug.TraceMask = (
			FAUDIO_LOG_ERRORS |
			FAUDIO_LOG_WARNINGS |
			FAUDIO_LOG_INFO |
			FAUDIO_LOG_DETAIL |
			FAUDIO_LOG_API_CALLS |
			FAUDIO_LOG_FUNC_CALLS |
			FAUDIO_LOG_TIMING |
			FAUDIO_LOG_LOCKS |
			FAUDIO_LOG_MEMORY |
			FAUDIO_LOG_STREAMING
		);
		audio->debug.LogThreadID = 1;
		audio->debug.LogFunctionName = 1;
		audio->debug.LogTiming = 1;
	}

	#define CHECK_ENV(type) \
		env = FAudio_getenv("FAUDIO_LOG_" #type); \
		if (env != NULL) \
		{ \
			if (*env == '1') \
			{ \
				audio->debug.TraceMask |= FAUDIO_LOG_##type; \
			} \
			else \
			{ \
				audio->debug.TraceMask &= ~FAUDIO_LOG_##type; \
			} \
		}
	CHECK_ENV(ERRORS)
	CHECK_ENV(WARNINGS)
	CHECK_ENV(INFO)
	CHECK_ENV(DETAIL)
	CHECK_ENV(API_CALLS)
	CHECK_ENV(FUNC_CALLS)
	CHECK_ENV(TIMING)
	CHECK_ENV(LOCKS)
	CHECK_ENV(MEMORY)
	CHECK_ENV(STREAMING)
	#undef CHECK_ENV
	#define CHECK_ENV(envvar, boolvar) \
		env = FAudio_getenv("FAUDIO_LOG_LOG" #envvar); \
		if (env != NULL) \
		{ \
			audio->debug.Log##boolvar = (*env == '1'); \
		}
	CHECK_ENV(THREADID, ThreadID)
	CHECK_ENV(FILELINE, Fileline)
	CHECK_ENV(FUNCTIONNAME, FunctionName)
	CHECK_ENV(TIMING, Timing)
	#undef CHECK_ENV

	LOG_API_EXIT(audio)
#endif /* FAUDIO_DISABLE_DEBUGCONFIGURATION */
}

void FAudio_GetProcessingQuantum(
	FAudio *audio,
	uint32_t *quantumNumerator,
	uint32_t *quantumDenominator
) {
	FAudio_assert(audio->master != NULL);
	if (quantumNumerator != NULL)
	{
		*quantumNumerator = audio->updateSize;
	}
	if (quantumDenominator != NULL)
	{
		*quantumDenominator = audio->master->master.inputSampleRate;
	}
}

/* FAudioVoice Interface */

void FAudioVoice_GetVoiceDetails(
	FAudioVoice *voice,
	FAudioVoiceDetails *pVoiceDetails
) {
	LOG_API_ENTER(voice->audio)

	pVoiceDetails->CreationFlags = voice->flags;
	pVoiceDetails->ActiveFlags = voice->flags;
	if (voice->type == FAUDIO_VOICE_SOURCE)
	{
		pVoiceDetails->InputChannels = voice->src.format->nChannels;
		pVoiceDetails->InputSampleRate = voice->src.format->nSamplesPerSec;
	}
	else if (voice->type == FAUDIO_VOICE_SUBMIX)
	{
		pVoiceDetails->InputChannels = voice->mix.inputChannels;
		pVoiceDetails->InputSampleRate = voice->mix.inputSampleRate;
	}
	else if (voice->type == FAUDIO_VOICE_MASTER)
	{
		pVoiceDetails->InputChannels = voice->master.inputChannels;
		pVoiceDetails->InputSampleRate = voice->master.inputSampleRate;
	}
	else
	{
		FAudio_assert(0 && "Unknown voice type!");
	}

	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioVoice_SetOutputVoices(
	FAudioVoice *voice,
	const FAudioVoiceSends *pSendList
) {
	uint32_t i;
	uint32_t outChannels;
	uint32_t channelCount = 0;
	uint32_t outSampleRate;
	uint32_t newResampleSamples;
	uint64_t resampleSanityCheck;
	FAudioVoiceSends defaultSends;
	FAudioSendDescriptor defaultSend;

	LOG_API_ENTER(voice->audio)

	if (voice->type == FAUDIO_VOICE_MASTER)
	{
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	FAudio_PlatformLockMutex(voice->sendLock);
	LOG_MUTEX_LOCK(voice->audio, voice->sendLock)

	/* FIXME: This is lazy... */
	for (i = 0; i < voice->sends.SendCount; i += 1)
	{
		voice->audio->pFree(voice->sendCoefficients[i]);
	}
	if (voice->sendCoefficients != NULL)
	{
		voice->audio->pFree(voice->sendCoefficients);
	}
	if (voice->sendMix != NULL)
	{
		voice->audio->pFree(voice->sendMix);
	}
	if (voice->sendFilter != NULL)
	{
		voice->audio->pFree(voice->sendFilter);
		voice->sendFilter = NULL;
	}
	if (voice->sendFilterState != NULL)
	{
		for (i = 0; i < voice->sends.SendCount; i += 1)
		{
			if (voice->sendFilterState[i] != NULL)
			{
				voice->audio->pFree(voice->sendFilterState[i]);
			}
		}
		voice->audio->pFree(voice->sendFilterState);
		voice->sendFilterState = NULL;
	}
	if (voice->sends.pSends != NULL)
	{
		voice->audio->pFree(voice->sends.pSends);
	}

	if (pSendList == NULL)
	{
		/* Default to the mastering voice as output */
		defaultSend.Flags = 0;
		defaultSend.pOutputVoice = voice->audio->master;
		defaultSends.SendCount = 1;
		defaultSends.pSends = &defaultSend;
		pSendList = &defaultSends;
	}
	else if (pSendList->SendCount == 0)
	{
		/* No sends? Nothing to do... */
		voice->sendCoefficients = NULL;
		voice->sendMix = NULL;
		voice->sendFilter = NULL;
		voice->sendFilterState = NULL;
		FAudio_zero(&voice->sends, sizeof(FAudioVoiceSends));
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)

		LOG_API_EXIT(voice->audio)
		return 0;
	}

	/* Copy send list */
	voice->sends.SendCount = pSendList->SendCount;
	voice->sends.pSends = (FAudioSendDescriptor*) voice->audio->pMalloc(
		pSendList->SendCount * sizeof(FAudioSendDescriptor)
	);
	FAudio_memcpy(
		voice->sends.pSends,
		pSendList->pSends,
		pSendList->SendCount * sizeof(FAudioSendDescriptor)
	);

	/* Allocate/Reset default output matrix, mixer function, filters */
	voice->sendCoefficients = (float**) voice->audio->pMalloc(
		sizeof(float*) * pSendList->SendCount
	);
	voice->sendMix = (FAudioMixCallback*) voice->audio->pMalloc(
		sizeof(FAudioMixCallback) * pSendList->SendCount
	);
	for (i = 0; i < pSendList->SendCount; i += 1)
	{
		if (pSendList->pSends[i].pOutputVoice->type == FAUDIO_VOICE_MASTER)
		{
			outChannels = pSendList->pSends[i].pOutputVoice->master.inputChannels;
		}
		else
		{
			outChannels = pSendList->pSends[i].pOutputVoice->mix.inputChannels;
		}
		voice->sendCoefficients[i] = (float*) voice->audio->pMalloc(
			sizeof(float) * voice->outputChannels * outChannels
		);

		FAudio_assert(voice->outputChannels > 0 && voice->outputChannels < 9);
		FAudio_assert(outChannels > 0 && outChannels < 9);
		FAudio_memcpy(
			voice->sendCoefficients[i],
			FAUDIO_INTERNAL_MATRIX_DEFAULTS[voice->outputChannels - 1][outChannels - 1],
			voice->outputChannels * outChannels * sizeof(float)
		);

		if (voice->outputChannels == 1)
		{
			if (outChannels == 1)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_1in_1out_Scalar;
			}
			else if (outChannels == 2)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_1in_2out_Scalar;
			}
			else if (outChannels == 6)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_1in_6out_Scalar;
			}
			else if (outChannels == 8)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_1in_8out_Scalar;
			}
			else
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_Generic_Scalar;
			}
		}
		else if (voice->outputChannels == 2)
		{
			if (outChannels == 1)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_2in_1out_Scalar;
			}
			else if (outChannels == 2)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_2in_2out_Scalar;
			}
			else if (outChannels == 6)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_2in_6out_Scalar;
			}
			else if (outChannels == 8)
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_2in_8out_Scalar;
			}
			else
			{
				voice->sendMix[i] = FAudio_INTERNAL_Mix_Generic_Scalar;
			}
		}
		else
		{
			voice->sendMix[i] = FAudio_INTERNAL_Mix_Generic_Scalar;
		}

		if (pSendList->pSends[i].Flags & FAUDIO_SEND_USEFILTER)
		{
			/* Allocate the whole send filter array if needed... */
			if (voice->sendFilter == NULL)
			{
				voice->sendFilter = (FAudioFilterParameters*) voice->audio->pMalloc(
					sizeof(FAudioFilterParameters) * pSendList->SendCount
				);
			}
			if (voice->sendFilterState == NULL)
			{
				voice->sendFilterState = (FAudioFilterState**) voice->audio->pMalloc(
					sizeof(FAudioFilterState*) * pSendList->SendCount
				);
				FAudio_zero(
					voice->sendFilterState,
					sizeof(FAudioFilterState*) * pSendList->SendCount
				);
			}

			/* ... then fill in this send's filter data */
			voice->sendFilter[i].Type = FAUDIO_DEFAULT_FILTER_TYPE;
			voice->sendFilter[i].Frequency = FAUDIO_DEFAULT_FILTER_FREQUENCY;
			voice->sendFilter[i].OneOverQ = FAUDIO_DEFAULT_FILTER_ONEOVERQ;
			voice->sendFilterState[i] = (FAudioFilterState*) voice->audio->pMalloc(
				sizeof(FAudioFilterState) * outChannels
			);
			FAudio_zero(
				voice->sendFilterState[i],
				sizeof(FAudioFilterState) * outChannels
			);
		}
	}

	/* Allocate resample cache */
	outSampleRate = voice->sends.pSends[0].pOutputVoice->type == FAUDIO_VOICE_MASTER ?
		voice->sends.pSends[0].pOutputVoice->master.inputSampleRate :
		voice->sends.pSends[0].pOutputVoice->mix.inputSampleRate;
	newResampleSamples = (uint32_t) FAudio_ceil(
		voice->audio->updateSize *
		(double) outSampleRate /
		(double) voice->audio->master->master.inputSampleRate
	);
	if (voice->type == FAUDIO_VOICE_SOURCE)
	{
		channelCount = voice->src.format->nChannels;
	}
	else if (voice->type == FAUDIO_VOICE_SUBMIX)
	{
		channelCount = voice->mix.inputChannels;
	}
	else if (voice->type == FAUDIO_VOICE_MASTER)
	{
		channelCount = voice->master.inputChannels;
	}
	FAudio_INTERNAL_ResizeResampleCache(
		voice->audio,
		newResampleSamples * channelCount
	);
	if (voice->type == FAUDIO_VOICE_SOURCE)
	{
		voice->src.resampleSamples = newResampleSamples;
	}
	else
	{
		voice->mix.outputSamples = newResampleSamples;

		voice->mix.resampleStep = DOUBLE_TO_FIXED((
			(double) voice->mix.inputSampleRate /
			(double) outSampleRate
		));

		/* Because we used ceil earlier, there's a chance that
		 * downsampling submixes will go past the number of samples
		 * available. Sources can do this thanks to padding, but we
		 * don't have that luxury for submixes, so unfortunately we
		 * just have to undo the ceil and turn it into a floor.
		 * -flibit
		 */
		resampleSanityCheck = (
			voice->mix.resampleStep * voice->mix.outputSamples
		) >> FIXED_PRECISION;
		if (resampleSanityCheck > (voice->mix.inputSamples / voice->mix.inputChannels))
		{
			voice->mix.outputSamples -= 1;
		}

		if (voice->mix.inputChannels == 1)
		{
			voice->mix.resample = FAudio_INTERNAL_ResampleMono;
		}
		else if (voice->mix.inputChannels == 2)
		{
			voice->mix.resample = FAudio_INTERNAL_ResampleStereo;
		}
		else
		{
			voice->mix.resample = FAudio_INTERNAL_ResampleGeneric;
		}
	}

	FAudio_PlatformUnlockMutex(voice->sendLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioVoice_SetEffectChain(
	FAudioVoice *voice,
	const FAudioEffectChain *pEffectChain
) {
	uint32_t i;
	FAPO *fapo;
	uint32_t channelCount;
	FAudioVoiceDetails voiceDetails;
	FAPORegistrationProperties *pProps;
	FAudioWaveFormatExtensible srcFmt, dstFmt;
	FAPOLockForProcessBufferParameters srcLockParams, dstLockParams;

	LOG_API_ENTER(voice->audio)

	FAudioVoice_GetVoiceDetails(voice, &voiceDetails);

	/* SetEffectChain must not change the number of output channels once the voice has been created */
	if (pEffectChain == NULL && voice->outputChannels != 0)
	{
		/* cannot remove an effect chain that changes the number of channels */
		if (voice->outputChannels != voiceDetails.InputChannels)
		{
			LOG_ERROR(
				voice->audio,
				"%s",
				"Cannot remove effect chain that changes the number of channels"
			)
			FAudio_assert(0 && "Cannot remove effect chain that changes the number of channels");
			LOG_API_EXIT(voice->audio)
			return FAUDIO_E_INVALID_CALL;
		}
	}

	if (pEffectChain != NULL && voice->outputChannels != 0)
	{
		uint32_t lst = pEffectChain->EffectCount - 1;

		/* new effect chain must have same number of output channels */
		if (voice->outputChannels != pEffectChain->pEffectDescriptors[lst].OutputChannels)
		{
			LOG_ERROR(
				voice->audio,
				"%s",
				"New effect chain must have same number of output channels as the old chain"
			)
			FAudio_assert(0 && "New effect chain must have same number of output channels as the old chain");
			LOG_API_EXIT(voice->audio)
			return FAUDIO_E_INVALID_CALL;
		}
	}

	FAudio_PlatformLockMutex(voice->effectLock);
	LOG_MUTEX_LOCK(voice->audio, voice->effectLock)

	if (pEffectChain == NULL)
	{
		FAudio_INTERNAL_FreeEffectChain(voice);
		FAudio_zero(&voice->effects, sizeof(voice->effects));
		voice->outputChannels = voiceDetails.InputChannels;
	}
	else
	{
		/* Validate incoming chain before changing the current chain */

		/* These are always the same, so just write them now. */
		srcLockParams.pFormat = &srcFmt.Format;
		dstLockParams.pFormat = &dstFmt.Format;
		if (voice->type == FAUDIO_VOICE_SOURCE)
		{
			srcLockParams.MaxFrameCount = voice->src.resampleSamples;
			dstLockParams.MaxFrameCount = voice->src.resampleSamples;
		}
		else if (voice->type == FAUDIO_VOICE_SUBMIX)
		{
			srcLockParams.MaxFrameCount = voice->mix.outputSamples;
			dstLockParams.MaxFrameCount = voice->mix.outputSamples;
		}
		else if (voice->type == FAUDIO_VOICE_MASTER)
		{
			srcLockParams.MaxFrameCount = voice->audio->updateSize;
			dstLockParams.MaxFrameCount = voice->audio->updateSize;
		}

		/* The first source is the voice input data... */
		srcFmt.Format.wBitsPerSample = 32;
		srcFmt.Format.wFormatTag = FAUDIO_FORMAT_EXTENSIBLE;
		srcFmt.Format.nChannels = voiceDetails.InputChannels;
		srcFmt.Format.nSamplesPerSec = voiceDetails.InputSampleRate;
		srcFmt.Format.nBlockAlign = srcFmt.Format.nChannels * (srcFmt.Format.wBitsPerSample / 8);
		srcFmt.Format.nAvgBytesPerSec = srcFmt.Format.nSamplesPerSec * srcFmt.Format.nBlockAlign;
		srcFmt.Format.cbSize = sizeof(FAudioWaveFormatExtensible) - sizeof(FAudioWaveFormatEx);
		srcFmt.Samples.wValidBitsPerSample = srcFmt.Format.wBitsPerSample;
		srcFmt.dwChannelMask = 0;
		FAudio_memcpy(&srcFmt.SubFormat, &DATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(FAudioGUID));
		FAudio_memcpy(&dstFmt, &srcFmt, sizeof(srcFmt));

		for (i = 0; i < pEffectChain->EffectCount; i += 1)
		{
			fapo = pEffectChain->pEffectDescriptors[i].pEffect;

			/* ... then we get this effect's format... */
			dstFmt.Format.nChannels = pEffectChain->pEffectDescriptors[i].OutputChannels;
			dstFmt.Format.nBlockAlign = dstFmt.Format.nChannels * (dstFmt.Format.wBitsPerSample / 8);
			dstFmt.Format.nAvgBytesPerSec = dstFmt.Format.nSamplesPerSec * dstFmt.Format.nBlockAlign;

			if (fapo->LockForProcess(fapo, 1, &srcLockParams, 1, &dstLockParams))
			{
				LOG_ERROR(
					voice->audio,
					"%s",
					"Effect output format not supported"
				)
				FAudio_assert(0 && "Effect output format not supported");
				FAudio_PlatformUnlockMutex(voice->effectLock);
				LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
				LOG_API_EXIT(voice->audio)
				return FAUDIO_E_UNSUPPORTED_FORMAT;
			}

			/* Okay, now this effect is the source and the next
			 * effect will be the destination. Repeat until no
			 * effects left.
			 */
			FAudio_memcpy(&srcFmt, &dstFmt, sizeof(srcFmt));
		}

		FAudio_INTERNAL_FreeEffectChain(voice);
		FAudio_INTERNAL_AllocEffectChain(
			voice,
			pEffectChain
		);

		/* check if in-place processing is supported */
		channelCount = voiceDetails.InputChannels;
		for (i = 0; i < voice->effects.count; i += 1)
		{
			fapo = voice->effects.desc[i].pEffect;
			if (fapo->GetRegistrationProperties(fapo, &pProps) == 0)
			{
				voice->effects.inPlaceProcessing[i] = (pProps->Flags & FAPO_FLAG_INPLACE_SUPPORTED) == FAPO_FLAG_INPLACE_SUPPORTED;
				voice->effects.inPlaceProcessing[i] &= (channelCount == voice->effects.desc[i].OutputChannels);
				channelCount = voice->effects.desc[i].OutputChannels;

				/* Fails if in-place processing is mandatory and
				 * the chain forces us to do otherwise...
				 */
				FAudio_assert(
					!(pProps->Flags & FAPO_FLAG_INPLACE_REQUIRED) ||
					voice->effects.inPlaceProcessing[i]
				);

				voice->audio->pFree(pProps);
			}
		}
		voice->outputChannels = channelCount;
	}

	FAudio_PlatformUnlockMutex(voice->effectLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioVoice_EnableEffect(
	FAudioVoice *voice,
	uint32_t EffectIndex,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueEnableEffect(
			voice,
			EffectIndex,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_PlatformLockMutex(voice->effectLock);
	LOG_MUTEX_LOCK(voice->audio, voice->effectLock)
	voice->effects.desc[EffectIndex].InitialState = 1;
	FAudio_PlatformUnlockMutex(voice->effectLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioVoice_DisableEffect(
	FAudioVoice *voice,
	uint32_t EffectIndex,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueDisableEffect(
			voice,
			EffectIndex,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_PlatformLockMutex(voice->effectLock);
	LOG_MUTEX_LOCK(voice->audio, voice->effectLock)
	voice->effects.desc[EffectIndex].InitialState = 0;
	FAudio_PlatformUnlockMutex(voice->effectLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioVoice_GetEffectState(
	FAudioVoice *voice,
	uint32_t EffectIndex,
	int32_t *pEnabled
) {
	LOG_API_ENTER(voice->audio)
	FAudio_PlatformLockMutex(voice->effectLock);
	LOG_MUTEX_LOCK(voice->audio, voice->effectLock)
	*pEnabled = voice->effects.desc[EffectIndex].InitialState;
	FAudio_PlatformUnlockMutex(voice->effectLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioVoice_SetEffectParameters(
	FAudioVoice *voice,
	uint32_t EffectIndex,
	const void *pParameters,
	uint32_t ParametersByteSize,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetEffectParameters(
			voice,
			EffectIndex,
			pParameters,
			ParametersByteSize,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	if (voice->effects.parameters[EffectIndex] == NULL)
	{
		voice->effects.parameters[EffectIndex] = voice->audio->pMalloc(
			ParametersByteSize
		);
		voice->effects.parameterSizes[EffectIndex] = ParametersByteSize;
	}
	FAudio_PlatformLockMutex(voice->effectLock);
	LOG_MUTEX_LOCK(voice->audio, voice->effectLock)
	if (voice->effects.parameterSizes[EffectIndex] < ParametersByteSize)
	{
		voice->effects.parameters[EffectIndex] = voice->audio->pRealloc(
			voice->effects.parameters[EffectIndex],
			ParametersByteSize
		);
		voice->effects.parameterSizes[EffectIndex] = ParametersByteSize;
	}
	FAudio_memcpy(
		voice->effects.parameters[EffectIndex],
		pParameters,
		ParametersByteSize
	);
	voice->effects.parameterUpdates[EffectIndex] = 1;
	FAudio_PlatformUnlockMutex(voice->effectLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioVoice_GetEffectParameters(
	FAudioVoice *voice,
	uint32_t EffectIndex,
	void *pParameters,
	uint32_t ParametersByteSize
) {
	FAPO *fapo;
	LOG_API_ENTER(voice->audio)
	FAudio_PlatformLockMutex(voice->effectLock);
	LOG_MUTEX_LOCK(voice->audio, voice->effectLock)
	fapo = voice->effects.desc[EffectIndex].pEffect;
	fapo->GetParameters(fapo, pParameters, ParametersByteSize);
	FAudio_PlatformUnlockMutex(voice->effectLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioVoice_SetFilterParameters(
	FAudioVoice *voice,
	const FAudioFilterParameters *pParameters,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetFilterParameters(
			voice,
			pParameters,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	/* MSDN: "This method is usable only on source and submix voices and
	 * has no effect on mastering voices."
	 */
	if (voice->type == FAUDIO_VOICE_MASTER)
	{
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	if (!(voice->flags & FAUDIO_VOICE_USEFILTER))
	{
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_PlatformLockMutex(voice->filterLock);
	LOG_MUTEX_LOCK(voice->audio, voice->filterLock)
	FAudio_memcpy(
		&voice->filter,
		pParameters,
		sizeof(FAudioFilterParameters)
	);
	FAudio_PlatformUnlockMutex(voice->filterLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->filterLock)

	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioVoice_GetFilterParameters(
	FAudioVoice *voice,
	FAudioFilterParameters *pParameters
) {
	LOG_API_ENTER(voice->audio)

	/* MSDN: "This method is usable only on source and submix voices and
	 * has no effect on mastering voices."
	 */
	if (voice->type == FAUDIO_VOICE_MASTER)
	{
		LOG_API_EXIT(voice->audio)
		return;
	}

	if (!(voice->flags & FAUDIO_VOICE_USEFILTER))
	{
		LOG_API_EXIT(voice->audio)
		return;
	}

	FAudio_PlatformLockMutex(voice->filterLock);
	LOG_MUTEX_LOCK(voice->audio, voice->filterLock)
	FAudio_memcpy(
		pParameters,
		&voice->filter,
		sizeof(FAudioFilterParameters)
	);
	FAudio_PlatformUnlockMutex(voice->filterLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->filterLock)
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioVoice_SetOutputFilterParameters(
	FAudioVoice *voice,
	FAudioVoice *pDestinationVoice,
	const FAudioFilterParameters *pParameters,
	uint32_t OperationSet
) {
	uint32_t i;
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetOutputFilterParameters(
			voice,
			pDestinationVoice,
			pParameters,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	/* MSDN: "This method is usable only on source and submix voices and
	 * has no effect on mastering voices."
	 */
	if (voice->type == FAUDIO_VOICE_MASTER)
	{
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_PlatformLockMutex(voice->sendLock);
	LOG_MUTEX_LOCK(voice->audio, voice->sendLock)

	/* Find the send index */
	if (pDestinationVoice == NULL && voice->sends.SendCount == 1)
	{
		pDestinationVoice = voice->sends.pSends[0].pOutputVoice;
	}
	for (i = 0; i < voice->sends.SendCount; i += 1)
	{
		if (pDestinationVoice == voice->sends.pSends[i].pOutputVoice)
		{
			break;
		}
	}
	if (i >= voice->sends.SendCount)
	{
		LOG_ERROR(
			voice->audio,
			"Destination not attached to source: %p %p",
			(void*) voice,
			(void*) pDestinationVoice
		)
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	if (!(voice->sends.pSends[i].Flags & FAUDIO_SEND_USEFILTER))
	{
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	/* Set the filter parameters, finally. */
	FAudio_memcpy(
		&voice->sendFilter[i],
		pParameters,
		sizeof(FAudioFilterParameters)
	);

	FAudio_PlatformUnlockMutex(voice->sendLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioVoice_GetOutputFilterParameters(
	FAudioVoice *voice,
	FAudioVoice *pDestinationVoice,
	FAudioFilterParameters *pParameters
) {
	uint32_t i;

	LOG_API_ENTER(voice->audio)

	/* MSDN: "This method is usable only on source and submix voices and
	 * has no effect on mastering voices."
	 */
	if (voice->type == FAUDIO_VOICE_MASTER)
	{
		LOG_API_EXIT(voice->audio)
		return;
	}

	FAudio_PlatformLockMutex(voice->sendLock);
	LOG_MUTEX_LOCK(voice->audio, voice->sendLock)

	/* Find the send index */
	if (pDestinationVoice == NULL && voice->sends.SendCount == 1)
	{
		pDestinationVoice = voice->sends.pSends[0].pOutputVoice;
	}
	for (i = 0; i < voice->sends.SendCount; i += 1)
	{
		if (pDestinationVoice == voice->sends.pSends[i].pOutputVoice)
		{
			break;
		}
	}
	if (i >= voice->sends.SendCount)
	{
		LOG_ERROR(
			voice->audio,
			"Destination not attached to source: %p %p",
			(void*) voice,
			(void*) pDestinationVoice
		)
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return;
	}

	if (!(voice->sends.pSends[i].Flags & FAUDIO_SEND_USEFILTER))
	{
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return;
	}

	/* Set the filter parameters, finally. */
	FAudio_memcpy(
		pParameters,
		&voice->sendFilter[i],
		sizeof(FAudioFilterParameters)
	);

	FAudio_PlatformUnlockMutex(voice->sendLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioVoice_SetVolume(
	FAudioVoice *voice,
	float Volume,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetVolume(
			voice,
			Volume,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	voice->volume = FAudio_clamp(
		Volume,
		-FAUDIO_MAX_VOLUME_LEVEL,
		FAUDIO_MAX_VOLUME_LEVEL
	);
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioVoice_GetVolume(
	FAudioVoice *voice,
	float *pVolume
) {
	LOG_API_ENTER(voice->audio)
	*pVolume = voice->volume;
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioVoice_SetChannelVolumes(
	FAudioVoice *voice,
	uint32_t Channels,
	const float *pVolumes,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetChannelVolumes(
			voice,
			Channels,
			pVolumes,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	if (pVolumes == NULL)
	{
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	if (voice->type == FAUDIO_VOICE_MASTER)
	{
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	if (voice->audio->version > 7 && Channels != voice->outputChannels)
	{
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	FAudio_PlatformLockMutex(voice->volumeLock);
	LOG_MUTEX_LOCK(voice->audio, voice->volumeLock)
	FAudio_memcpy(
		voice->channelVolume,
		pVolumes,
		sizeof(float) * Channels
	);
	FAudio_PlatformUnlockMutex(voice->volumeLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->volumeLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioVoice_GetChannelVolumes(
	FAudioVoice *voice,
	uint32_t Channels,
	float *pVolumes
) {
	LOG_API_ENTER(voice->audio)
	FAudio_PlatformLockMutex(voice->volumeLock);
	LOG_MUTEX_LOCK(voice->audio, voice->volumeLock)
	FAudio_memcpy(
		pVolumes,
		voice->channelVolume,
		sizeof(float) * Channels
	);
	FAudio_PlatformUnlockMutex(voice->volumeLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->volumeLock)
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioVoice_SetOutputMatrix(
	FAudioVoice *voice,
	FAudioVoice *pDestinationVoice,
	uint32_t SourceChannels,
	uint32_t DestinationChannels,
	const float *pLevelMatrix,
	uint32_t OperationSet
) {
	uint32_t i;
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetOutputMatrix(
			voice,
			pDestinationVoice,
			SourceChannels,
			DestinationChannels,
			pLevelMatrix,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_PlatformLockMutex(voice->sendLock);
	LOG_MUTEX_LOCK(voice->audio, voice->sendLock)

	/* Find the send index */
	if (pDestinationVoice == NULL && voice->sends.SendCount == 1)
	{
		pDestinationVoice = voice->sends.pSends[0].pOutputVoice;
	}
	FAudio_assert(pDestinationVoice != NULL);
	for (i = 0; i < voice->sends.SendCount; i += 1)
	{
		if (pDestinationVoice == voice->sends.pSends[i].pOutputVoice)
		{
			break;
		}
	}
	if (i >= voice->sends.SendCount)
	{
		LOG_ERROR(
			voice->audio,
			"Destination not attached to source: %p %p",
			(void*) voice,
			(void*) pDestinationVoice
		)
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	/* Verify the Source/Destination channel count */
	FAudio_assert(SourceChannels == voice->outputChannels);

	if (pDestinationVoice->type == FAUDIO_VOICE_MASTER)
	{
		FAudio_assert(DestinationChannels == pDestinationVoice->master.inputChannels);
	}
	else
	{
		FAudio_assert(DestinationChannels == pDestinationVoice->mix.inputChannels);
	}

	/* Set the matrix values, finally */
	FAudio_memcpy(
		voice->sendCoefficients[i],
		pLevelMatrix,
		sizeof(float) * SourceChannels * DestinationChannels
	);

	FAudio_PlatformUnlockMutex(voice->sendLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioVoice_GetOutputMatrix(
	FAudioVoice *voice,
	FAudioVoice *pDestinationVoice,
	uint32_t SourceChannels,
	uint32_t DestinationChannels,
	float *pLevelMatrix
) {
	uint32_t i;

	LOG_API_ENTER(voice->audio)
	FAudio_PlatformLockMutex(voice->sendLock);
	LOG_MUTEX_LOCK(voice->audio, voice->sendLock)

	/* Find the send index */
	for (i = 0; i < voice->sends.SendCount; i += 1)
	{
		if (pDestinationVoice == voice->sends.pSends[i].pOutputVoice)
		{
			break;
		}
	}
	if (i >= voice->sends.SendCount)
	{
		LOG_ERROR(
			voice->audio,
			"Destination not attached to source: %p %p",
			(void*) voice,
			(void*) pDestinationVoice
		)
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return;
	}

	/* Verify the Source/Destination channel count */
	if (voice->type == FAUDIO_VOICE_SOURCE)
	{
		FAudio_assert(SourceChannels == voice->src.format->nChannels);
	}
	else
	{
		FAudio_assert(SourceChannels == voice->mix.inputChannels);
	}
	if (pDestinationVoice->type == FAUDIO_VOICE_MASTER)
	{
		FAudio_assert(DestinationChannels == pDestinationVoice->master.inputChannels);
	}
	else
	{
		FAudio_assert(DestinationChannels == pDestinationVoice->mix.inputChannels);
	}

	/* Get the matrix values, finally */
	FAudio_memcpy(
		pLevelMatrix,
		voice->sendCoefficients[i],
		sizeof(float) * SourceChannels * DestinationChannels
	);

	FAudio_PlatformUnlockMutex(voice->sendLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
	LOG_API_EXIT(voice->audio)
}

void FAudioVoice_DestroyVoice(FAudioVoice *voice)
{
	uint32_t i;
	LOG_API_ENTER(voice->audio)

	/* TODO: Check for dependencies and remove from audio graph first! */
	FAudio_OPERATIONSET_ClearAllForVoice(voice);

	if (voice->type == FAUDIO_VOICE_SOURCE)
	{
		FAudioBufferEntry *entry, *next;

		FAudio_PlatformLockMutex(voice->audio->sourceLock);
		LOG_MUTEX_LOCK(voice->audio, voice->audio->sourceLock)
		while (voice == voice->audio->processingSource)
		{
			FAudio_PlatformUnlockMutex(voice->audio->sourceLock);
			LOG_MUTEX_UNLOCK(voice->audio, voice->audio->sourceLock)
			FAudio_PlatformLockMutex(voice->audio->sourceLock);
			LOG_MUTEX_LOCK(voice->audio, voice->audio->sourceLock)
		}
		LinkedList_RemoveEntry(
			&voice->audio->sources,
			voice,
			voice->audio->sourceLock,
			voice->audio->pFree
		);
		FAudio_PlatformUnlockMutex(voice->audio->sourceLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->audio->sourceLock)

		entry = voice->src.bufferList;
		while (entry != NULL)
		{
			next = entry->next;
			voice->audio->pFree(entry);
			entry = next;
		}

		voice->audio->pFree(voice->src.format);
		LOG_MUTEX_DESTROY(voice->audio, voice->src.bufferLock)
		FAudio_PlatformDestroyMutex(voice->src.bufferLock);
#ifdef HAVE_FFMPEG
		if (voice->src.ffmpeg)
		{
			FAudio_FFMPEG_free(voice);
		}
#endif /* HAVE_FFMPEG */
	}
	else if (voice->type == FAUDIO_VOICE_SUBMIX)
	{
		/* Remove submix from list */
		LinkedList_RemoveEntry(
			&voice->audio->submixes,
			voice,
			voice->audio->submixLock,
			voice->audio->pFree
		);

		/* Delete submix data */
		voice->audio->pFree(voice->mix.inputCache);
	}
	else if (voice->type == FAUDIO_VOICE_MASTER)
	{
		if (voice->audio->platform != NULL)
		{
			FAudio_PlatformQuit(voice->audio->platform);
			voice->audio->platform = NULL;
		}
		if (voice->master.effectCache != NULL)
		{
			voice->audio->pFree(voice->master.effectCache);
		}
		voice->audio->master = NULL;
	}

	if (voice->sendLock != NULL)
	{
		FAudio_PlatformLockMutex(voice->sendLock);
		LOG_MUTEX_LOCK(voice->audio, voice->sendLock)
		for (i = 0; i < voice->sends.SendCount; i += 1)
		{
			voice->audio->pFree(voice->sendCoefficients[i]);
		}
		if (voice->sendCoefficients != NULL)
		{
			voice->audio->pFree(voice->sendCoefficients);
		}
		if (voice->sendMix != NULL)
		{
			voice->audio->pFree(voice->sendMix);
		}
		if (voice->sendFilter != NULL)
		{
			voice->audio->pFree(voice->sendFilter);
		}
		if (voice->sendFilterState != NULL)
		{
			for (i = 0; i < voice->sends.SendCount; i += 1)
			{
				if (voice->sendFilterState[i] != NULL)
				{
					voice->audio->pFree(voice->sendFilterState[i]);
				}
			}
			voice->audio->pFree(voice->sendFilterState);
		}
		if (voice->sends.pSends != NULL)
		{
			voice->audio->pFree(voice->sends.pSends);
		}
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_MUTEX_DESTROY(voice->audio, voice->sendLock)
		FAudio_PlatformDestroyMutex(voice->sendLock);
	}

	if (voice->effectLock != NULL)
	{
		FAudio_PlatformLockMutex(voice->effectLock);
		LOG_MUTEX_LOCK(voice->audio, voice->effectLock)
		FAudio_INTERNAL_FreeEffectChain(voice);
		FAudio_PlatformUnlockMutex(voice->effectLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->effectLock)
		LOG_MUTEX_DESTROY(voice->audio, voice->effectLock)
		FAudio_PlatformDestroyMutex(voice->effectLock);
	}

	if (voice->filterLock != NULL)
	{
		FAudio_PlatformLockMutex(voice->filterLock);
		LOG_MUTEX_LOCK(voice->audio, voice->filterLock)
		if (voice->filterState != NULL)
		{
			voice->audio->pFree(voice->filterState);
		}
		FAudio_PlatformUnlockMutex(voice->filterLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->filterLock)
		LOG_MUTEX_DESTROY(voice->audio, voice->filterLock)
		FAudio_PlatformDestroyMutex(voice->filterLock);
	}

	if (voice->volumeLock != NULL)
	{
		FAudio_PlatformLockMutex(voice->volumeLock);
		LOG_MUTEX_LOCK(voice->audio, voice->volumeLock)
		if (voice->channelVolume != NULL)
		{
			voice->audio->pFree(voice->channelVolume);
		}
		FAudio_PlatformUnlockMutex(voice->volumeLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->volumeLock)
		LOG_MUTEX_DESTROY(voice->audio, voice->volumeLock)
		FAudio_PlatformDestroyMutex(voice->volumeLock);
	}

	LOG_API_EXIT(voice->audio)
	FAudio_Release(voice->audio);
	voice->audio->pFree(voice);
}

/* FAudioSourceVoice Interface */

uint32_t FAudioSourceVoice_Start(
	FAudioSourceVoice *voice,
	uint32_t Flags,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueStart(
			voice,
			Flags,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}


	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	FAudio_assert(Flags == 0);
	voice->src.active = 1;
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioSourceVoice_Stop(
	FAudioSourceVoice *voice,
	uint32_t Flags,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueStop(
			voice,
			Flags,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	if (Flags & FAUDIO_PLAY_TAILS)
	{
		voice->src.active = 2;
	}
	else
	{
		voice->src.active = 0;
	}
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioSourceVoice_SubmitSourceBuffer(
	FAudioSourceVoice *voice,
	const FAudioBuffer *pBuffer,
	const FAudioBufferWMA *pBufferWMA
) {
	uint32_t adpcmMask, *adpcmByteCount;
	uint32_t playBegin, playLength, loopBegin, loopLength;
	FAudioBufferEntry *entry, *list;

	LOG_API_ENTER(voice->audio)
	LOG_INFO(
		voice->audio,
		"%p: {Flags: 0x%x, AudioBytes: %u, pAudioData: %p, Play: %u + %u, Loop: %u + %u x %u}",
		(void*) voice,
		pBuffer->Flags,
		pBuffer->AudioBytes,
		(const void*) pBuffer->pAudioData,
		pBuffer->PlayBegin,
		pBuffer->PlayLength,
		pBuffer->LoopBegin,
		pBuffer->LoopLength,
		pBuffer->LoopCount
	)

	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);
#ifdef HAVE_FFMPEG
	FAudio_assert(	(voice->src.ffmpeg != NULL && pBufferWMA != NULL) ||
			(voice->src.ffmpeg == NULL && pBufferWMA == NULL)	);
#endif /* HAVE_FFMPEG */

	/* Start off with whatever they just sent us... */
	playBegin = pBuffer->PlayBegin;
	playLength = pBuffer->PlayLength;
	loopBegin = pBuffer->LoopBegin;
	loopLength = pBuffer->LoopLength;

	/* "LoopBegin/LoopLength must be zero if LoopCount is 0" */
	if (pBuffer->LoopCount == 0 && (loopBegin > 0 || loopLength > 0))
	{
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}

	/* PlayLength Default */
	if (playLength == 0)
	{
		if (voice->src.format->wFormatTag == FAUDIO_FORMAT_MSADPCM)
		{
			FAudioADPCMWaveFormat *fmtex = (FAudioADPCMWaveFormat*) voice->src.format;
			playLength = (
				pBuffer->AudioBytes /
				fmtex->wfx.nBlockAlign *
				fmtex->wSamplesPerBlock
			) - playBegin;
		}
		else if (pBufferWMA != NULL)
		{
			playLength = (
				pBufferWMA->pDecodedPacketCumulativeBytes[pBufferWMA->PacketCount - 1] /
				(voice->src.format->nChannels * voice->src.format->wBitsPerSample / 8)
			) - playBegin;
		}
		else
		{
			playLength = (
				pBuffer->AudioBytes /
				voice->src.format->nBlockAlign
			) - playBegin;
		}
	}

	if (pBuffer->LoopCount > 0)
	{
		/* "The value of LoopBegin must be less than PlayBegin + PlayLength" */
		if (loopBegin >= (playBegin + playLength))
		{
			LOG_API_EXIT(voice->audio)
			return FAUDIO_E_INVALID_CALL;
		}

		/* LoopLength Default */
		if (loopLength == 0)
		{
			loopLength = playBegin + playLength - loopBegin;
		}

		/* "The value of LoopBegin + LoopLength must be greater than PlayBegin
		 * and less than PlayBegin + PlayLength"
		 */
		if (	voice->audio->version > 7 && (
			(loopBegin + loopLength) <= playBegin ||
			(loopBegin + loopLength) > (playBegin + playLength))	)
		{
			LOG_API_EXIT(voice->audio)
			return FAUDIO_E_INVALID_CALL;
		}
	}

	/* For ADPCM, round down to the nearest sample block size */
	if (voice->src.format->wFormatTag == FAUDIO_FORMAT_MSADPCM)
	{
		adpcmMask = ((FAudioADPCMWaveFormat*) voice->src.format)->wSamplesPerBlock;
		playBegin -= playBegin % adpcmMask;
		playLength -= playLength % adpcmMask;
		loopBegin -= loopBegin % adpcmMask;
		loopLength -= loopLength % adpcmMask;

		/* This is basically a const_cast... */
		adpcmByteCount = (uint32_t*) &pBuffer->AudioBytes;
		*adpcmByteCount = (
			pBuffer->AudioBytes / voice->src.format->nBlockAlign
		) * voice->src.format->nBlockAlign;
	}

	/* Allocate, now that we have valid input */
	entry = (FAudioBufferEntry*) voice->audio->pMalloc(sizeof(FAudioBufferEntry));
	FAudio_memcpy(&entry->buffer, pBuffer, sizeof(FAudioBuffer));
	entry->buffer.PlayBegin = playBegin;
	entry->buffer.PlayLength = playLength;
	entry->buffer.LoopBegin = loopBegin;
	entry->buffer.LoopLength = loopLength;
	if (pBufferWMA != NULL)
	{
		FAudio_memcpy(&entry->bufferWMA, pBufferWMA, sizeof(FAudioBufferWMA));
	}
	entry->next = NULL;

	if (	voice->audio->version <= 7 && (
		entry->buffer.LoopCount > 0 &&
		entry->buffer.LoopBegin + entry->buffer.LoopLength <= entry->buffer.PlayBegin))
	{
		entry->buffer.LoopCount = 0;
	}

	/* Submit! */
	FAudio_PlatformLockMutex(voice->src.bufferLock);
	LOG_MUTEX_LOCK(voice->audio, voice->src.bufferLock)
	if (voice->src.bufferList == NULL)
	{
		voice->src.bufferList = entry;
		voice->src.curBufferOffset = entry->buffer.PlayBegin;
		voice->src.newBuffer = 1;
	}
	else
	{
		list = voice->src.bufferList;
		while (list->next != NULL)
		{
			list = list->next;
		}
		list->next = entry;

		/* For some bizarre reason we get scenarios where a buffer is freed, only to
		 * have the allocator give us the exact same address and somehow get a single
		 * buffer referencing itself. I don't even know.
		 */
		FAudio_assert(list != entry);
	}
	LOG_INFO(
		voice->audio,
		"%p: appended buffer %p",
		(void*) voice,
		(void*) &entry->buffer
	)
	FAudio_PlatformUnlockMutex(voice->src.bufferLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioSourceVoice_FlushSourceBuffers(
	FAudioSourceVoice *voice
) {
	FAudioBufferEntry *entry, *next;

	LOG_API_ENTER(voice->audio)
	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	FAudio_PlatformLockMutex(voice->src.bufferLock);
	LOG_MUTEX_LOCK(voice->audio, voice->src.bufferLock)

	/* If the source is playing, don't flush the active buffer */
	entry = voice->src.bufferList;
	if ((voice->src.active == 1) && entry != NULL && !voice->src.newBuffer)
	{
		entry = entry->next;
		voice->src.bufferList->next = NULL;
	}
	else
	{
		voice->src.curBufferOffset = 0;
		voice->src.bufferList = NULL;
		voice->src.newBuffer = 0;
	}

	/* Go through each buffer, send an event for each one before deleting */
	while (entry != NULL)
	{
		if (voice->src.callback != NULL && voice->src.callback->OnBufferEnd != NULL)
		{
			voice->src.callback->OnBufferEnd(
				voice->src.callback,
				entry->buffer.pContext
			);
		}
		next = entry->next;
		voice->audio->pFree(entry);
		entry = next;
	}

	FAudio_PlatformUnlockMutex(voice->src.bufferLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioSourceVoice_Discontinuity(
	FAudioSourceVoice *voice
) {
	FAudioBufferEntry *buf;

	LOG_API_ENTER(voice->audio)
	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	FAudio_PlatformLockMutex(voice->src.bufferLock);
	LOG_MUTEX_LOCK(voice->audio, voice->src.bufferLock)

	if (voice->src.bufferList != NULL)
	{
		for (buf = voice->src.bufferList; buf->next != NULL; buf = buf->next);
		buf->buffer.Flags |= FAUDIO_END_OF_STREAM;
	}

	FAudio_PlatformUnlockMutex(voice->src.bufferLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

uint32_t FAudioSourceVoice_ExitLoop(
	FAudioSourceVoice *voice,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueExitLoop(
			voice,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	FAudio_PlatformLockMutex(voice->src.bufferLock);
	LOG_MUTEX_LOCK(voice->audio, voice->src.bufferLock)

	if (voice->src.bufferList != NULL)
	{
		voice->src.bufferList->buffer.LoopCount = 0;
	}

	FAudio_PlatformUnlockMutex(voice->src.bufferLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioSourceVoice_GetState(
	FAudioSourceVoice *voice,
	FAudioVoiceState *pVoiceState,
	uint32_t Flags
) {
	FAudioBufferEntry *entry;

	LOG_API_ENTER(voice->audio)
	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	FAudio_PlatformLockMutex(voice->src.bufferLock);
	LOG_MUTEX_LOCK(voice->audio, voice->src.bufferLock)

	if (!(Flags & FAUDIO_VOICE_NOSAMPLESPLAYED))
	{
		pVoiceState->SamplesPlayed = voice->src.totalSamples;
	}

	pVoiceState->BuffersQueued = 0;
	pVoiceState->pCurrentBufferContext = NULL;
	if (voice->src.bufferList != NULL)
	{
		entry = voice->src.bufferList;
		if (!voice->src.newBuffer)
		{
			pVoiceState->pCurrentBufferContext = entry->buffer.pContext;
		}
		do
		{
			pVoiceState->BuffersQueued += 1;
			entry = entry->next;
		} while (entry != NULL);
	}

	LOG_INFO(
		voice->audio,
		"-> {pCurrentBufferContext: %p, BuffersQueued: %u, SamplesPlayed: %"FAudio_PRIu64"}",
		pVoiceState->pCurrentBufferContext, pVoiceState->BuffersQueued,
		pVoiceState->SamplesPlayed
	)

	FAudio_PlatformUnlockMutex(voice->src.bufferLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioSourceVoice_SetFrequencyRatio(
	FAudioSourceVoice *voice,
	float Ratio,
	uint32_t OperationSet
) {
	LOG_API_ENTER(voice->audio)

	if (OperationSet != FAUDIO_COMMIT_NOW && voice->audio->active)
	{
		FAudio_OPERATIONSET_QueueSetFrequencyRatio(
			voice,
			Ratio,
			OperationSet
		);
		LOG_API_EXIT(voice->audio)
		return 0;
	}
	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	if (voice->flags & FAUDIO_VOICE_NOPITCH)
	{
		LOG_API_EXIT(voice->audio)
		return 0;
	}

	voice->src.freqRatio = FAudio_clamp(
		Ratio,
		FAUDIO_MIN_FREQ_RATIO,
		voice->src.maxFreqRatio
	);
	LOG_API_EXIT(voice->audio)
	return 0;
}

void FAudioSourceVoice_GetFrequencyRatio(
	FAudioSourceVoice *voice,
	float *pRatio
) {
	LOG_API_ENTER(voice->audio)
	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);

	*pRatio = voice->src.freqRatio;
	LOG_API_EXIT(voice->audio)
}

uint32_t FAudioSourceVoice_SetSourceSampleRate(
	FAudioSourceVoice *voice,
	uint32_t NewSourceSampleRate
) {
	uint32_t outSampleRate;
	uint32_t newDecodeSamples, newResampleSamples;

	LOG_API_ENTER(voice->audio)
	FAudio_assert(voice->type == FAUDIO_VOICE_SOURCE);
	FAudio_assert(	NewSourceSampleRate >= FAUDIO_MIN_SAMPLE_RATE &&
			NewSourceSampleRate <= FAUDIO_MAX_SAMPLE_RATE	);

	FAudio_PlatformLockMutex(voice->src.bufferLock);
	LOG_MUTEX_LOCK(voice->audio, voice->src.bufferLock)
	if (	voice->audio->version > 7 &&
		voice->src.bufferList != NULL	)
	{
		FAudio_PlatformUnlockMutex(voice->src.bufferLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)
		LOG_API_EXIT(voice->audio)
		return FAUDIO_E_INVALID_CALL;
	}
	FAudio_PlatformUnlockMutex(voice->src.bufferLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->src.bufferLock)

	voice->src.format->nSamplesPerSec = NewSourceSampleRate;

	/* Resize decode cache */
	newDecodeSamples = (uint32_t) FAudio_ceil(
		voice->audio->updateSize *
		(double) voice->src.maxFreqRatio *
		(double) NewSourceSampleRate /
		(double) voice->audio->master->master.inputSampleRate
	) + EXTRA_DECODE_PADDING * voice->src.format->nChannels;
	FAudio_INTERNAL_ResizeDecodeCache(
		voice->audio,
		(newDecodeSamples + EXTRA_DECODE_PADDING) * voice->src.format->nChannels
	);
	voice->src.decodeSamples = newDecodeSamples;

	FAudio_PlatformLockMutex(voice->sendLock);
	LOG_MUTEX_LOCK(voice->audio, voice->sendLock)

	if (voice->sends.SendCount == 0)
	{
		FAudio_PlatformUnlockMutex(voice->sendLock);
		LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)
		LOG_API_EXIT(voice->audio)
		return 0;
	}
	outSampleRate = voice->sends.pSends[0].pOutputVoice->type == FAUDIO_VOICE_MASTER ?
		voice->sends.pSends[0].pOutputVoice->master.inputSampleRate :
		voice->sends.pSends[0].pOutputVoice->mix.inputSampleRate;

	FAudio_PlatformUnlockMutex(voice->sendLock);
	LOG_MUTEX_UNLOCK(voice->audio, voice->sendLock)

	/* Resize resample cache */
	newResampleSamples = (uint32_t) (FAudio_ceil(
		(double) voice->audio->updateSize *
		(double) outSampleRate /
		(double) voice->audio->master->master.inputSampleRate
	));
	FAudio_INTERNAL_ResizeResampleCache(
		voice->audio,
		newResampleSamples * voice->src.format->nChannels
	);
	voice->src.resampleSamples = newResampleSamples;
	LOG_API_EXIT(voice->audio)
	return 0;
}

/* FAudioMasteringVoice Interface */

FAUDIOAPI uint32_t FAudioMasteringVoice_GetChannelMask(
	FAudioMasteringVoice *voice,
	uint32_t *pChannelMask
) {
	LOG_API_ENTER(voice->audio)
	FAudio_assert(voice->type == FAUDIO_VOICE_MASTER);
	FAudio_assert(pChannelMask != NULL);

	*pChannelMask = voice->audio->mixFormat.dwChannelMask;
	LOG_API_EXIT(voice->audio)
	return 0;
}

/* vim: set noexpandtab shiftwidth=8 tabstop=8: */
