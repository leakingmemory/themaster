//
// Created by sigsegv on 12/31/23.
//

#include "HelseidTokenRequest.h"
#include <iostream>

const std::string jwk = "{\"d\":\"B7ZPKRMu9szDGjead9aFD27alz6Y8NyDX9_8wdnNEJ1Phto4om6mU8BoRq1A-LxU2y3fNHYYA18GGykfuXK5redAul49Ev6A077xWR6Ida0oxTXkB3BKSiAm5oN69N1XoxYMcq24pHpMhpYTHcqfR3xndelN5NzkcDzOhsBemdtwbUo4MRO-kdNBimuNvXSy6nTM39Apq7GNsvMJGuv0mb7DFr6Q8cH-K9PZUuYIm9DJffVdo2KpTvRvS6mo6UyyA0VekBHlPY2LPJ1L9kI-cJiP9jTzQzV1mcj4dWlgu8GmZE1HNLBM06SUjG7UiLKWkUKed7yIlALwk39YYotaEA\",\"dp\":\"X6Ce2mUPzC-D7XSgRGy6AYsvDh1adpjGGlSGymIDsKlEo8exkVaOaRnRc3D8kGhKt8gsORk-ktr6MPSqx8n7TRgwQ4SfIhtc_RkeNTpd1Rro6J1xb4CBd9lThpqIF7SyIEBPRo2QAp6LqzVgl5XJ28UY-92rpkSeRMx_m6bYeio\",\"dq\":\"5VnXSYkBTUejrq2lr5h_5JiIyh7QEUnGa5B5HE7bm5ogBikcgwHnVX7NQXVnqZkaIgF1K67y-oWJ674Fo7L6BgtWtm5RD_wIRFktLHc616d-do_J_-A3xJcslzxbhynL8fxeqF5v081UtE8SxBoW31Gv3Zk3mwfNsgkQxhYTMG4\",\"e\":\"AQAB\",\"kty\":\"RSA\",\"n\":\"PXVLxGo1t4MQEsZnOLV3nmS1jJIHMJiK7kqi9m2yVKI1DbH_U21YjcS2fdwBoENy-_5YRV0LoNZZKuicSar0C38coUljHQzI5ocuO9J-2HkiIXRoiHTDqSGqk0nXUugBqjVI6ir7cB2x19J8JrDX4xeUF-PnSH6oofzk7kgOU2_QmvxEh6PUtebb1-HvbYZLHCGu-420Kmbbxqr6tDOL8toqgTKF9u65Hstp59iG4fWi5fV7onGsFcMKYR_fd5aTq9B5uLpmJ3KtVzKp5R0gsgEdnXvpF08UMqBzSbei2sH-ALspMMknhBCH4bvtOMM58L-vi_jpT8F_JHMOGKAEqQ\",\"p\":\"l7T00JtUaHRyrNu0Gk4qfUCh8VuDNUBjzJvsWiywJ3ZuGBLzCjgcDKzuk8t2ivNcl7VTHs4vsYEgPNJJIkKXJxJxZUEQY8HulrIIGjTIcl7Mbb8uqaD_FgpDffRLbxFG_3cfrMRe1iSV_VAnVbBFbzUe8_ENh1Vc5FR84T_LgNQ\",\"q\":\"S3uaQTCrxYh-Jqax-UMX1wW6rCtMzIFpdq0nUhVLVHxy3mQOvTuP4qF0HRUG4vgC0AF2iGGvYHdxQaNcoBhhKw6F4LpXGk7ilEiR1-YZYS8FiGV7lIj7ZwGrKhWziQC0BYWihT4tH2NaXNGZb5RnfgDhPq2inheQ4Ij4CO40ncs\",\"qi\":\"3ZthvpofKKtpaLBklCo_8doxVu_de6YwPUISVOLRSqq_Y_I4ll1AqLxPl6ILmmQUuK4NubpSnobm74b_4EGMfzgDJzrqPHrfrjyM-aRlsdkCguoygiL4_D_8ab7X2AD4b-IRmQ6evI6RFJT4HzUYg0Qp492EmGu84U8ILeD0CA4\"}";

int main() {
    HelseidTokenRequest requestGenerator{"https://testing.radiotube.org", "client-id", jwk, "app://appredirect", "code", {"scope1", "scope2"}, "verifier"};
    auto request = requestGenerator.GetTokenRequest();
    std::cout << "Url: " << request.url << "\nParams:\n";
    for (const auto &p : request.params) {
        std::cout << " " << p.first << ": " << p.second << "\n";
    }
}