// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include <common/models/AbstractRelaxationFadeModel.hpp>
#include <common/widgets/BellSideListItemWithCallbacks.hpp>
#include <apps-common/InternalModel.hpp>

namespace app::bell_settings
{
    class RelaxationListItemProvider : public app::InternalModel<gui::BellSideListItemWithCallbacks *>,
                                       public gui::ListItemProvider
    {
      public:
        explicit RelaxationListItemProvider(std::unique_ptr<AbstractRelaxationFadeModel> &&model);
        auto getListItems() -> std::vector<gui::BellSideListItemWithCallbacks *>;
        auto requestRecords(std::uint32_t offset, std::uint32_t limit) -> void override;
        [[nodiscard]] auto getItem(gui::Order order) -> gui::ListItem * override;
        [[nodiscard]] auto requestRecordsCount() -> unsigned int override;
        [[nodiscard]] auto getMinimalItemSpaceRequired() const -> unsigned int override;

        std::function<void()> onExit;

      private:
        auto buildListItems() -> void;

        std::unique_ptr<AbstractRelaxationFadeModel> model;
    };
} // namespace app::bell_settings
