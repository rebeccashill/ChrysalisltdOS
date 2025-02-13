// Copyright (c) 2017-2024, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/blob/master/LICENSE.md

#include "NotesMainWindowPresenter.hpp"

namespace app::notes
{
    NotesMainWindowPresenter::NotesMainWindowPresenter(std::shared_ptr<NotesListItemProvider> notesProvider)
        : notesProvider{std::move(notesProvider)}
    {}

    std::shared_ptr<gui::ListItemProvider> NotesMainWindowPresenter::getNotesItemProvider() const
    {
        return notesProvider;
    }
} // namespace app::notes
