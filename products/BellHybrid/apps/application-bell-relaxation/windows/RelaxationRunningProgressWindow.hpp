// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#pragma once

#include "presenter/RelaxationRunningProgressPresenter.hpp"
#include <data/RelaxationAudioData.hpp>

#include <AppWindow.hpp>
#include <apps-common/widgets/BarGraph.hpp>
#include <apps-common/widgets/TimeMinuteSecondWidget.hpp>
#include <common/widgets/BellStatusClock.hpp>
#include <gui/widgets/Icon.hpp>

namespace gui
{
    class RelaxationRunningProgressWindow : public AppWindow,
                                            public app::relaxation::RelaxationRunningProgressContract::View
    {
      public:
        RelaxationRunningProgressWindow(
            app::ApplicationCommon *app,
            std::unique_ptr<app::relaxation::RelaxationRunningProgressContract::Presenter> &&presenter);

      private:
        std::unique_ptr<app::relaxation::RelaxationRunningProgressContract::Presenter> presenter;
        std::unique_ptr<RelaxationAudioContext> audioContext;

        VBox *mainVBox{nullptr};
        ArcProgressBar *progress{nullptr};
        TimeMinuteSecondWidget *timer{nullptr};
        TextFixedSize *bottomDescription{nullptr};
        VBox *pauseBox{nullptr};
        BellStatusClock *clock{nullptr};

        void setTime(std::time_t newTime) override;
        void setTimeFormat(utils::time::Locale::TimeFormat fmt) override;
        void onBeforeShow(ShowMode mode, SwitchData *data) override;
        bool onInput(const InputEvent &inputEvent) override;
        void buildInterface() override;
        void onFinished() override;
        void onPaused() override;
        void resume() override;
        RefreshModes updateTime() override;
        void handleError() override;
        void handleDeletedFile() override;

        void buildLayout();
        void configureTimer();
    };
} // namespace gui
