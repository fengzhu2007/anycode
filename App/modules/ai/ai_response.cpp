#include "ai_response.h"


namespace ady{
AIResponse::AIResponse():HttpResponse() {


}

void AIResponse::parse(){
    HttpResponse::parse();
}

QString AIResponse::suggestion(){

    return {};
}

}
