#ifndef PYTHON_H
#define PYTHON_H

#include "python_global.h"
#include <languages/loader.h>
#include <QTextDocument>

namespace Python {

PYTHON_EXPORT TextEditor::LanguageLoader *createLoader(QTextDocument *doc);

} // namespace Python

#ifdef __cplusplus
extern "C" {
#endif
// Addon exported functions (resolved by AddonLoader via QLibrary)
PYTHON_EXPORT bool addonInstall();
PYTHON_EXPORT bool addonUninstall();
PYTHON_EXPORT TextEditor::LanguageLoader* createLanguageLoader(QString languageName, QTextDocument* doc);
#ifdef __cplusplus
}
#endif

#endif // PYTHON_H
