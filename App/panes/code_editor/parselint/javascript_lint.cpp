#include "javascript_lint.h"
#include <tree_sitter/api.h>
#include <javascript/tree-sitter-javascript.h>
#include <QTextDocument>
#include <QDebug>


namespace ady{




JavascriptLint::JavascriptLint():CodeParseLint() {
    this->setup(tree_sitter_javascript());
}


}

