// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#ifndef PUREPHONE_LINUXCELLULARAUDIO_HPP
#define PUREPHONE_LINUXCELLULARAUDIO_HPP

#include "bsp/audio/bsp_audio.hpp"
#include "portaudio.h"

namespace bsp
{

    class LinuxCellularAudio : public AudioDevice
    {
      public:
        LinuxCellularAudio(AudioDevice::audioCallback_t callback);

        virtual ~LinuxCellularAudio();

        AudioDevice::RetCode Start(const Format &format) override final;

        AudioDevice::RetCode Stop() override final;

        AudioDevice::RetCode OutputVolumeCtrl(float vol) override final;

        AudioDevice::RetCode InputGainCtrl(float gain) override final;

        AudioDevice::RetCode OutputPathCtrl(OutputPath outputPath) override final;

        AudioDevice::RetCode InputPathCtrl(InputPath inputPath) override final;

        bool IsFormatSupported(const Format &format) override final;

        void onDataReceive() final;
        void onDataSend() final;
        void enableInput() final;
        void enableOutput() final;
        void disableInput() final;
        void disableOutput() final;

      private:
        PaStream *stream;

        static int portAudioCallback(const void *inputBuffer,
                                     void *outputBuffer,
                                     unsigned long framesPerBuffer,
                                     const PaStreamCallbackTimeInfo *timeInfo,
                                     PaStreamCallbackFlags statusFlags,
                                     void *userData);

        bool TryOpenStream(const bsp::AudioDevice::Format &format);
    };

} // namespace bsp

#endif // PUREPHONE_LINUXCELLULARAUDIO_HPP
