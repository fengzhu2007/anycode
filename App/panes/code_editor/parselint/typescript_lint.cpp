#include "typescript_lint.h"
#include <tree_sitter/api.h>
#include <typescript/tree-sitter-typescript.h>
#include <QTextDocument>
#include <QDebug>


namespace ady{




TypescriptLint::TypescriptLint():CodeParseLint() {
    this->setup(tree_sitter_typescript());
}


}
