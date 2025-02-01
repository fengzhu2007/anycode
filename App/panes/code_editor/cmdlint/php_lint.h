#ifndef PHP_LINT_H
#define PHP_LINT_H
#include "interface/code_parse_lint.h"
#include <QString>
#include <QString>
namespace ady{
class PHPLint : public CodeParseLint
{
public:
    PHPLint();
    virtual void parse(const QString& source,const QString& path) override;
    virtual QList<CodeErrorInfo> results() override;
    static void settingsChanged();
private:
    void command(const QString& path);
    bool executeFound();
    void initExecutePath();
private:
    QString m_output;
    static QString executePath;
};
}
#endif // PHP_LINT_H
