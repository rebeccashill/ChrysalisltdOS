// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include "AlarmSettingsModel.hpp"
#include <db/SystemSettings.hpp>

namespace app::bell_settings
{
    namespace
    {
        inline void validateBrightness(std::string &brightness)
        {
            constexpr auto defaultBrightness = std::string_view{"5"};

            if (brightness.empty()) {
                brightness = defaultBrightness;
            }
        }
    }

    void AlarmToneModel::setValue(UTF8 value)
    {
        settings.setValue(bell::settings::Alarm::tonePath, value, settings::SettingsScope::Global);
    }

    UTF8 AlarmToneModel::getValue() const
    {
        return settings.getValue(bell::settings::Alarm::tonePath, settings::SettingsScope::Global);
    }

    void AlarmVolumeModel::setValue(std::uint8_t value)
    {
        audioModel.setVolume(value, AbstractAudioModel::PlaybackType::Alarm);
    }

    std::uint8_t AlarmVolumeModel::getValue() const
    {
        return defaultValue;
    }

    void AlarmVolumeModel::restoreDefault()
    {
        setValue(defaultValue);
    }

    AlarmVolumeModel::AlarmVolumeModel(AbstractAudioModel &audioModel) : audioModel{audioModel}
    {
        defaultValue = audioModel.getVolume(AbstractAudioModel::PlaybackType::Alarm).value_or(0);
    }

    void AlarmFadeOnOffModel::setValue(bool value)
    {
        const auto valStr = std::to_string(static_cast<int>(value));
        settings.setValue(bell::settings::Alarm::fadeActive, valStr, settings::SettingsScope::Global);
    }

    bool AlarmFadeOnOffModel::getValue() const
    {
        const auto str = settings.getValue(bell::settings::Alarm::fadeActive, settings::SettingsScope::Global);
        return (utils::toNumeric(str) != 0);
    }

    void AlarmLightOnOffModel::setValue(bool value)
    {
        const auto valStr = std::to_string(static_cast<int>(value));
        settings.setValue(bell::settings::Alarm::lightActive, valStr, settings::SettingsScope::Global);
    }

    bool AlarmLightOnOffModel::getValue() const
    {
        const auto str = settings.getValue(bell::settings::Alarm::lightActive, settings::SettingsScope::Global);
        return (utils::toNumeric(str) != 0);
    }

    void AlarmFrontlightModel::setValue(std::uint8_t value)
    {
        const auto valStr = std::to_string(value);
        settings.setValue(bell::settings::Alarm::brightness, valStr, settings::SettingsScope::Global);
    }

    std::uint8_t AlarmFrontlightModel::getValue() const
    {
        auto str = settings.getValue(bell::settings::Alarm::brightness, settings::SettingsScope::Global);
        validateBrightness(str);
        return utils::toNumeric(str);
    }
} // namespace app::bell_settings
