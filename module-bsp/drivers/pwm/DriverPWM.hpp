// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include <memory>
#include <cstdint>
#include <bsp/common.hpp>

namespace drivers
{
    enum class PWMInstances
    {
        OFFSET = 1,
        PWM_1  = OFFSET,
        PWM_2,
        PWM_3,
        PWM_4,
        COUNT = PWM_4
    };

    enum class PWMModules
    {
        MODULE0 = 0,
        MODULE1,
        MODULE2,
        MODULE3,
        COUNT
    };

    enum class PWMChannel
    {
        A = 0,
        B,
        X
    };

    struct DriverPWMParams
    {
        PWMChannel channel;
        std::uint32_t outputFrequency;
    };

    class DriverPWM
    {
      public:
        static std::shared_ptr<DriverPWM> Create(PWMInstances inst, PWMModules mod, const DriverPWMParams &params);

        DriverPWM(PWMInstances inst, PWMModules mod, const DriverPWMParams &params)
            : instance(inst), module(mod), parameters(params)
        {}

        virtual ~DriverPWM() = default;

        virtual void InitNextChannel(const DriverPWMParams &params) = 0;

        // Duty cycle in percent: 0 - 100
        virtual void SetDutyCycle(float dutyCyclePercent, PWMChannel channel) = 0;

        virtual void Start(PWMChannel channel) = 0;

        virtual void Stop(PWMChannel channel) = 0;

        virtual void UpdateClockFrequency() = 0;

      protected:
        PWMInstances instance;
        PWMModules module;
        const DriverPWMParams parameters;

      private:
        static std::weak_ptr<DriverPWM> pwmDrivers[static_cast<std::uint32_t>(PWMInstances::COUNT)]
                                                  [static_cast<std::uint32_t>(PWMModules::COUNT)];
    };

} // namespace drivers
