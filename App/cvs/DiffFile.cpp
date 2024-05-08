#include "DiffFile.h"
namespace ady {
namespace cvs {
    DiffFile::DiffFile(QString path,Status status)
    {
        m_path = path;
        m_status = status;
    }

}
}
