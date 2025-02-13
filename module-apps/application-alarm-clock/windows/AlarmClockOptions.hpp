// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include "application-alarm-clock/ApplicationAlarmClock.hpp"
#include "application-alarm-clock/models/AlarmsRepository.hpp"
#include <module-db/Interface/AlarmEventRecord.hpp>

namespace gui
{
    class Option; // Fw declaration
    class Text;   // Fw declaration
} // namespace gui

namespace app::alarmClock
{
    std::list<gui::Option> alarmsListOptions(ApplicationCommon *application,
                                             const AlarmEventRecord &record,
                                             AbstractAlarmsRepository &alarmsRepository);
}
