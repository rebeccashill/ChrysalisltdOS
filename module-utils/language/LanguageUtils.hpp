// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include <string>

namespace utils::language
{
    auto getCorrectMinutesNumeralForm(std::uint32_t val) -> std::string;
    auto getCorrectSecondsNumeralForm(std::uint32_t val) -> std::string;

    auto getCorrectMinutesAccusativeForm(std::uint32_t val) -> std::string;

    auto getCorrectMultiplicityForm(std::uint32_t val) -> std::string;
} // namespace utils::language
