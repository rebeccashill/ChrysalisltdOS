// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include <widgets/BellSideListItem.hpp>

namespace gui
{
    class BellSideListItemWithCallbacks : public BellSideListItem
    {
      public:
        explicit BellSideListItemWithCallbacks(
            const std::string &description, BellBaseLayout::LayoutType type = BellBaseLayout::LayoutType::WithArrows);
        /// Fetch value from the list item and perform custom action.
        std::function<void()> getValue;
        /// Set list item's value and perform custom action.
        std::function<void()> setValue;
        std::function<void()> onEnter;
        std::function<void()> onExit;

        std::function<bool()> onProceed;
        std::function<bool()> onReturn;

      protected:
        void OnFocusChangedCallback();
        bool OnInputCallback(const InputEvent &inputEvent);
    };
} // namespace gui
