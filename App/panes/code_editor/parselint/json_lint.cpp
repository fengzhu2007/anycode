#include "json_lint.h"
#include <tree_sitter/api.h>
#include <json/tree-sitter-json.h>
#include <QTextDocument>
#include <QDebug>


namespace ady{

JSONLint::JSONLint():CodeParseLint() {
    this->setup(tree_sitter_json());
}



}
