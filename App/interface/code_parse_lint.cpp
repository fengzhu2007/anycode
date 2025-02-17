#include "code_parse_lint.h"
#include <tree_sitter/api.h>
#include <QDebug>
namespace ady{
class CodeParseLintPrivate{
public:
    TSLanguage* lang;
    TSNode node;
    QString source;
    QString path;
    QString errorMsg;
    int row;
    int col;
    QList<CodeErrorInfo> list;
};


CodeParseLint::CodeParseLint(){
    d = new CodeParseLintPrivate;
    d->lang = nullptr;
}

CodeParseLint::~CodeParseLint(){
    delete d;
}


static void findErrors(const TSNode node,QString* error,int *row,int *col){
    //qDebug()<<"node"<<ts_node_type(node)<<ts_node_string(node);
    auto ret = ts_node_is_missing(node);
    if(ret){
        TSPoint start = ts_node_start_point(node);
        *row = start.row;
        *col = start.column;
        auto string = ts_node_string(node);
        auto type = ts_node_type(node);
        *error = QString::fromUtf8(string);
        QString nodeType = QString::fromUtf8(type);
        free(string);
    }else{
        ret = ts_node_is_error(node);
        int count = ts_node_child_count(node);
        //qDebug()<<"count"<<count;
        if(ret){
            if(count==0){
                TSPoint start = ts_node_start_point(node);
                *row = start.row;
                *col = start.column;
                auto string = ts_node_string(node);
                auto type = ts_node_type(node);
                *error = QString::fromUtf8(string);
                QString nodeType = QString::fromUtf8(type);
                free(string);
                return;
            }
             for(int i=0;i<count;i++){
                 auto child = ts_node_child(node,i);
                 findErrors(child,error,row,col);
                 if((*error).isEmpty()==false){
                     break;
                 }
             }
        }else{
            //walk all children if has children
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



    /*auto ret = ts_node_is_error(node);
    if(!ret){
        ret = ts_node_is_missing(node);
    }
    if(ret){
        TSPoint start = ts_node_start_point(node);
        *row = start.row;
        *col = start.column;
        auto string = ts_node_string(node);
        qDebug()<<"string"<<string;
        auto type = ts_node_type(node);
        *error = QString::fromUtf8(string);
        QString nodeType = QString::fromUtf8(type);
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
    }*/
}


static void walkAll(const TSNode node){
    int count = ts_node_child_count(node);
    qDebug()<<"type"<<ts_node_type(node);
    for(int i=0;i<count;i++){
        walkAll(ts_node_child(node,i));
    }
}


void CodeParseLint::parse(const QString& source,const QString& path){
    Q_UNUSED(path);
    std::string code = source.toStdString();
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, d->lang);
    TSTree *tree = ts_parser_parse_string(
        parser,
        NULL,
        code.c_str(),
        code.length()
        );
    if(tree){
        TSNode root_node = ts_tree_root_node(tree);
        //walkAll(root_node);
        bool ret = ts_node_has_error(root_node);
        if(ret){
            //row col 0-base
            findErrors(root_node,&d->errorMsg,&d->row,&d->col);
            //this->walkErrors(root_node);
        }
        ts_tree_delete(tree);
    }
    ts_parser_delete(parser);
}

QList<CodeErrorInfo> CodeParseLint::results(){
    QList<CodeErrorInfo> list;
    if(d->errorMsg.isEmpty()==false){
        auto errorMsg = d->errorMsg.mid(1,d->errorMsg.length() - 2);
        if(errorMsg.startsWith("ERROR")){
            errorMsg = QString::fromUtf8("Syntax error");
        }else if(errorMsg.startsWith("MISSING")){
            errorMsg = errorMsg.replace("MISSING","Missing");
        }else if(errorMsg.startsWith("UNEXPECTED")){
            errorMsg = errorMsg.replace("UNEXPECTED","Unexpected");
        }
        list << CodeErrorInfo(CodeErrorInfo::Error,d->row,d->col,0,errorMsg);
    }

    return list;
    /*for(auto one:d->list){
        qDebug()<<one.message<<one.line<<one.column<<one.length;
    }
    return d->list;*/
}

void CodeParseLint::setup(const TSLanguage* l){
    d->lang = const_cast<TSLanguage*>(l);
}

int CodeParseLint::row(){
    return d->row;
}
int CodeParseLint::col(){
    return d->col;
}
int CodeParseLint::length(){
    return 0;
}
QString& CodeParseLint::message(){
    return d->errorMsg;
}
void CodeParseLint::setRow(int row){
    d->row = row;
}
void CodeParseLint::setCol(int col){
    d->col = col;
}
void CodeParseLint::setLength(int length){

}
void CodeParseLint::setMessage(const QString& message){
    d->errorMsg = message;
}
void CodeParseLint::walkErrors(const TSNode& node){
    auto ret = ts_node_is_error(node);
    if(!ret){
        ret = ts_node_is_missing(node);
    }
    if(ret){
        TSPoint start = ts_node_start_point(node);
        int row = start.row;
        int col = start.column;
        auto string = ts_node_string(node);
        auto type = ts_node_type(node);
        auto parent = ts_node_parent(node);
        int count = ts_node_child_count(node);
        //qDebug()<<"type"<<QString::fromUtf8(type)<<"parent"<<QString::fromUtf8(ts_node_type(parent))<<count;

        auto startByte = ts_node_start_byte(node);
        auto endByte = ts_node_end_byte(node);
        int length = endByte - startByte;
        QString errorMsg = QString::fromUtf8(string);
        QString nodeType = QString::fromUtf8(type);
        free(string);
        d->list << CodeErrorInfo{CodeErrorInfo::Error,row,col,length,errorMsg};
        //qDebug()<<"error ";
    }
        //find children
        int count = ts_node_child_count(node);
        if(count>0){
            for(int i=0;i<count;i++){
                auto child = ts_node_child(node,i);
                this->walkErrors(child);
            }
        }

}
}
