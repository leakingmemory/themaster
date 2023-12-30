//
// Created by sigsegv on 12/30/23.
//

#include <string>
#include "JwkPemRsaKey.h"
#include <iostream>

const std::string jwk = "{\"d\":\"B7ZPKRMu9szDGjead9aFD27alz6Y8NyDX9_8wdnNEJ1Phto4om6mU8BoRq1A-LxU2y3fNHYYA18GGykfuXK5redAul49Ev6A077xWR6Ida0oxTXkB3BKSiAm5oN69N1XoxYMcq24pHpMhpYTHcqfR3xndelN5NzkcDzOhsBemdtwbUo4MRO-kdNBimuNvXSy6nTM39Apq7GNsvMJGuv0mb7DFr6Q8cH-K9PZUuYIm9DJffVdo2KpTvRvS6mo6UyyA0VekBHlPY2LPJ1L9kI-cJiP9jTzQzV1mcj4dWlgu8GmZE1HNLBM06SUjG7UiLKWkUKed7yIlALwk39YYotaEA\",\"dp\":\"X6Ce2mUPzC-D7XSgRGy6AYsvDh1adpjGGlSGymIDsKlEo8exkVaOaRnRc3D8kGhKt8gsORk-ktr6MPSqx8n7TRgwQ4SfIhtc_RkeNTpd1Rro6J1xb4CBd9lThpqIF7SyIEBPRo2QAp6LqzVgl5XJ28UY-92rpkSeRMx_m6bYeio\",\"dq\":\"5VnXSYkBTUejrq2lr5h_5JiIyh7QEUnGa5B5HE7bm5ogBikcgwHnVX7NQXVnqZkaIgF1K67y-oWJ674Fo7L6BgtWtm5RD_wIRFktLHc616d-do_J_-A3xJcslzxbhynL8fxeqF5v081UtE8SxBoW31Gv3Zk3mwfNsgkQxhYTMG4\",\"e\":\"AQAB\",\"kty\":\"RSA\",\"n\":\"PXVLxGo1t4MQEsZnOLV3nmS1jJIHMJiK7kqi9m2yVKI1DbH_U21YjcS2fdwBoENy-_5YRV0LoNZZKuicSar0C38coUljHQzI5ocuO9J-2HkiIXRoiHTDqSGqk0nXUugBqjVI6ir7cB2x19J8JrDX4xeUF-PnSH6oofzk7kgOU2_QmvxEh6PUtebb1-HvbYZLHCGu-420Kmbbxqr6tDOL8toqgTKF9u65Hstp59iG4fWi5fV7onGsFcMKYR_fd5aTq9B5uLpmJ3KtVzKp5R0gsgEdnXvpF08UMqBzSbei2sH-ALspMMknhBCH4bvtOMM58L-vi_jpT8F_JHMOGKAEqQ\",\"p\":\"l7T00JtUaHRyrNu0Gk4qfUCh8VuDNUBjzJvsWiywJ3ZuGBLzCjgcDKzuk8t2ivNcl7VTHs4vsYEgPNJJIkKXJxJxZUEQY8HulrIIGjTIcl7Mbb8uqaD_FgpDffRLbxFG_3cfrMRe1iSV_VAnVbBFbzUe8_ENh1Vc5FR84T_LgNQ\",\"q\":\"S3uaQTCrxYh-Jqax-UMX1wW6rCtMzIFpdq0nUhVLVHxy3mQOvTuP4qF0HRUG4vgC0AF2iGGvYHdxQaNcoBhhKw6F4LpXGk7ilEiR1-YZYS8FiGV7lIj7ZwGrKhWziQC0BYWihT4tH2NaXNGZb5RnfgDhPq2inheQ4Ij4CO40ncs\",\"qi\":\"3ZthvpofKKtpaLBklCo_8doxVu_de6YwPUISVOLRSqq_Y_I4ll1AqLxPl6ILmmQUuK4NubpSnobm74b_4EGMfzgDJzrqPHrfrjyM-aRlsdkCguoygiL4_D_8ab7X2AD4b-IRmQ6evI6RFJT4HzUYg0Qp492EmGu84U8ILeD0CA4\"}";
const std::string pem = "-----BEGIN RSA PRIVATE KEY-----\n"
                        "MIIEogIBAAKCAQEAqQSgGA5zJH/BT+n4i6+/8DnDOO274YcQhCfJMCm7AP7B2qK3\n"
                        "SXOgMhRPF+l7nR0BsiAd5akyV61yJ2a6uHnQq5OWd98fYQrDFaxxonv15aL14YbY\n"
                        "52nLHrnu9oUygSra8osztPqqxttmKrSN+64hHEuGbe/h19vmtdSjh0T8mtBvUw5I\n"
                        "7uT8oah+SOfjF5QX49ewJnzS17EdcPsq6kg1qgHoUtdJk6ohqcN0iGh0ISJ52H7S\n"
                        "Oy6H5sgMHWNJoRx/C/SqSZzoKlnWoAtdRVj++3JDoAHcfbbEjVhtU/+xDTWiVLJt\n"
                        "9qJK7oqYMAeSjLVknne1OGfGEhCDtzVqxEt1PQIDAQABAoIBABBai2JYf5PwApSI\n"
                        "vHeeQpGWsojUboyUpNNMsDRHTWSmwbtgaXX4yJl1NUPzNPaPmHA+QvZLnTyLjT3l\n"
                        "EZBeRQOyTOmoqUtv9E6pYqNd9X3J0JsI5lLZ0yv+wfGQvhbDvpn06xoJ87KNsasp\n"
                        "0N/MdOqydL2Na4pB05G+EzE4Sm1w25lewIbOPHDk3ORN6XVnfEefyh0TloZMeqS4\n"
                        "rXIMFqNX3fR6g+YmIEpKcAfkNcUorXWIHlnxvtOA/hI9XrpA5625crkfKRsGXwMY\n"
                        "djTfLdtUvPhArUZowFOmbqI42oZPnRDN2cH831+D3PCYPpfabg+F1neaNxrDzPYu\n"
                        "EylPtgcCgYEA1IDLP+F8VORcVYcN8fMeNW9FsFUnUP2VJNZexKwfd/9GEW9L9H1D\n"
                        "Chb/oKkuv23MXnLINBoIspbuwWMQQWVxEieXQiJJ0jwggbEvzh5TtZdc84p2y5Pu\n"
                        "rAwcOArzEhhudiewLFrsm8xjQDWDW/GhQH0qThq026xydGhUm9D0tJcCgYEAy500\n"
                        "7gj4iOCQF56irT7hAH5nlG+Z0VxaYx8tPoWihQW0AImzFSqrAWf7iJR7ZYgFL2EZ\n"
                        "5teRSJTiThpXuuCFDithGKBco0Fxd2CvYYh2AdAC+OIGFR10oeKPO70OZN5yfFRL\n"
                        "FVInrXZpgcxMK6y6BdcXQ/mxpiZ+iMWrMEGae0sCgYAqetimm3/MRJ5Epqvd+xjF\n"
                        "28mVl2A1q4ueApCNRk9AILK0F4iahlPZd4GAb3Gd6Oga1V06NR4Z/VwbIp+EQzAY\n"
                        "TfvJx6r0MPrakj4ZOSzIt0pokPxwc9EZaY5WkbHHo0SpsANiyoZUGsaYdlodDi+L\n"
                        "AbpsRKB07YMvzA9l2p6gXwKBgG4wExbGEAmyzQebN5ndr1HfFhrEEk+0VM3Tb16o\n"
                        "XvzxyymHWzyXLJfEN+D/yY92fqfXOncsLVlECPwPUW62VgsG+rKjBb7riYX68q4r\n"
                        "dQEiGpmpZ3VBzX5V5wGDHCkGIJqb204ceZBrxkkR0B7KiJjkf5ivpa2uo0dNAYlJ\n"
                        "11nlAoGADgj04C0IT+G8a5iE3eMpRIMYNR/4lBSRjryeDpkR4m/4ANjXvmn8P/z4\n"
                        "IoIy6oIC2bFlpPmMPK7fejzqOicDOH+MQeD/hu/mhp5SurkNrrgUZJoLopdPvKhA\n"
                        "XZY48mO/qkrR4lQSQj0wpnvd71Yx2vE/KpRksGhpqygfmr5hm90=\n"
                        "-----END RSA PRIVATE KEY-----\n";

int main() {
    JwkPemRsaKey jwkKey{};
    jwkKey.FromJwk(jwk);
    auto outputJwk = jwkKey.ToJwk();
    if (outputJwk != jwk) {
        std::cerr << "Got:\n";
        std::cerr << outputJwk;
        std::cerr << "Expected:\n";
        std::cerr << jwk;
        return 1;
    }
    auto outputPem = jwkKey.ToTraditionalPrivatePem();
    if (outputPem != pem) {
        std::cerr << "Got:\n";
        std::cerr << outputPem;
        std::cerr << "Expected:\n";
        std::cerr << pem;
        return 1;
    }
    return 0;
}