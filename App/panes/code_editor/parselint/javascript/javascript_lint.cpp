#include "javascript_lint.h"
#include <quickjs.h>


#include <QTextDocument>
#include <QDebug>


namespace ady{
JavascriptLint::JavascriptLint(TextEditor::TextDocument* textDocument):m_textDocument(textDocument) {
    QString source = m_textDocument->document()->toPlainText();
    QString path = m_textDocument->filePath().toString();
    JSRuntime *rt;
    JSContext *ctx;
    rt = JS_NewRuntime();
    ctx = JS_NewContext(rt);
    auto ret = JS_Parse(ctx,source.toUtf8(),source.length(),path.toUtf8(),JS_EVAL_FLAG_COMPILE_ONLY|JS_EVAL_TYPE_MODULE);
    if(ret.tag==JS_TAG_EXCEPTION){
        auto exception = JS_GetException(ctx);
        auto str = JS_ToCStringLen2(ctx,NULL,exception,0);
        m_errorMsg = QString::fromUtf8(str);
        JS_FreeCString(ctx,str);
        JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
        auto stack_str = JS_ToCStringLen2(ctx,NULL,stack,0);
        if (stack_str) {
            m_stackInfo = QString::fromUtf8(stack_str);
            JS_FreeCString(ctx, stack_str);
        }
        JS_FreeValue(ctx, exception);
    }
    JS_FreeValue(ctx, ret);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}


QList<CodeErrorInfo> JavascriptLint::parse(){
    QList<CodeErrorInfo> infolist;
    //qDebug()<<"parse"<<m_errorMsg<<m_stackInfo;
    if(m_errorMsg.isEmpty()==false && m_stackInfo.isEmpty()==false){
        CodeErrorInfo info;
        info.level = CodeErrorInfo::Error;
        info.message = m_errorMsg;
        int index = 0;
        int state = 0;
        int start = -1;
        int LINE = 1;
        int COLUMN = 2;
        while(index < m_stackInfo.length()){
            QChar ch = m_stackInfo[index];
            if(ch==QLatin1Char(':') && index + 1 < m_stackInfo.length() && m_stackInfo[index+1].isNumber()){
                if(state==0){
                    state|= LINE;
                    index+=1;
                    start=index;//line start
                }else if((state&LINE)==LINE){
                    auto num = m_stackInfo.mid(start,index - start);
                    info.line = num.toInt();
                    state|= COLUMN;
                    index+=1;
                    start=index;//column start
                }
            }else if(ch.isSpace() && (state&COLUMN)==COLUMN){
                auto num = m_stackInfo.mid(start,index - start);
                info.column = num.toInt();
                start = -1;
            }
            ++index;
        }
        if((state&COLUMN)==COLUMN && start>0){
            auto num = m_stackInfo.mid(start,index);
            info.column = num.toInt();
        }
        //qDebug()<<"info"<<info.message<<info.line<<info.column;
        infolist<<info;
    }
    return infolist;
}

}
