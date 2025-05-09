#include "html_lint.h"
#include <html/tree-sitter-html.h>
#include <php/tree-sitter-php_only.h>
#include <javascript/tree-sitter-javascript.h>
#include <template/tree-sitter-embedded-template.h>
#include <algorithm>
#include <QTextDocument>
#include <QDebug>
namespace ady{

HTMLLint::HTMLLint():CodeParseLint() {
    this->setup(tree_sitter_html());
}

static void findHtmlErrors(const TSNode node,QString* error,int *row,int *col,const std::string& text,std::vector<TSRange>& ranges,std::vector<std::pair<int,std::string>>& codes){
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
                if (strcmp(ts_node_type(child), "script_element") == 0) {
                    uint32_t script_child_count = ts_node_child_count(child);
                    for (uint32_t j = 0; j < script_child_count; ++j) {
                        TSNode script_child = ts_node_child(child, j);

                        if (strcmp(ts_node_type(script_child), "raw_text") == 0 || strcmp(ts_node_type(script_child), "text") == 0) {

                            auto start = ts_node_start_byte(script_child);
                            auto end = ts_node_end_byte(script_child);
                            if(end>start){

                                std::string js_code = text.substr(start, end - start);
                                //qDebug()<<"code"<<js_code.c_str();

                                if(js_code.find("<?")!= std::string::npos){
                                    //codes
                                    std::string result;
                                    size_t pos = 0;
                                    size_t php_start, php_end;
                                    while ((php_start = js_code.find("<?", pos)) != std::string::npos) {
                                        result += js_code.substr(pos, php_start - pos);
                                        php_end = js_code.find("?>", php_start);
                                        if (php_end == std::string::npos) {
                                            php_end = js_code.length();
                                        } else {
                                            php_end += 2;
                                        }
                                        result += std::string(php_end - php_start,'a');
                                        pos = php_end;
                                    }
                                    result += js_code.substr(pos);
                                    TSPoint start_point = ts_node_start_point(script_child);
                                    codes.push_back({start_point.row,result});
                                    //codes.push_back(result);
                                }else{
                                    ranges.push_back( {
                                        ts_node_start_point(script_child),
                                        ts_node_end_point(script_child),
                                        start,
                                        end,
                                    });
                                }
                            }
                        }
                    }
                }
                findHtmlErrors(child,error,row,col,text,ranges,codes);
                if((*error).isEmpty()==false){
                    break;
                }
            }
        }
    }
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
        //qDebug()<<"type"<<nodeType<<string<<*row<<*col;
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
        auto ret = ts_node_is_error(node);
        QString nodeType = QString::fromUtf8(ts_node_type(node));
        //qDebug()<<"nodeType"<<nodeType<<ts_node_string(node);
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
        }else  if(nodeType.startsWith("template_directive")){


            continue;
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
        }else if(nodeType=="ERROR"){
            QString errorMsg;
            int row;
            int col;
            findErrors(node,&errorMsg,&row,&col);
            if(!errorMsg.isEmpty()){
                this->setMessage(errorMsg);
                this->setRow(row);
                this->setCol(col);
                ts_tree_delete(html_tree);
                ts_parser_delete(parser);
                return ;
            }
        }
    }

    for(auto l:orders){
        if(l==Html){
            auto ret = this->parseHtml(parser,text,html_ranges);
            if(ret){
                break;
            }
        }else if(l==Php){
            auto ret = this->parsePhp(parser,text,php_ranges);
            if(ret){
                break;
            }
        }
    }
    ts_parser_delete(parser);
    return ;

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
    std::vector<TSRange> js_ranges;
    js_ranges.reserve(3);
    //std::vector<std::string> js_codes;
    std::vector<std::pair<int,std::string>> js_codes;
    findHtmlErrors(root_node,&errorMsg,&row,&col,text,js_ranges,js_codes);
    delete[] array;
    if(!errorMsg.isEmpty()){
        this->setMessage(errorMsg);
        this->setRow(row);
        this->setCol(col);
        ts_tree_delete(tree);
        return true;
    }else{
        if(js_ranges.size()>0){
            this->parseJS(parser,text,js_ranges);
        }
        //qDebug()<<"code size"<<js_codes.size();
        if(js_codes.size()>0){
            TSParser *parser = ts_parser_new();
            ts_parser_set_language(parser, tree_sitter_javascript());
            for(auto one:js_codes){
                auto ret = this->parseJS(parser,one.second,one.first);
                if(ret==true){
                    ts_parser_delete(parser);
                    return true;
                }
            }
            ts_parser_delete(parser);

        }
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
        ts_tree_delete(tree);
        return true;
    }else{
        ts_tree_delete(tree);
        return false;
    }
}

bool HTMLLint::parseJS(TSParser* parser,const std::string& text,const std::vector<TSRange>& ranges){
    ts_parser_set_language(parser, tree_sitter_javascript());
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
        ts_tree_delete(tree);
        return true;
    }else{
        ts_tree_delete(tree);
        return false;
    }
}


bool HTMLLint::parseJS(TSParser* parser,const std::string& text,int start){


    //qDebug()<<"code"<<text.c_str()<<start;
    TSTree *tree = ts_parser_parse_string(parser, NULL, text.c_str(), text.size());
    TSNode root_node = ts_tree_root_node(tree);
    QString errorMsg;
    int row;
    int col;
    findErrors(root_node,&errorMsg,&row,&col);
    if(!errorMsg.isEmpty()){
        this->setMessage(errorMsg);
        this->setRow(row + start);
        this->setCol(col);
        ts_tree_delete(tree);
        return true;
    }else{
        ts_tree_delete(tree);
        return false;
    }
}

}
