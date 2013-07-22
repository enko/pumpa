/**
 * KQOAuth - An OAuth authentication library for Qt.
 *
 * Author: Johan Paul (johan.paul@gmail.com)
 *         http://www.johanpaul.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  KQOAuth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with KQOAuth.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KQOAUTHREQUEST_P_H
#define KQOAUTHREQUEST_P_H
#include "kqoauthglobals.h"
#include "kqoauthrequest.h"

#include <QString>
#include <QUrl>
#include <QMap>
#include <QPair>
#include <QMultiMap>
#include <QTimer>

class KQOAUTH_EXPORT KQOAuthRequestPrivate {

public:
    KQOAuthRequestPrivate();
    ~KQOAuthRequestPrivate();

    // Helper methods to get the values for the OAuth request parameters.
    QString oauthTimestamp() const;
    QString oauthNonce() const;
    QString oauthSignature();

    // Utility methods for making the request happen.
    void prepareRequest();
    void signRequest();
    bool validateRequest() const;
    QByteArray requestBaseString();
    QByteArray encodedParamaterList(const QList< QPair<QString, QString> > &requestParameters);
    void insertAdditionalParams();
    void insertPostBody();

    QUrl oauthRequestEndpoint;
    KQOAuthRequest::RequestHttpMethod oauthHttpMethod;
    QString oauthHttpMethodString;
    QString oauthConsumerKey;
    QString oauthConsumerSecretKey;
    QString oauthToken;
    QString oauthTokenSecret;
    QString oauthSignatureMethod;
    QUrl oauthCallbackUrl;
    QString oauthVersion;
    QString oauthVerifier;

    // These will be generated by the helper methods
    QString oauthTimestamp_;
    QString oauthNonce_;

    // User specified additional parameters needed for the request.
    QList< QPair<QString, QString> > additionalParameters;

     // The raw POST body content as given to the HTTP request.
     QByteArray postBodyContent;

    // Protocol parameters.
    // These parameters are used in the "Authorized" header of the HTTP request.
    QList< QPair<QString, QString> > requestParameters;

    KQOAuthRequest::RequestType requestType;

    //The Content-Type HTTP header
    QString contentType;


    //The Content-Length HTTP header
    int contentLength;

    //Raw data to post if type is not url-encoded
    QByteArray postRawData;

    // Timeout for this request in milliseconds.
    int timeout;
    QTimer timer;

    bool debugOutput;

};
#endif // KQOAUTHREQUEST_P_H
