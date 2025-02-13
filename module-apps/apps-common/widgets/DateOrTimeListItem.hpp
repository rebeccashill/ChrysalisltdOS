// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include <time/FromTillDate.hpp>
#include <module-gui/gui/widgets/ListItem.hpp>

namespace gui
{
    using DateOrTimeListItem = ListItemWithCallbacks<utils::time::FromTillDate>;
} /* namespace gui */
