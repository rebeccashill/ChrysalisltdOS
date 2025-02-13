// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include "service-bluetooth/BluetoothMessage.hpp"

namespace message::bluetooth
{
    class ResponseVisibleDevices : public BluetoothMessage
    {
      public:
        explicit ResponseVisibleDevices(std::vector<Devicei> devices) : devices(std::move(devices))
        {}
        [[nodiscard]] auto getDevices() const noexcept -> std::vector<Devicei>
        {
            return devices;
        }

      private:
        std::vector<Devicei> devices;
    };
} // namespace message::bluetooth
