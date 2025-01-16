#include "python_lint.h"
#include <QProcess>
#include <QDebug>
namespace ady{


PythonLint::PythonLint(const QString& path){
    QProcess process;
    process.start("python", QStringList() << "-m py_compile" << path);
    process.waitForFinished();
    m_output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

}


QList<CodeErrorInfo> PythonLint::parse(){
    CodeErrorInfo info;
    if(!m_output.isEmpty() && m_output.startsWith("No")==false){
        //"\nParse error: syntax error, unexpected '$a' (T_VARIABLE), expecting ';' in D:/wamp/www/test/a.php on line 16\n"
        //No syntax errors detected in D:/wamp/www/test/a.php\n
        qDebug()<<m_output;
        int index = 0;
        int state = 0;
        const int CONTENT = 1;
        const int QOUTE = 2;
        const int LINE = 4;
        int start = 0;


        while(index<m_output.length()){
            QChar ch = m_output.at(index);

            if(ch==QLatin1Char(':') && state==0){
                //error msg start
                info.level = CodeErrorInfo::Error;
                state |= CONTENT;
                start = index + 1;
            }else if(ch==QLatin1Char('\'')){
                if((state&QOUTE)==QOUTE){
                    state = state&~QOUTE;
                }else{
                    state |= QOUTE;
                }
            }else if(state==CONTENT && ch==QLatin1Char('i') ){
                if(index+2 < m_output.length() && m_output[index+1]==QLatin1Char('n') && m_output[index+2].isSpace()){
                    //content is end
                    info.message = m_output.mid(start,index - start).trimmed();
                    start = 0;
                    state |= LINE;
                    index += 2;
                }
            }else if((state & LINE)==LINE && ch==QLatin1Char('o') && index+7 < m_output.length() && m_output[index+1]==QLatin1Char('n') && m_output[index+2].isSpace()){

                index += 7;
                start = index;

            }else if((state & LINE)==LINE  && start>0){
                if(!ch.isNumber()){
                    //number is end
                    QString number = m_output.mid(start,index - start);
                    info.line = number.toInt();
                    break;
                }
            }
            index++;
        }
    }
    return {info};
}

}

