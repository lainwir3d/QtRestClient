#ifndef QRESTCLIENT_P_H
#define QRESTCLIENT_P_H

#include "restclient.h"

namespace QtRestClient {

class RestClientPrivate
{
public:
	QUrl baseUrl;
	QVersionNumber apiVersion;
	HeaderHash headers;
	QUrlQuery query;
};

}

#endif // QRESTCLIENT_P_H
