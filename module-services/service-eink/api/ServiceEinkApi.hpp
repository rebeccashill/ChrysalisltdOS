// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

namespace service::eink::api
{
    [[nodiscard]] auto isInvertedModeEnabled() noexcept -> bool;
} // namespace service::eink::api
