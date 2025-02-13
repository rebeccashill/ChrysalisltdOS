// Copyright (c) 2017-2025, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include "RelaxationRunningProgressWindow.hpp"
#include <data/RelaxationStyle.hpp>
#include <data/RelaxationSwitchData.hpp>
#include <common/BellCommonNames.hpp>
#include <popups/data/AudioErrorParams.hpp>
#include <ApplicationBellRelaxation.hpp>
#include <apps-common/widgets/BellBaseLayout.hpp>
#include <apps-common/widgets/ProgressTimerWithBarGraphAndCounter.hpp>

namespace
{
    constexpr auto relaxationProgressTimerName{"RelaxationProgressTimer"};
    constexpr std::chrono::seconds initialInterval{1};
    constexpr auto relaxationProgressMode{app::ProgressCountdownMode::Increasing};
} // namespace

namespace gui
{
    RelaxationRunningProgressWindow::RelaxationRunningProgressWindow(
        app::ApplicationCommon *app,
        std::unique_ptr<app::relaxation::RelaxationRunningProgressContract::Presenter> &&presenter)
        : AppWindow(app, gui::window::name::relaxationRunningProgress), presenter{std::move(presenter)}
    {
        this->presenter->attach(this);
        buildInterface();
    }

    void RelaxationRunningProgressWindow::buildInterface()
    {
        AppWindow::buildInterface();

        statusBar->setVisible(false);
        header->setTitleVisibility(false);
        navBar->setVisible(false);

        buildLayout();
        configureTimer();
    }

    void RelaxationRunningProgressWindow::buildLayout()
    {
        using namespace gui::relaxationStyle;

        const auto progressArcRadius = relStyle::progressTime::radius;
        const auto progressArcWidth  = relStyle::progressTime::penWidth;
        const auto arcStartAngle     = -90 - relStyle::progressTime::verticalDeviationDegrees;
        const auto arcSweepAngle     = 360 - (2 * relStyle::progressTime::verticalDeviationDegrees);
        const auto arcProgressSteps  = 1000;

        Arc::ShapeParams arcParams;
        arcParams.setCenterPoint(Point(getWidth() / 2, getHeight() / 2))
            .setRadius(progressArcRadius)
            .setStartAngle(arcStartAngle)
            .setSweepAngle(arcSweepAngle)
            .setPenWidth(progressArcWidth)
            .setBorderColor(ColorFullBlack);

        progress = new ArcProgressBar(this,
                                      arcParams,
                                      ArcProgressBar::ProgressDirection::CounterClockwise,
                                      ArcProgressBar::ProgressChange::DecrementFromFull);
        progress->setMaximum(arcProgressSteps);

        mainVBox = new VBox(this, 0, 0, style::window_width, style::window_height);
        mainVBox->setEdges(rectangle_enums::RectangleEdge::None);

        clock = new BellStatusClock(mainVBox);
        clock->setMaximumSize(relStyle::clock::maxSizeX, relStyle::clock::maxSizeY);
        clock->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        clock->setMargins(Margins(0, relStyle::clock::marginTop, 0, 0));

        timer = new TimeMinuteSecondWidget(mainVBox,
                                           0,
                                           0,
                                           relStyle::timer::maxSizeX,
                                           relStyle::timer::maxSizeY,
                                           TimeMinuteSecondWidget::DisplayType::MinutesThenSeconds);
        timer->setMinimumSize(relStyle::timer::maxSizeX, relStyle::timer::maxSizeY);
        timer->setMargins(Margins(0, relStyle::timer::marginTop, 0, 0));
        timer->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));

        pauseBox = new VBox(mainVBox);
        pauseBox->setAlignment(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        pauseBox->setEdges(RectangleEdge::None);
        pauseBox->setMargins(Margins(0, relStyle::pauseIcon::marginTop, 0, 0));
        pauseBox->setMinimumSize(relStyle::pauseIcon::minSizeX, relStyle::pauseIcon::runningMinSizeY);
        new Image(pauseBox, relStyle::pauseIcon::image, ImageTypeSpecifier::W_G);
        pauseBox->setVisible(false);
        pauseBox->resizeItems();

        bottomDescription = new gui::TextFixedSize(mainVBox);
        bottomDescription->setMaximumSize(relStyle::bottomDescription::maxSizeX, relStyle::bottomDescription::maxSizeY);
        bottomDescription->setFont(relStyle::bottomDescription::font);
        bottomDescription->setMargins(gui::Margins(0, 0, 0, 0));
        bottomDescription->activeItem = false;
        bottomDescription->setAlignment(
            gui::Alignment(gui::Alignment::Horizontal::Center, gui::Alignment::Vertical::Top));
        bottomDescription->setText(utils::translate("app_bellmain_relaxation"));
        bottomDescription->drawUnderline(false);
        bottomDescription->setVisible(true);

        mainVBox->resizeItems();
    }

    void RelaxationRunningProgressWindow::onBeforeShow(ShowMode mode, SwitchData *data)
    {
        presenter->onBeforeShow();
        updateTime();

        if (data && typeid(*data) == typeid(RelaxationSwitchData)) {
            auto *audioSwitchData = static_cast<RelaxationSwitchData *>(data);
            audioContext          = audioSwitchData->getAudioContext();
            presenter->activate(audioContext->getSound());
        }
    }

    bool RelaxationRunningProgressWindow::onInput(const InputEvent &inputEvent)
    {
        if (inputEvent.isShortRelease()) {
            if (inputEvent.is(KeyCode::KEY_RF)) {
                presenter->stop();
                return true;
            }
            else if (inputEvent.is(KeyCode::KEY_ENTER)) {
                if (presenter->isTimerStopped()) {
                    presenter->resume();
                }
                else {
                    presenter->pause();
                }
                return true;
            }
            else if (inputEvent.is(KeyCode::KEY_DOWN) || inputEvent.is(KeyCode::KEY_UP)) {
                application->switchWindow(gui::popup::window::volume_window);
                return true;
            }
        }
        return AppWindow::onInput(inputEvent);
    }

    void RelaxationRunningProgressWindow::onFinished()
    {
        if (presenter->isTimerFinished()) {
            application->switchWindow(gui::window::name::relaxationEnded,
                                      std::make_unique<gui::RelaxationEndedSwitchData>());
        }
        else {
            application->returnToPreviousWindow();
        }
    }

    void RelaxationRunningProgressWindow::onPaused()
    {
        timer->setVisible(false);
        pauseBox->setVisible(true);
        mainVBox->resizeItems();
        application->refreshWindow(RefreshModes::GUI_REFRESH_DEEP);
    }

    void RelaxationRunningProgressWindow::resume()
    {
        timer->setVisible(true);
        pauseBox->setVisible(false);
        mainVBox->resizeItems();
        application->refreshWindow(RefreshModes::GUI_REFRESH_DEEP);
    }

    void RelaxationRunningProgressWindow::configureTimer()
    {
        auto progressTimer = std::make_unique<app::ProgressTimerWithBarGraphAndCounter>(
            application, *this, relaxationProgressTimerName, initialInterval, relaxationProgressMode);
        progressTimer->attach(progress);
        progressTimer->attach(timer);
        presenter->setTimer(std::move(progressTimer));
    }

    void RelaxationRunningProgressWindow::setTime(std::time_t newTime)
    {
        clock->setTime(newTime);
        clock->setTimeFormatSpinnerVisibility(true);
    }

    void RelaxationRunningProgressWindow::setTimeFormat(utils::time::Locale::TimeFormat fmt)
    {
        clock->setTimeFormat(fmt);
    }

    RefreshModes RelaxationRunningProgressWindow::updateTime()
    {
        if (presenter != nullptr) {
            presenter->handleUpdateTimeEvent();
        }
        return RefreshModes::GUI_REFRESH_FAST;
    }

    void RelaxationRunningProgressWindow::handleError()
    {
        auto switchData = std::make_unique<AudioErrorParams>(AudioErrorType::UnsupportedMediaType);
        application->switchWindow(gui::window::name::audioErrorWindow, std::move(switchData));
    }

    void RelaxationRunningProgressWindow::handleDeletedFile()
    {
        auto switchData = std::make_unique<AudioErrorParams>(AudioErrorType::FileDeleted);
        application->switchWindow(gui::window::name::audioErrorWindow, std::move(switchData));
    }
} // namespace gui
