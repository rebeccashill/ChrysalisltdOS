﻿// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include <endpoints/factoryReset/FactoryResetEndpoint.hpp>

#include <endpoints/JsonKeyNames.hpp>
#include <endpoints/message/Sender.hpp>
#include <service-desktop/DesktopMessages.hpp>
#include <service-desktop/ServiceDesktop.hpp>

#include <json11.hpp>

#include <memory>

namespace sdesktop::endpoints
{
    auto FactoryResetEndpoint::handle(Context &context) -> void
    {
        if (context.getMethod() == http::Method::Post) {
            if (context.getBody()[json::factoryReset::factoryRequest] == true) {
                auto msg = std::make_shared<sdesktop::FactoryMessage>();
                ownerServicePtr->bus.sendUnicast(std::move(msg), service::name::service_desktop);

                context.setResponseBody(json11::Json::object({{json::factoryReset::factoryRequest, true}}));
            }
            else {
                context.setResponseBody(json11::Json::object({{json::factoryReset::factoryRequest, false}}));
            }
        }
        else {
            context.setResponseBody(json11::Json::object({{json::factoryReset::factoryRequest, false}}));
        }

        sender::putToSendQueue(context.createSimpleResponse());
    }
} // namespace sdesktop::endpoints
