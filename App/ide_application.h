#ifndef IDE_APPLICATION_H
#define IDE_APPLICATION_H

#include <QApplication>
#include "global.h"

namespace ady{
class ANYENGINE_EXPORT IDEApplication : public QApplication
{
public:
    IDEApplication(int &argc, char **argv);
    ~IDEApplication();
};
}
#endif // IDE_APPLICATION_H
