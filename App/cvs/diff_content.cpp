#include "diff_content.h"

namespace ady {
namespace cvs {

// --- DiffLine ---

DiffLine::DiffLine()
    : m_type(Context), m_oldLineNo(-1), m_newLineNo(-1)
{
}

DiffLine::DiffLine(Type type, const QString &content, int oldLineNo, int newLineNo)
    : m_type(type), m_content(content), m_oldLineNo(oldLineNo), m_newLineNo(newLineNo)
{
}

// --- DiffHunk ---

DiffHunk::DiffHunk()
    : m_oldStart(0), m_oldCount(0), m_newStart(0), m_newCount(0)
{
}

// --- DiffContent ---

DiffContent::DiffContent()
{
}

QList<DiffLine> DiffContent::allLines() const
{
    QList<DiffLine> lines;
    for (const auto &hunk : m_hunks) {
        lines.append(hunk.lines());
    }
    return lines;
}

int DiffContent::additions() const
{
    int count = 0;
    for (const auto &hunk : m_hunks) {
        for (const auto &line : hunk.lines()) {
            if (line.type() == DiffLine::Addition)
                count++;
        }
    }
    return count;
}

int DiffContent::deletions() const
{
    int count = 0;
    for (const auto &hunk : m_hunks) {
        for (const auto &line : hunk.lines()) {
            if (line.type() == DiffLine::Deletion)
                count++;
        }
    }
    return count;
}

} // namespace cvs
} // namespace ady
