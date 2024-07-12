#include "html_language.h"

namespace ady{

HtmlLanguage::HtmlLanguage(){

}

HtmlLanguage::~HtmlLanguage(){


}

QString HtmlLanguage::name(){
    return "html";
}

QVector<QString> HtmlLanguage::keywords(){
    return {"a","abbr","acronym","address","applet","area","article","aside","audio","b","base","basefont","bdi","bdo","bgsound","big","blink","blockquote","body","br","button","canvas","caption","center","cite","code","col","colgroup","content","data","datalist","dd","del","details","dfn","dialog","dir","div","dl","dt","element","em","embed","fieldset","figcaption","figure","font","footer","form","frame","frameset","h1","h2","h3","h4","h5","h6","head","header","hgroup","hr","html","i","iframe","image","img","input","ins","isindex","kbd","keygen","label","legend","li","link","listing","main","map","mark","marquee","menu","menuitem","meta","meter","nav","nobr","noembed","noframes","noscript","object","ol","optgroup","option","output","p","param","picture","plaintext","pre","progress","q","rb","rp","rt","rtc","ruby","s","samp","script","section","select","shadow","slot","small","source","spacer","span","strike","strong","style","sub","summary","sup","table","tbody","td","template","textarea","tfoot","th","thead","time","title","tr","track","tt","u","ul","var","video","wbr","xmp"};
}


QVector<QString> HtmlLanguage::classes(){
    return {};
}

QVector<QString> HtmlLanguage::functions(){

    return {};
}

QVector<QString> HtmlLanguage::variants(){
    return {};
}

QVector<QString> HtmlLanguage::constants(){
    return {};
}
QVector<QString> HtmlLanguage::fields(){
    return {"accept", "accept-charset", "accesskey", "action", "align", "allow", "alt", "async", "autocapitalize", "autocomplete", "autofocus", "autoplay", "charset", "checked", "cite", "class", "color", "cols", "colspan", "content", "contenteditable", "controls", "coords", "crossorigin", "data", "datetime", "decoding", "default", "defer", "dir", "dirname", "disabled", "download", "draggable", "enctype", "enterkeyhint", "for", "form", "formaction", "formenctype", "formmethod", "formnovalidate", "formtarget", "headers", "height", "hidden", "high", "href", "hreflang", "http-equiv", "icon", "id", "importance", "integrity", "intrinsicsize", "inputmode", "is", "ismap", "itemid", "itemprop", "itemref", "itemscope", "itemtype", "kind", "label", "lang", "language", "list", "loading", "loop", "low", "max", "maxlength", "media", "method", "min", "minlength", "multiple", "muted", "name", "novalidate", "open", "optimum", "pattern", "ping", "placeholder", "playsinline", "poster", "preload", "readonly", "referrerpolicy", "rel", "required", "reversed", "role", "rows", "rowspan", "sandbox", "scope", "selected", "shape", "size", "sizes", "slot", "span", "spellcheck", "src", "srcdoc", "srclang", "srcset", "start", "step", "style", "tabindex", "target", "title", "translate", "type", "usemap", "value", "width", "wrap"};
}



}
