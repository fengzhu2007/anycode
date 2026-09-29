#pragma once

#include "autocompleter.h"
#include "python_global.h"

namespace Python {

class PYTHON_EXPORT AutoCompleter : public TextEditor::AutoCompleter
{
public:
    AutoCompleter();
    ~AutoCompleter() override;

    bool contextAllowsAutoBrackets(const QTextCursor &cursor,
                                   const QString &textToInsert = QString()) const override;
    bool contextAllowsAutoQuotes(const QTextCursor &cursor,
                                 const QString &textToInsert = QString()) const override;
    bool contextAllowsElectricCharacters(const QTextCursor &cursor) const override;
    bool isInComment(const QTextCursor &cursor) const override;
    QString insertMatchingBrace(const QTextCursor &tc,
                                const QString &text,
                                QChar lookAhead,
                                bool skipChars,
                                int *skippedChars,int* adjustPos) const override;
    QString insertMatchingQuote(const QTextCursor &tc,
                                const QString &text,
                                QChar lookAhead,
                                bool skipChars,
                                int *skippedChars) const override;
    QString insertParagraphSeparator(const QTextCursor &tc) const override;
};

} // namespace Python
