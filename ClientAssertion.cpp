//
// Created by sigsegv on 11/4/25.
//

#include "ClientAssertion.h"
#include <jjwtid/SigningKey.h>
#include <memory>
#include <jjwtid/JwkPemRsaKey.h>
#include <jjwtid/Jwt.h>
#include <jjwtid/Rs256.h>
#include "Uuid.h"
#include <ctime>

ClientAssertion::ClientAssertion(const std::string &audienceUrl, const std::string &clientId, const std::string &jwk,
                                 const HelseidMultiTenantInfo &multiTenantInfo)
    : url(audienceUrl), clientId(clientId), jwk(jwk), helseidMultiTenantInfo(multiTenantInfo) {}

std::string ClientAssertion::ToString() const {
    std::string jwt{};
    {
        std::shared_ptr<SigningKey> signingKey{};
        {
            JwkPemRsaKey rsa{};
            rsa.FromJwk(jwk);
            signingKey = rsa.ToSigningKey();
        }
        auto iat = std::time(nullptr);
        Jwt token{JwtType::CLIENT_AUTHENTICATION};
        {
            std::string uuidStr = Uuid::RandomUuidString();
            token.Body()->Add("jti", uuidStr);
        }
        token.Body()->Add("iss", clientId);
        token.Body()->Add("iat", iat);
        token.Body()->Add("nbf", iat);
        token.Body()->Add("exp", iat + 120);
        token.Body()->Add("sub", clientId);
        token.Body()->Add("aud", url);
        if (helseidMultiTenantInfo.IsSet()) {
            JwtPartArray arr{};
            if (!helseidMultiTenantInfo.journalId.empty()) {
                JwtPartObject jid{};
                jid.Add("type", "nhn:sfm:journal-id");
                JwtPartObject val{};
                val.Add("journal_id", helseidMultiTenantInfo.journalId);
                jid.Add("value", val);
                arr.Add(jid);
            }
            if (!helseidMultiTenantInfo.consumerOrgNo.empty() || !helseidMultiTenantInfo.consumerChildOrgNo.empty()) {
                std::string strval{"NO:ORGNR"};
                if (!helseidMultiTenantInfo.consumerOrgNo.empty()) {
                    strval.append(":");
                    strval.append(helseidMultiTenantInfo.consumerOrgNo);
                }
                if (!helseidMultiTenantInfo.consumerChildOrgNo.empty()) {
                    strval.append(":");
                    strval.append(helseidMultiTenantInfo.consumerChildOrgNo);
                }
                JwtPartObject auth{};
                auth.Add("type", "helseid_authorization");
                JwtPartObject pr{};
                JwtPartObject org{};
                JwtPartObject identifier{};
                identifier.Add("system", "urn:oid:1.0.6523");
                identifier.Add("type", "ENH");
                identifier.Add("value", strval);
                org.Add("identifier", identifier);
                pr.Add("organization", org);
                auth.Add("practitioner_role", pr);
                arr.Add(auth);
            }
            token.Body()->Add("authorization_details", arr);
        }
        Rs256 rs256{signingKey};
        rs256.Sign(token);
        jwt = token.ToString();
    }
    return jwt;
}
