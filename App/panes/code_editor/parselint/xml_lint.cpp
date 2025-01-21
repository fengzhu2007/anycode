#include "xml_lint.h"
#include <tree_sitter/api.h>
#include <xml/tree-sitter-xml.h>
#include <QTextDocument>
#include <QDebug>
namespace ady{



XMLLint::XMLLint():CodeParseLint() {
    this->setup(tree_sitter_xml());
}


}
