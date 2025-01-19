#include "css_lint.h"
#include <tree_sitter/api.h>
#include <css/tree-sitter-css.h>
#include <QTextDocument>
#include <QDebug>
namespace ady{




CSSLint::CSSLint():CodeParseLint() {
    this->setup(tree_sitter_css());
}


}
