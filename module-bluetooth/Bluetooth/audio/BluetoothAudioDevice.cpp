// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include "BluetoothAudioDevice.hpp"

#include <interface/profiles/A2DP/AVDTP.hpp>
#include <interface/profiles/A2DP/AVRCP.hpp>
#include <interface/profiles/SCO/SCO.hpp>
#include <interface/profiles/SCO/ScoUtils.hpp>

#include <Audio/AudioCommon.hpp>
#include <Audio/VolumeScaler.hpp>

#include <Utils.hpp>
#include <ticks.hpp>

#include <chrono>
#include <cassert>

using audio::AudioFormat;
using bluetooth::A2DPAudioDevice;
using bluetooth::BluetoothAudioDevice;
using bluetooth::CVSDAudioDevice;

using namespace std::chrono_literals;

BluetoothAudioDevice::BluetoothAudioDevice(AudioProfile audioProfile) : profile(audioProfile)
{
    LOG_DEBUG("Creating bluetooth device with %s profile", magic_enum::enum_name(audioProfile).data());
}

BluetoothAudioDevice::~BluetoothAudioDevice()
{
    LOG_DEBUG("Destroying bluetooth audio device");
}

auto BluetoothAudioDevice::getProfileType() const -> AudioProfile
{
    return profile;
}

auto BluetoothAudioDevice::setInputGain(float gain) -> audio::AudioDevice::RetCode
{
    return audio::AudioDevice::RetCode::Success;
}

auto BluetoothAudioDevice::isInputEnabled() const -> bool
{
    return inputEnabled;
}

auto BluetoothAudioDevice::isOutputEnabled() const -> bool
{
    return outputEnabled;
}

auto A2DPAudioDevice::setOutputVolume(float vol) -> audio::AudioDevice::RetCode
{
    outputVolume = vol;
    return audio::AudioDevice::RetCode::Success;
}

void A2DPAudioDevice::onDataSend()
{
    if (isOutputEnabled()) {
        fillSbcAudioBuffer();
    }
}

void A2DPAudioDevice::onDataReceive()
{}

auto CVSDAudioDevice::setOutputVolume(float vol) -> audio::AudioDevice::RetCode
{
    LOG_DEBUG("Setting CVSD volume: %f", vol);
    if (getProfileType() == AudioProfile::HSP) {
        const auto volumeToSet = audio::volume::scaler::hsp::toHSPGain(vol);
        hsp_ag_set_speaker_gain(volumeToSet);
        LOG_DEBUG("Volume to be set %d", volumeToSet);
    }
    else {
        const auto volumeToSet = audio::volume::scaler::hfp::toHFPGain(vol);
        hfp_ag_set_speaker_gain(aclHandle, volumeToSet);
        LOG_DEBUG("Volume to be set %d", volumeToSet);
    }

    outputVolume = vol;
    return audio::AudioDevice::RetCode::Success;
}

void CVSDAudioDevice::onDataSend()
{}

void CVSDAudioDevice::onDataSend(std::uint16_t scoHandle)
{
    if (!isOutputEnabled()) {
        bluetooth::sco::utils::sendZeros(scoHandle);
        return;
    }

    hci_reserve_packet_buffer();
    auto scoPacket = hci_get_outgoing_packet_buffer();

    // get data to send
    audio::AbstractStream::Span dataSpan;
    Sink::_stream->peek(dataSpan);

    // prepare packet to send
    std::copy(dataSpan.data, dataSpan.dataEnd(), &scoPacket[packetDataOffset]);
    Sink::_stream->consume();
    little_endian_store_16(scoPacket, packetHandleOffset, scoHandle);
    scoPacket[packetLengthOffset] = dataSpan.dataSize;

    // send packet
    hci_send_sco_packet_buffer(dataSpan.dataSize + packetDataOffset);
    hci_request_sco_can_send_now_event();
}

void CVSDAudioDevice::receiveCVSD(audio::AbstractStream::Span receivedData)
{
    if (!isInputEnabled()) {
        return;
    }

    auto blockSize          = Source::_stream->getInputTraits().blockSize;
    auto decodedData        = decodeCVSD(receivedData);
    auto processedDataIndex = 0;

    // try to complete leftovers to the full block size
    if (leftoversSize != 0) {
        auto maxFillSize = blockSize - leftoversSize;
        auto fillSize    = std::min(maxFillSize, decodedData.dataSize);

        std::copy_n(decodedData.data, fillSize, &rxLeftovers[leftoversSize]);

        if (fillSize + leftoversSize < blockSize) {
            leftoversSize += fillSize;
            return;
        }

        Source::_stream->push(&rxLeftovers[0], blockSize);
        leftoversSize      = 0;
        processedDataIndex = fillSize;
    }

    // push as many blocks as possible
    while (decodedData.dataSize - processedDataIndex >= blockSize) {
        Source::_stream->push(&decodedData.data[processedDataIndex], blockSize);
        processedDataIndex += blockSize;
    }

    // save leftovers
    leftoversSize = decodedData.dataSize - processedDataIndex;
    if (leftoversSize > 0) {
        std::copy_n(&decodedData.data[processedDataIndex], leftoversSize, &rxLeftovers[0]);
    }
}

auto CVSDAudioDevice::decodeCVSD(audio::AbstractStream::Span dataToDecode) -> audio::AbstractStream::Span
{
    auto decodedData = dataToDecode;
    std::array<std::int16_t, scratchBufferSize> scratchBuffer{};

    const auto audioBytesRead = dataToDecode.dataSize - packetDataOffset;
    const auto samplesCount   = audioBytesRead / sizeof(std::int16_t);
    auto dataStart            = &dataToDecode.data[packetDataOffset];

    for (auto i = 0; i < samplesCount; ++i) {
        scratchBuffer[i] = little_endian_read_16(dataStart, i * sizeof(std::uint16_t));
    }

    auto packetStatusByte = dataToDecode.data[packetStatusOffset];
    auto isBadFrame       = (packetStatusByte & allGoodMask) != 0;

    btstack_cvsd_plc_process_data(&cvsdPlcState, isBadFrame, &scratchBuffer[0], samplesCount, &decoderBuffer[0]);

    decodedData.data     = reinterpret_cast<std::uint8_t *>(decoderBuffer.get());
    decodedData.dataSize = audioBytesRead;

    return decodedData;
}

void CVSDAudioDevice::onDataReceive()
{}

void BluetoothAudioDevice::enableInput()
{
    inputEnabled = true;
}

void BluetoothAudioDevice::enableOutput()
{
    LOG_DEBUG("Enabling bluetooth audio output.");
    outputEnabled = true;
}

void BluetoothAudioDevice::disableInput()
{
    LOG_DEBUG("Disabling bluetooth audio input.");
    inputEnabled = false;
}

void BluetoothAudioDevice::disableOutput()
{
    LOG_DEBUG("Disabling bluetooth audio output.");
    outputEnabled = false;
}

void CVSDAudioDevice::enableInput()
{
    LOG_DEBUG("Enabling CVSD bluetooth audio input.");
    auto blockSize = Source::_stream->getInputTraits().blockSize;
    rxLeftovers    = std::make_unique<std::uint8_t[]>(blockSize);
    decoderBuffer  = std::make_unique<std::int16_t[]>(scratchBufferSize);
    BluetoothAudioDevice::enableInput();
}

auto BluetoothAudioDevice::fillSbcAudioBuffer() -> int
{
    int totalNumBytesRead                    = 0;
    const auto audioSamplesPerSbcBuffer      = btstack_sbc_encoder_num_audio_frames();
    const auto context                       = &AVRCP::mediaTracker;

    while (
        (context->samples_ready >= audioSamplesPerSbcBuffer) &&
        ((context->max_media_payload_size - context->sbc_storage_count) >= btstack_sbc_encoder_sbc_buffer_length())) {
        audio::Stream::Span dataSpan;
        Sink::_stream->peek(dataSpan);

        scaleVolume(dataSpan);
        btstack_sbc_encoder_process_data(reinterpret_cast<std::int16_t *>(dataSpan.data));
        Sink::_stream->consume();

        const auto sbcFrameSize = btstack_sbc_encoder_sbc_buffer_length();
        const auto sbcFrame     = btstack_sbc_encoder_sbc_buffer();

        totalNumBytesRead += audioSamplesPerSbcBuffer;
        std::memcpy(&context->sbc_storage[context->sbc_storage_count], sbcFrame, sbcFrameSize);
        context->sbc_storage_count += sbcFrameSize;
        context->samples_ready -= audioSamplesPerSbcBuffer;
    }

    return totalNumBytesRead;
}

auto BluetoothAudioDevice::scaleVolume(audio::Stream::Span &dataSpan) const -> void
{
    constexpr auto bytesPerSample =
        sizeof(std::int16_t) / sizeof(std::uint8_t); // Samples are signed 16-bit, but stored in uint8_t array
    constexpr auto volumeLevels   = (audio::maxVolume - audio::minVolume) + 1;
    constexpr auto volumeScaleLut = utils::makeArray<float, volumeLevels>([](auto index) {
        /* Return zero when muted */
        if (index == 0) {
            return 0.0f;
        }

        /* Normalize volume to <0;1> range */
        const auto volumeNormalized = static_cast<float>(index) / audio::maxVolume;

        /* Coefficients for a curve with a dynamic range of 42dB
         * For 10 available steps this gives ~4dB change between each step, what seems
         * to be a fair compromise between step size and volume at the lowest level.
         * For more info check: https://www.dr-lex.be/info-stuff/volumecontrols.html */
        constexpr auto a = 7.943e-3f;
        constexpr auto b = 4.835f;
        return std::clamp(a * std::exp(b * volumeNormalized), 0.0f, 1.0f);
    });

    auto firstSample = reinterpret_cast<std::int16_t *>(dataSpan.data);
    auto lastSample  = &firstSample[dataSpan.dataSize / bytesPerSample];

    /* Scale each sample to reduce volume */
    const auto lutIndex = static_cast<std::size_t>(outputVolume);
    std::for_each(firstSample, lastSample, [&](std::int16_t &sample) { sample *= volumeScaleLut[lutIndex]; });
}

auto A2DPAudioDevice::getSupportedFormats() -> std::vector<audio::AudioFormat>
{
    constexpr auto supportedBitWidth = 16U;
    return std::vector<AudioFormat>{AudioFormat{static_cast<unsigned>(AVDTP::sbcConfig.samplingFrequency),
                                                supportedBitWidth,
                                                static_cast<unsigned>(AVDTP::sbcConfig.numChannels)}};
}

auto CVSDAudioDevice::getSupportedFormats() -> std::vector<audio::AudioFormat>
{
    return std::vector<AudioFormat>{getSourceFormat()};
}

auto A2DPAudioDevice::getTraits() const -> ::audio::Endpoint::Traits
{
    return Traits{.usesDMA = false, .blockSizeConstraint = 512U, .timeConstraint = 10ms};
}

auto CVSDAudioDevice::getTraits() const -> ::audio::Endpoint::Traits
{
    return Traits{.usesDMA = false, .blockSizeConstraint = 128U, .timeConstraint = 16ms};
}

auto A2DPAudioDevice::getSourceFormat() -> ::audio::AudioFormat
{
    return audio::nullFormat;
}

auto CVSDAudioDevice::getSourceFormat() -> ::audio::AudioFormat
{
    return AudioFormat{bluetooth::SCO::CVSD_SAMPLE_RATE, supportedBitWidth, supportedChannels};
}

void CVSDAudioDevice::setAclHandle(hci_con_handle_t handle)
{
    aclHandle = handle;
}

audio::AudioDevice::RetCode A2DPAudioDevice::waitUntilStreamPaused() const
{
    /* Wait until A2DP stream is paused before proceeding. This is hacky way to prevent
     * race condition when music player track automatically changes to the next one.
     * a2dp_source functions are asynchronous, so in such case sometimes play request is
     * sent before pause request has been fully processed, what results in A2DP stream
     * not being restarted. */
    constexpr auto timeoutMs    = 200;
    constexpr auto retryDelayMs = 10;

    auto timeoutLoops = timeoutMs / retryDelayMs;
    while ((AVRCP::playInfo.status == AVRCP_PLAYBACK_STATUS_PLAYING) && (timeoutLoops > 0)) {
        vTaskDelay(cpp_freertos::Ticks::MsToTicks(retryDelayMs));
        timeoutLoops--;
    }

    return (timeoutLoops == 0) ? audio::AudioDevice::RetCode::Failure : audio::AudioDevice::RetCode::Success;
}

audio::AudioDevice::RetCode A2DPAudioDevice::Start()
{
    const auto retCode = a2dp_source_start_stream(AVRCP::mediaTracker.a2dp_cid, AVRCP::mediaTracker.local_seid);
    switch (retCode) {
    case ERROR_CODE_SUCCESS:
        return audio::AudioDevice::RetCode::Success;
    case ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER:
        return audio::AudioDevice::RetCode::Disconnected;
    default:
        return audio::AudioDevice::RetCode::Failure;
    }
}

audio::AudioDevice::RetCode A2DPAudioDevice::Stop()
{
    const auto retCode = a2dp_source_pause_stream(AVRCP::mediaTracker.a2dp_cid, AVRCP::mediaTracker.local_seid);
    switch (retCode) {
    case ERROR_CODE_SUCCESS:
        return waitUntilStreamPaused();
    case ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER:
        return audio::AudioDevice::RetCode::Disconnected; // Device disconnected during playback, e.g. headphones
                                                          // discharged
    default:
        return audio::AudioDevice::RetCode::Failure;
    }
}

audio::AudioDevice::RetCode A2DPAudioDevice::Resume()
{
    return Start();
}

audio::AudioDevice::RetCode A2DPAudioDevice::Pause()
{
    return Stop();
}
