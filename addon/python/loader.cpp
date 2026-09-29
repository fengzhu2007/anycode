#include "loader.h"
#include "pythonhighlighter.h"
#include "pythonindenter.h"
#include "pythonautocompleter.h"
#include "pythoncodeformatter.h"

namespace Python {

Loader::Loader(QTextDocument* doc) : TextEditor::LanguageLoader(doc) {
    m_hightlighter = new Python::Highlighter();
    m_indenter = Python::createIndenter(doc);
    m_autoCompleter = new Python::AutoCompleter();
    m_codeFormatter = new Python::CodeFormatter();
}

} // namespace Python
