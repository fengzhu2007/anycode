#include "python_lint.h"
#include <tree_sitter/api.h>
#include <python/tree-sitter-python.h>

#include <QDebug>


namespace ady{

PythonLint::PythonLint():CodeParseLint() {
    this->setup(tree_sitter_python());
}





}

