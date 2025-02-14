#include "html_lint.h"
#include <html/tree-sitter-html.h>
#include <php/tree-sitter-php_only.h>
#include <template/tree-sitter-embedded-template.h>
#include <algorithm>
#include <QTextDocument>
#include <QDebug>
namespace ady{

HTMLLint::HTMLLint():CodeParseLint() {
    this->setup(tree_sitter_html());
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
        auto type = ts_node_type(node);
        *error = QString::fromUtf8(string);
        QString nodeType = QString::fromUtf8(type);
        free(string);
    }else{
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

void HTMLLint::parse(const QString& source,const QString& path) {
    std::string text = source.toStdString();
    unsigned len = text.size();
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_embedded_template());
    TSTree *html_tree = ts_parser_parse_string(parser, NULL, text.c_str(), len);
    TSNode html_root_node = ts_tree_root_node(html_tree);

    unsigned child_count = ts_node_child_count(html_root_node);

    std::vector<TSRange> html_ranges;
    std::vector<TSRange> php_ranges;
    html_ranges.reserve(10);
    php_ranges.reserve(10);
    std::vector<Order> orders;
    for (unsigned i = 0; i < child_count; i++) {
        TSNode node = ts_node_child(html_root_node, i);
        QString nodeType = QString::fromUtf8(ts_node_type(node));
        if(nodeType.startsWith("php_directive")){
            //php
            TSNode code_node = ts_node_named_child(node, 0);
            php_ranges.push_back({
                ts_node_start_point(code_node),
                ts_node_end_point(code_node),
                ts_node_start_byte(code_node),
                ts_node_end_byte(code_node),
            });
            if(std::find(orders.begin(),orders.end(),Php)==orders.end()){
                orders.push_back(Php);
            }
        }else if(nodeType=="content"){
            //html
            html_ranges.push_back( {
                ts_node_start_point(node),
                ts_node_end_point(node),
                ts_node_start_byte(node),
                ts_node_end_byte(node),
            });
            if(std::find(orders.begin(),orders.end(),Html)==orders.end()){
                orders.push_back(Html);
            }
        }
    }

    for(auto l:orders){
        if(l==Html){
            auto ret = this->parseHtml(parser,text,html_ranges);
            if(ret){
                return ;
            }
        }else if(l==Php){
            auto ret = this->parsePhp(parser,text,php_ranges);
            if(ret){
                return ;
            }
        }
    }

}

bool HTMLLint::parseHtml(TSParser* parser,const std::string& text,const std::vector<TSRange>& ranges){
    ts_parser_set_language(parser, tree_sitter_html());
    TSRange* array = new TSRange[ranges.size()];
    for(int i=0;i<ranges.size();i++){
        array[i] = ranges.at(i);
    }
    ts_parser_set_included_ranges(parser, array, ranges.size());
    TSTree *tree = ts_parser_parse_string(parser, NULL, text.c_str(), text.size());
    TSNode root_node = ts_tree_root_node(tree);
    QString errorMsg;
    int row;
    int col;
    findErrors(root_node,&errorMsg,&row,&col);
    delete[] array;
    if(!errorMsg.isEmpty()){
        this->setMessage(errorMsg);
        this->setRow(row);
        this->setCol(col);
        return true;
    }else{
        return false;
    }
}

bool HTMLLint::parsePhp(TSParser* parser,const std::string& text,const std::vector<TSRange>& ranges){
    ts_parser_set_language(parser, tree_sitter_php_only());
    TSRange* array = new TSRange[ranges.size()];
    for(int i=0;i<ranges.size();i++){
        array[i] = ranges.at(i);
    }
    ts_parser_set_included_ranges(parser, array, ranges.size());
    TSTree *tree = ts_parser_parse_string(parser, NULL, text.c_str(), text.size());
    TSNode root_node = ts_tree_root_node(tree);
    QString errorMsg;
    int row;
    int col;
    findErrors(root_node,&errorMsg,&row,&col);
    delete[] array;
    if(!errorMsg.isEmpty()){
        this->setMessage(errorMsg);
        this->setRow(row);
        this->setCol(col);
        return true;
    }else{
        return false;
    }
}

}
