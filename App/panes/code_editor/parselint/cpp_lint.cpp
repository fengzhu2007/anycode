#include "cpp_lint.h"

#include <tree_sitter/api.h>
#include <cpp/tree-sitter-cpp.h>
#include <QTextDocument>
#include <QDebug>
namespace ady{




CPPLint::CPPLint():CodeParseLint() {
    this->setup(tree_sitter_cpp());
}


}
