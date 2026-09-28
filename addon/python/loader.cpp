#include "loader.h"
#include "pythonhighlighter.h"
#include "pythonindenter.h"

namespace Python {

Loader::Loader(QTextDocument* doc) : TextEditor::LanguageLoader(doc) {
    m_hightlighter = new Python::Highlighter();
    m_indenter = Python::createIndenter(doc);
}

} // namespace Python
