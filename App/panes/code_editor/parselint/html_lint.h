#ifndef HTML_LINT_H
#define HTML_LINT_H
#include "interface/code_parse_lint.h"
#include <tree_sitter/api.h>
#include <vector>
namespace ady{
class HTMLLint : public CodeParseLint
{
public:
    enum Order{
        Html=0,
        Php
    };
    HTMLLint();

    virtual void parse(const QString& source,const QString& path) override;


private:
    bool parseHtml(TSParser* parser,const std::string& text,const std::vector<TSRange>& ranges);
    bool parsePhp(TSParser* parser,const std::string& text,const std::vector<TSRange>& ranges);

};
}

#endif // HTML_LINT_H
