#include "js_language.h"
#include <QStringList>
namespace ady{

JsLanguage::JsLanguage(){

}

JsLanguage::~JsLanguage(){


}

QString JsLanguage::name(){
    return "js";
}

QVector<QString> JsLanguage::keywords(){
    return {"arguments", "await", "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do", "else", "enum", "export", "extends", "false", "finally", "for", "function", "if", "import", "in", "instanceof", "new", "null", "return", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with", "yield"};
}

QVector<QString> JsLanguage::classes(){
    return QString::fromUtf8("Array,ArrayBuffer,Atomics,BigInt,BigInt64Array,BigUint64Array,Boolean,DataView,Date,decodeURI,decodeURIComponent,Document,Element,encodeURI,encodeURIComponent,Error,eval,EvalError,Event,Float32Array,Float64Array,Function,globalThis,Infinity,Int16Array,Int32Array,Int8Array,isFinite,isNaN,JSON,Location,Map,Math,NaN,Number,Object,parseFloat,parseInt,Promise,Proxy,RangeError,ReferenceError,Reflect,RegExp,Set,SharedArrayBuffer,String,Symbol,SyntaxError,TypeError,Uint16Array,Uint32Array,Uint8Array,Uint8ClampedArray,undefined,URIError,WeakMap,WeakSet,Window").split(',').toVector();
}

QVector<QString> JsLanguage::functions(){
    return {"alert","atob","btoa","clearInterval","clearTimeout","decodeURI","decodeURIComponent","encodeURI","encodeURIComponent","eval","isFinite","isNaN","parseFloat","parseInt","setInterval","setTimeout","escape","unescape","adoptNode","addEventListener","alert","anchors","animate","append","assign","atob","back","blur","body","btoa","cancelAnimationFrame","cancelIdleCallback","captureEvents","caretRangeFromPoint","clear","clearInterval","clearTimeout","clientInformation","close","closed","closest","compareDocumentPosition","confirm","console","content","cookie","createAttribute","createComment","createDocumentFragment","createElement","createEvent","createNodeIterator","createRange","createTextNode","crypto","customElements","defaultStatus","designMode","devicePixelRatio","dir","dispatchEvent","document","documentElement","documentMode","documentURI","domain","embeds","evaluate","execCommand","exitFullscreen","exitPointerLock","fetch","focus","forward","frameElement","frames","getComputedStyle","getElementById","getElementsByClassName","getElementsByTagName","getSelection","go","hasFocus","hash","head","history","host","hostname","href","images","importNode","indexedDB","innerHeight","innerWidth","inputEncoding","isSecureContext","lastModified","lastStyleSheetSet","linkColor","links","localStorage","log","locationbar","matchMedia","menubar","moveBy","moveTo","name","navigator","normalize","onabort","onafterprint","onbeforeprint","onbeforeunload","onblur","oncancel","oncanplay","oncanplaythrough","onchange","onclick","onclose","oncontextmenu","oncopy","oncuechange","oncut","ondblclick","ondrag","ondragend","ondragenter","ondragleave","ondragover","ondragstart","ondrop","ondurationchange","onemptied","onended","onerror","onfocus","onhashchange","oninput","oninvalid","onkeydown","onkeypress","onkeyup","onlanguagechange","onload","onloadeddata","onloadedmetadata","onloadend","onloadstart","onmessage","onmessageerror","onmousedown","onmouseenter","onmouseleave","onmousemove","onmouseout","onmouseover","onmouseup","onoffline","ononline","onpagehide","onpageshow","onpaste","onpause","onplay","onplaying","onpointerdown","onpointermove","onpointerup","onpopstate","onprogress","onratechange","onrejectionhandled","onreset","onresize","onscroll","onsearch","onseeked","onseeking","onselect","onstalled","onstorage","onsubmit","onsuspend","ontimeupdate","ontoggle","onunload","onvolumechange","onwaiting","onwheel","open","openDatabase","opener","origin","outerHeight","outerWidth","pageXOffset","pageYOffset","parent","pathname","performance","personalbar","plugins","port","postMessage","print","prompt","protocol","querySelector","querySelectorAll","readyState","referrer","releaseCapture","reload","removeChild","renameNode","replace","requestAnimationFrame","requestFullscreen","requestIdleCallback","requestPointerLock","resizeBy","resizeTo","screen","screenLeft","screenTop","screenX","screenY","scripts","scroll","scrollBy","scrollTo","scrollX","scrollY","scrollbars","scrollingElement","self","selectedStyleSheetSet","sessionStorage","setInterval","setTimeout","sidebar","speechSynthesis","status","statusbar","stop","styleSheets","title","toolbar","top","visibilityState","visualViewport","window","write","writeln"};
}

QVector<QString> JsLanguage::variants(){
    return {"console","document","history","location","window"};
}

QVector<QString> JsLanguage::constants(){
    return {};
}

QVector<QString> JsLanguage::fields(){
    return {};
}


}
