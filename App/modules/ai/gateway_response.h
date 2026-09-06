#ifndef GATEWAY_RESPONSE_H
#define GATEWAY_RESPONSE_H
#include "network/http/http_response.h"

namespace ady{
class GatewayResponse : public HttpResponse
{
public:
    GatewayResponse();
    virtual void parse() override;
    QString suggestion();
};
}

#endif // GATEWAY_RESPONSE_H
