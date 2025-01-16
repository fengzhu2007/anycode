#include "python_lint.h"
#include <tree_sitter/api.h>
#include <python/tree-sitter-python.h>
#include <QTextDocument>
#include <QDebug>


namespace ady{


static void findErrors(const TSNode node,QString* error,int *row,int *col){
    auto ret = ts_node_is_error(node);
    if(!ret){
        ret = ts_node_is_missing(node);
    }
    if(ret){
        TSPoint start = ts_node_start_point(node);
        *row = start.row;
        *col = start.column;
        auto string = ts_node_string(node);
        *error = QString::fromUtf8(string);
        free(string);
    }else{
        //find children
        int count = ts_node_child_count(node);
        if(count>0){
            for(int i=0;i<count;i++){
                auto child = ts_node_child(node,i);
                findErrors(child,error,row,col);
                if((*error).isEmpty()==false){
                    break;
                }
            }
        }
    }
}



PythonLint::PythonLint(TextEditor::TextDocument* textDocument):m_textDocument(textDocument) {
    QString source = m_textDocument->document()->toPlainText();
    QString path = m_textDocument->filePath().toString();
    std::string code = source.toStdString();
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_python());

    TSTree *tree = ts_parser_parse_string(
        parser,
        NULL,
        code.c_str(),
        code.length()
        );
    if(tree){
        TSNode root_node = ts_tree_root_node(tree);
        bool ret = ts_node_has_error(root_node);
        if(ret){
            //row col 0-base
            findErrors(root_node,&m_errorMsg,&m_row,&m_col);
        }
        ts_tree_delete(tree);
    }
    ts_parser_delete(parser);
}


QList<CodeErrorInfo> PythonLint::parse(){
    QList<CodeErrorInfo> infolist;
    if(m_errorMsg.isEmpty()==false){
        CodeErrorInfo info;
        info.level = CodeErrorInfo::Error;
        info.message = m_errorMsg.mid(1,m_errorMsg.length() - 2);//clear start and end ()
        info.column = m_col;
        info.line = m_row + 1;//to 1-base
        infolist<<info;
    }
    return infolist;
}

}

