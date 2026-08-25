#ifndef CVS_DIFF_CONTENT_H
#define CVS_DIFF_CONTENT_H

#include "global.h"
#include <QString>
#include <QList>

namespace ady {
namespace cvs {

class ANYENGINE_EXPORT DiffLine {
public:
    enum Type {
        Context = 0,    // Unchanged line
        Addition,       // Added line
        Deletion,       // Deleted line
        Modification    // Modified line (reserved for future extension)
    };

    DiffLine();
    DiffLine(Type type, const QString &content, int oldLineNo = -1, int newLineNo = -1);

    Type type() const { return m_type; }
    QString content() const { return m_content; }
    int oldLineNo() const { return m_oldLineNo; }
    int newLineNo() const { return m_newLineNo; }

    void setType(Type type) { m_type = type; }
    void setContent(const QString &content) { m_content = content; }
    void setOldLineNo(int no) { m_oldLineNo = no; }
    void setNewLineNo(int no) { m_newLineNo = no; }

private:
    Type m_type;
    QString m_content;
    int m_oldLineNo;
    int m_newLineNo;
};

class ANYENGINE_EXPORT DiffHunk {
public:
    DiffHunk();

    int oldStart() const { return m_oldStart; }
    int oldCount() const { return m_oldCount; }
    int newStart() const { return m_newStart; }
    int newCount() const { return m_newCount; }
    QString header() const { return m_header; }
    QList<DiffLine> lines() const { return m_lines; }

    void setOldStart(int start) { m_oldStart = start; }
    void setOldCount(int count) { m_oldCount = count; }
    void setNewStart(int start) { m_newStart = start; }
    void setNewCount(int count) { m_newCount = count; }
    void setHeader(const QString &header) { m_header = header; }
    void addLine(const DiffLine &line) { m_lines.append(line); }

private:
    int m_oldStart;
    int m_oldCount;
    int m_newStart;
    int m_newCount;
    QString m_header;
    QList<DiffLine> m_lines;
};

class ANYENGINE_EXPORT DiffContent {
public:
    DiffContent();

    void addHunk(const DiffHunk &hunk) { m_hunks.append(hunk); }
    QList<DiffHunk> hunks() const { return m_hunks; }
    bool isEmpty() const { return m_hunks.isEmpty(); }

    // Get all lines (expand all hunks)
    QList<DiffLine> allLines() const;

    // Statistics
    int additions() const;
    int deletions() const;

private:
    QList<DiffHunk> m_hunks;
};

} // namespace cvs
} // namespace ady

#endif // CVS_DIFF_CONTENT_H
