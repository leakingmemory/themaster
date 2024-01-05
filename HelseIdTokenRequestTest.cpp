//
// Created by sigsegv on 12/31/23.
//

#include "HelseidTokenRequest.h"
#include "JwkPemRsaKey.h"
#include "Rs256.h"
#include "Jwt.h"
#include <iostream>

const std::string jwk = "{\"d\":\"R5CBPBWg_X401Qt6uNKLfGqtmE_MU-B-k18JvdxvoEAP1OAr8wwsRmfcXL-_hsWnXAaPm80kF0ry-HltUzesBg4h_QqRohtDrHFfvN0GAnCjsGgAbj6twwqB5AsuHxQuQ_pBKGYifBgdUqIWQXchEPe0csT_5GNBRk-pygPUC_d908j4iRtKUbTHYf82TxPVf5eK-8Z1_Fc1Vraj9exes5CNaWZWwaYa9gflxq--PjMmZ4u_N20BO_IghoLQPrImSFjQ-e08UARHfRSGPvbYeaffIe-n8kGYVG-hRM4Pb642IFEHQwHBo43e8irYOyf0cq77fZkf8gJZCoAIEwR01Q\",\"dp\":\"jpDkpyOMLoMOLrDugtakbjAW17U3Us4WBFYuIcnN6usbPPh99IQuNGLjF_R_7vAGQ6uSoOjTrQ9yiqoAJE3a_mcphJyCgjqENx2PFyowZP1RWQMswzc3EqoVB5l2I7bDn3amuqn-4ngrdb5BFdrwH_ihUqTYoX0AoanvuQhn_rc\",\"dq\":\"x9b06kqcmTlCpz3mdDwgJbyTPbEjqRAv0_TNTbgWALOibEvQyiHMwXXr8igrUEV-7hzID6xvNMaKonjDyZL3tR-oYL7eCd6QDikYOWNdtlCsBZ21aiQqYdIvsCWRGl9xJbsULb45zXiYHEDv56RPH4qAUQadV2j2KwoKnBBYoW0\",\"e\":\"AQAB\",\"kty\":\"RSA\",\"n\":\"zsgiUX0Y6rW1cVP0Jv-ow7Dyp0BQOPCGfb25wBALP9QePIfhBTiz2Wxpfh_0slIz53Btu9NH34dNe4g5Eq9mBfjyP8PZbfiazjWI9iZORHYbFrMkIyMHSEzZv_W7HrhiSWGDY_yfu38NxcmkUBVtn5vVv3HoUyaorODPbIXznb1R5jZJw8osOeCluM_WXiJ6xGD0EXl6r9E3b76hAlNDjuwxY5XoQ2TxmkykuZcfbtBfF5m0jmWC6SYMC4eb2G3sGfoEItKUHGZtQ9q1GfheuTd_eyomFg-AX9kUySxO5bp2fhjrTo2w83GuxynAj7ZxttasCOFY0kB7gUt3houm_w\",\"p\":\"7y_GzRZIt9xGWAIGv6U9-2cgVxwWrOC9EluRnMQobURVpFCFxugaox_JppFr-oVfNsr6OsFYfcupOBT9WQroMVlX9AiQ9JwzDKqdlzECx6T5T59Bace4ZT8Mo7_W4LnadoP9NCatB2s_cas6qUcIjuyNFE957iwKWIdTen4cB7s\",\"q\":\"3VE5WLqIVDoJgr9LbKrcjnMnYXrBmSckJy3UsLb0Ro46Auznx0njPwUUmuf1Rf5ypWYz3uq3LUvn5l7CCAy06fcEXd78wR2PLBmV7a6ScUqEUzj3CBMoZodeAoXuC_xCoCQavP8Kv7IWOw3zua1pRc6fUyDSr2mDhgZdycpnX40\",\"qi\":\"Jn6Cvw745msJme3N2YIAUehcalOa3ga_y0U-RgCdXd8Q7wMDEMWj4pIQGfvpX_kGWocY3BQUYogRnoXKBLVYf4J-Qr2S9ry9imhidFShFfB1sNXGWygRAaWBRBMW-qcs7nm_JnQbOMQV_8zlTBW4j6odJW-sBgjpjlhAXj8PdDg\"}";

int main() {
    HelseidTokenRequest requestGenerator{"https://testing.radiotube.org", "client-id", jwk, "app://appredirect", "code", {"scope1", "scope2"}, "verifier"};
    auto request = requestGenerator.GetTokenRequest();
    JwkPemRsaKey jwkobj{};
    jwkobj.FromJwk(jwk);
    std::cout << "Jwk (private):" << jwkobj.ToJwk() << "\n";
    std::cout << "Jwk (public): " << jwkobj.ToPublicJwk() << "\n";
    std::cout << jwkobj.ToPublicPem() << "\n";
    std::string jwt{};
    std::cout << "Url: " << request.url << "\nParams:\n";
    for (const auto &p : request.params) {
        if (p.first == "client_assertion") {
            jwt = p.second;
        }
        std::cout << " " << p.first << ": " << p.second << "\n";
    }
    if (jwt.empty()) {
        std::cerr << "No jwt\n";
        return 1;
    }
    Rs256 rs256{jwkobj.ToVerificationKey()};
    {
        Jwt jwtobj{jwt};
        if (!rs256.Verify(jwtobj)) {
            std::cerr << "Verify failed\n";
            return 1;
        }
        std::cout << "Verify ok\n";
    }
    {
        Jwt jwtobj{jwt};
        std::string signature{jwtobj.GetSignature()};
        signature[0] = signature.at(0) == 'A' ? 'B' : 'A';
        jwtobj.SetSignature(signature);
        if (rs256.Verify(jwtobj)) {
            std::cerr << "Verify invalid succeeded\n";
            return 1;
        }
        std::cout << "Verify invalid failed as expected\n";
    }
}