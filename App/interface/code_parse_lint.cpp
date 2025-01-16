#include "code_parse_lint.h"
#include <tree_sitter/api.h>

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
};


CodeParseLint::CodeParseLint(){
    d = new CodeParseLintPrivate;
    d->lang = nullptr;
}

CodeParseLint::~CodeParseLint(){
    delete d;
}


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
        bool ret = ts_node_has_error(root_node);
        if(ret){
            //row col 0-base
            findErrors(root_node,&d->errorMsg,&d->row,&d->col);
        }
        ts_tree_delete(tree);
    }
    ts_parser_delete(parser);
}

void CodeParseLint::setup(TSLanguage* l){
    d->lang = l;
}

}
