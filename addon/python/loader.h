#pragma once

#include "languages/loader.h"
#include "python_global.h"

namespace Python {

class PYTHON_EXPORT Loader : public TextEditor::LanguageLoader
{
public:
    explicit Loader(QTextDocument* doc);
};

} // namespace Python
