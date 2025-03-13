#ifndef AI_RESPONSE_H
#define AI_RESPONSE_H
#include "network/http/http_response.h"

namespace ady{
class AIResponse : public HttpResponse
{
public:
    AIResponse();
    virtual void parse() override;
    QString suggestion();
};
}

#endif // AI_RESPONSE_H
