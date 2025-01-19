#include "html_lint.h"

#include <tree_sitter/api.h>
#include <html/tree-sitter-html.h>
#include <QTextDocument>
#include <QDebug>


namespace ady{




HTMLLint::HTMLLint():CodeParseLint() {
    this->setup(tree_sitter_html());
}


}
