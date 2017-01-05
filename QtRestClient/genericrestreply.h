#ifndef GENERICRESTREPLY_H
#define GENERICRESTREPLY_H

#include "jsonserializer.h"
#include "restclient.h"
#include "restobject.h"
#include "restreply.h"
#include "paging_fwd.h"

#include <type_traits>

namespace QtRestClient {

template <typename DataClassType, typename ErrorClassType = RestObject>
class GenericRestReply : public RestReply
{
	static_assert(std::is_base_of<RestObject, DataClassType>::value, "DataClassType must inherit RestObject!");
	static_assert(std::is_base_of<RestObject, ErrorClassType>::value, "ErrorClassType must inherit RestObject!");
public:
	GenericRestReply(QNetworkReply *networkReply,
					 RestClient *client,
					 QObject *parent = nullptr);

	GenericRestReply<DataClassType, ErrorClassType> *onSucceeded(std::function<void(RestReply*, int, DataClassType*)> handler);
	GenericRestReply<DataClassType, ErrorClassType> *onFailed(std::function<void(RestReply*, int, ErrorClassType*)> handler);
	GenericRestReply<DataClassType, ErrorClassType> *onSerializeException(std::function<void(RestReply*, SerializerException &)> handler);

	//overshadowing, for the right return type only...
	inline GenericRestReply<DataClassType, ErrorClassType> *onError(std::function<void(RestReply*, QString, int, ErrorType)> handler);
	GenericRestReply<DataClassType, ErrorClassType> *enableAutoDelete();

private:
	JsonSerializer *serializer;
	std::function<void(RestReply*, SerializerException &)> exceptionHandler;
};

template <typename DataClassType, typename ErrorClassType>
class GenericRestReply<QList<DataClassType>, ErrorClassType> : public RestReply
{
	static_assert(std::is_base_of<RestObject, DataClassType>::value, "DataClassType must inherit RestObject!");
	static_assert(std::is_base_of<RestObject, ErrorClassType>::value, "ErrorClassType must inherit RestObject!");
public:
	GenericRestReply(QNetworkReply *networkReply,
					 RestClient *client,
					 QObject *parent = nullptr);

	GenericRestReply<QList<DataClassType>, ErrorClassType> *onSucceeded(std::function<void(RestReply*, int, QList<DataClassType*>)> handler);
	GenericRestReply<QList<DataClassType>, ErrorClassType> *onFailed(std::function<void(RestReply*, int, ErrorClassType*)> handler);
	GenericRestReply<QList<DataClassType>, ErrorClassType> *onSerializeException(std::function<void(RestReply*, SerializerException &)> handler);

	//overshadowing, for the right return type only...
	GenericRestReply<QList<DataClassType>, ErrorClassType> *onError(std::function<void(RestReply*, QString, int, ErrorType)> handler);
	GenericRestReply<QList<DataClassType>, ErrorClassType> *enableAutoDelete();

private:
	JsonSerializer *serializer;
	std::function<void(RestReply*, SerializerException &)> exceptionHandler;
};

template <typename DataClassType, typename ErrorClassType>
class GenericRestReply<Paging<DataClassType>, ErrorClassType> : public RestReply
{
	static_assert(std::is_base_of<RestObject, DataClassType>::value, "DataClassType must inherit RestObject!");
	static_assert(std::is_base_of<RestObject, ErrorClassType>::value, "ErrorClassType must inherit RestObject!");
public:
	GenericRestReply(QNetworkReply *networkReply,
					 RestClient *client,
					 QObject *parent = nullptr);

	GenericRestReply<Paging<DataClassType>, ErrorClassType> *onSucceeded(std::function<void(RestReply*, int, Paging<DataClassType>)> handler);
	GenericRestReply<Paging<DataClassType>, ErrorClassType> *onFailed(std::function<void(RestReply*, int, ErrorClassType*)> handler);
	GenericRestReply<Paging<DataClassType>, ErrorClassType> *onSerializeException(std::function<void(RestReply*, SerializerException &)> handler);

	GenericRestReply<Paging<DataClassType>, ErrorClassType> *iterate(std::function<bool(Paging<DataClassType>*, DataClassType*, int)> iterator, int to = -1, int from = 0);

	//overshadowing, for the right return type only...
	GenericRestReply<Paging<DataClassType>, ErrorClassType> *onError(std::function<void(RestReply*, QString, int, ErrorType)> handler);
	GenericRestReply<Paging<DataClassType>, ErrorClassType> *enableAutoDelete();

private:
	RestClient *client;
	std::function<void(RestReply*, int, ErrorClassType*)> failureHandler;
	std::function<void(RestReply*, QString, int, ErrorType)> errorHandler;
	std::function<void(RestReply*, SerializerException &)> exceptionHandler;
};

//include after delecation, to allow foreward declared types
#include "paging.h"

// ------------- Implementation Single Element -------------

template<typename DataClassType, typename ErrorClassType>
GenericRestReply<DataClassType, ErrorClassType>::GenericRestReply(QNetworkReply *networkReply, RestClient *client, QObject *parent) :
	RestReply(networkReply, parent),
	serializer(client->serializer()),
	exceptionHandler()
{}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<DataClassType, ErrorClassType> *GenericRestReply<DataClassType, ErrorClassType>::onSucceeded(std::function<void (RestReply *, int, DataClassType *)> handler)
{
	if(!handler)
		return this;
	connect(this, &RestReply::succeeded, this, [=](int code, const QJsonValue &value){
		try {
			if(!value.isObject())
				throw SerializerException(QStringLiteral("Expected JSON object but got %1").arg(value.type()), true);
			handler(this, code, serializer->deserialize<DataClassType>(value.toObject()));
		} catch(SerializerException &e) {
			if(exceptionHandler)
				exceptionHandler(this, e);
			else
				throw;
		}
	});
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<DataClassType, ErrorClassType> *GenericRestReply<DataClassType, ErrorClassType>::onFailed(std::function<void (RestReply *, int, ErrorClassType *)> handler)
{
	if(!handler)
		return this;
	connect(this, &RestReply::failed, this, [=](int code, const QJsonValue &value){
		try {
			if(!value.isObject())
				throw SerializerException(QStringLiteral("Expected JSON object but got %1").arg(value.type()), true);
			handler(this, code, serializer->deserialize<ErrorClassType>(value.toObject()));
		} catch(SerializerException &e) {
			if(exceptionHandler)
				exceptionHandler(this, e);
			else
				throw;
		}
	});
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<DataClassType, ErrorClassType> *GenericRestReply<DataClassType, ErrorClassType>::onSerializeException(std::function<void (RestReply *, SerializerException &)> handler)
{
	exceptionHandler = handler;
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<DataClassType, ErrorClassType> *GenericRestReply<DataClassType, ErrorClassType>::onError(std::function<void (RestReply *, QString, int, RestReply::ErrorType)> handler)
{
	RestReply::onError(handler);
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<DataClassType, ErrorClassType> *GenericRestReply<DataClassType, ErrorClassType>::enableAutoDelete()
{
	RestReply::enableAutoDelete();
	return this;
}

// ------------- Implementation List of Elements -------------

template<typename DataClassType, typename ErrorClassType>
GenericRestReply<QList<DataClassType>, ErrorClassType>::GenericRestReply(QNetworkReply *networkReply, RestClient *client, QObject *parent) :
	RestReply(networkReply, parent),
	serializer(client->serializer()),
	exceptionHandler()
{}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<QList<DataClassType>, ErrorClassType> *GenericRestReply<QList<DataClassType>, ErrorClassType>::onSucceeded(std::function<void (RestReply *, int, QList<DataClassType*>)> handler)
{
	if(!handler)
		return this;
	connect(this, &RestReply::succeeded, this, [=](int code, const QJsonValue &value){
		try {
			if(!value.isArray())
				throw SerializerException(QStringLiteral("Expected JSON object but got %1").arg(value.type()), true);
			handler(this, code, serializer->deserialize<DataClassType>(value.toArray()));
		} catch(SerializerException &e) {
			if(exceptionHandler)
				exceptionHandler(this, e);
			else
				throw;
		}
	});
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<QList<DataClassType>, ErrorClassType> *GenericRestReply<QList<DataClassType>, ErrorClassType>::onFailed(std::function<void (RestReply *, int, ErrorClassType *)> handler)
{
	if(!handler)
		return this;
	connect(this, &RestReply::failed, this, [=](int code, const QJsonValue &value){
		try {
			if(!value.isObject())
				throw SerializerException(QStringLiteral("Expected JSON object but got %1").arg(value.type()), true);
			handler(this, code, serializer->deserialize<ErrorClassType>(value.toObject()));
		} catch(SerializerException &e) {
			if(exceptionHandler)
				exceptionHandler(this, e);
			else
				throw;
		}
	});
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<QList<DataClassType>, ErrorClassType> *GenericRestReply<QList<DataClassType>, ErrorClassType>::onSerializeException(std::function<void (RestReply *, SerializerException &)> handler)
{
	exceptionHandler = handler;
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<QList<DataClassType>, ErrorClassType> *GenericRestReply<QList<DataClassType>, ErrorClassType>::onError(std::function<void (RestReply *, QString, int, RestReply::ErrorType)> handler)
{
	RestReply::onError(handler);
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<QList<DataClassType>, ErrorClassType> *GenericRestReply<QList<DataClassType>, ErrorClassType>::enableAutoDelete()
{
	RestReply::enableAutoDelete();
	return this;
}

// ------------- Implementation Paging of Elements -------------

template<typename DataClassType, typename ErrorClassType>
GenericRestReply<Paging<DataClassType>, ErrorClassType>::GenericRestReply(QNetworkReply *networkReply, RestClient *client, QObject *parent) :
	RestReply(networkReply, parent),
	client(client),
	exceptionHandler()
{}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<Paging<DataClassType>, ErrorClassType> *GenericRestReply<Paging<DataClassType>, ErrorClassType>::onSucceeded(std::function<void (RestReply *, int, Paging<DataClassType>)> handler)
{
	if(!handler)
		return this;
	connect(this, &RestReply::succeeded, this, [=](int code, const QJsonValue &value){
		try {
			if(!value.isObject())
				throw SerializerException(QStringLiteral("Expected JSON object but got %1").arg(value.type()), true);
			auto iPaging = client->pagingFactory()->createPaging(value.toObject());
			auto data = client->serializer()->deserialize<DataClassType>(iPaging->items());
			handler(this, code, Paging<DataClassType>(iPaging, data, client));
		} catch(SerializerException &e) {
			if(exceptionHandler)
				exceptionHandler(this, e);
			else
				throw;
		}
	});
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<Paging<DataClassType>, ErrorClassType> *GenericRestReply<Paging<DataClassType>, ErrorClassType>::onFailed(std::function<void (RestReply *, int, ErrorClassType *)> handler)
{
	failureHandler = handler;
	if(!handler)
		return this;
	connect(this, &RestReply::failed, this, [=](int code, const QJsonValue &value){
		try {
			if(!value.isObject())
				throw SerializerException(QStringLiteral("Expected JSON object but got %1").arg(value.type()), true);
			handler(this, code, client->serializer()->deserialize<ErrorClassType>(value.toObject()));
		} catch(SerializerException &e) {
			if(exceptionHandler)
				exceptionHandler(this, e);
			else
				throw;
		}
	});
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<Paging<DataClassType>, ErrorClassType> *GenericRestReply<Paging<DataClassType>, ErrorClassType>::onSerializeException(std::function<void (RestReply *, SerializerException &)> handler)
{
	exceptionHandler = handler;
	return this;
}

template<typename DataClassType, typename ErrorClassType>
GenericRestReply<Paging<DataClassType>, ErrorClassType> *GenericRestReply<Paging<DataClassType>, ErrorClassType>::iterate(std::function<bool (Paging<DataClassType>*, DataClassType*, int)> iterator, int to, int from)
{
	return onSucceeded([=](RestReply*, int, Paging<DataClassType> paging){
		paging.iterate(iterator, failureHandler, errorHandler, exceptionHandler, to, from);
	});
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<Paging<DataClassType>, ErrorClassType> *GenericRestReply<Paging<DataClassType>, ErrorClassType>::onError(std::function<void (RestReply *, QString, int, RestReply::ErrorType)> handler)
{
	errorHandler = handler;
	RestReply::onError(handler);
	return this;
}

template<typename DataClassType, typename ErrorClassType>
typename GenericRestReply<Paging<DataClassType>, ErrorClassType> *GenericRestReply<Paging<DataClassType>, ErrorClassType>::enableAutoDelete()
{
	RestReply::enableAutoDelete();
	return this;
}

}

#endif // GENERICRESTREPLY_H
