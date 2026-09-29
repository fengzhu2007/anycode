#pragma once

#include "textindenter.h"
#include "python_global.h"

namespace Python {

PYTHON_EXPORT TextEditor::TextIndenter *createIndenter(QTextDocument *doc);

} // namespace Python
