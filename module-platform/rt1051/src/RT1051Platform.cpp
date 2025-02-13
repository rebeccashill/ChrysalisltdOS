// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include <platform/rt1051/RT1051Platform.hpp>

#include "BlockDeviceFactory.hpp"

#include <bsp/bsp.hpp>
#include <purefs/vfs_subsystem.hpp>
#include <exception>
#include <Logger.hpp>

using platform::rt1051::BlockDeviceFactory;
using platform::rt1051::RT1051Platform;

RT1051Platform::RT1051Platform()
{
    bsp::board_init();
    bsp::register_exit_function(Log::Logger::destroyInstance);
}

void RT1051Platform::init()
{
    initFilesystem();
}

void RT1051Platform::initFilesystem()
{
    if (usesFilesystem) {
        throw std::runtime_error("Filesystem already initialized");
    }

    auto blockDeviceFactory = std::make_unique<BlockDeviceFactory>();
    vfs                     = purefs::subsystem::initialize(std::move(blockDeviceFactory));

    if (const auto err = purefs::subsystem::mount_defaults(); err != 0) {
        throw std::runtime_error("Failed to initiate filesystem: " + std::to_string(err));
    }

    usesFilesystem = true;
}
void platform::rt1051::RT1051Platform::deinit()
{
    if (usesFilesystem) {
        if (const auto err = purefs::subsystem::unmount_all(); err != 0) {
            throw std::runtime_error("Failed to unmount all: " + std::to_string(err));
        }
        usesFilesystem = false;
    }
}
