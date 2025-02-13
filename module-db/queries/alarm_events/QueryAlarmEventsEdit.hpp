// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include "Interface/AlarmEventRecord.hpp"

#include <Common/Query.hpp>

#include <string>

namespace db::query::alarmEvents
{
    class Edit : public Query
    {
        const AlarmEventRecord record;

      public:
        explicit Edit(const AlarmEventRecord &record);

        [[nodiscard]] auto getRecord() const -> AlarmEventRecord;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

    class EditResult : public QueryResult
    {
        const bool ret = true;

      public:
        explicit EditResult(bool ret);
        [[nodiscard]] auto getResult() const -> bool;
        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

} // namespace db::query::alarmEvents
