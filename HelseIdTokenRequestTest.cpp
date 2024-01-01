//
// Created by sigsegv on 12/31/23.
//

#include "HelseidTokenRequest.h"
#include "JwkPemRsaKey.h"
#include <iostream>

const std::string jwk = "{\"d\":\"QVI8PFk_lRYGHDz5arVimx5ldPQJU1Lp1Nujc9SKz_OvON9_HSj8LYh8Fj3T7pedeLPA6E_54cM-of2r8yVu6IkMHvgDSu2WI8lx0IF3uYOU3k3HCxMeGzpKtS6lm1NbnbxT7XhyYUrXTqL04idNQz30wepyiG_xTnQ-zuqIVAP7LlGU1AyHjUsxICBIsiUrEq9g61y86TWx7v43egG13ui4KaBVdgMCTeORTyLlUD_gQTMzP0MoNKZq7pGzroKRkcrbPsIOcHSrPtydn_itsUpo7bWRPJWD6-yp4TNje-8iR2avSLhsCgyiZKdynL2NKRlQJmTXiTZL1YCbmRCi\",\"dp\":\"oQeH3Si7GZEfgJ-PInaFHedDqPa9WKSj_O_oXzFZ3nmmYIfzszjniIXcViVc393MsrwxUoQkqfW3KPQ4FHk6BrSydpmi-8OUbwnOHVCLXcYLR9ZOUqY2UUQY95abzcnIxCkEwrMS-5j26KRMdW5-hQon0FqflAVx1zOQjTup-g\",\"dq\":\"mUW8COAL3gvSRNV0LT-6IQWBhjqLnzpf4dOwWhzCJ9zxlsi8jJVHmGMBYamWDjM6iXox7SGXKdvg0zBlyfvjAnYHiyWuLRPNwz6bWAwiOYADl354gGokAgQLm4ne2EE1yv1EIxvblbpwUr3AAid9ed2ZuAnV3DG2fWLksdz5cik\",\"e\":\"AQAB\",\"kty\":\"RSA\",\"n\":\"3fzqAi5HV3icKebq0ncgVKTQEk6tq8yVrOpp67SK7-XzeWDoKGunwm8Ip0QQ2_HStGup6Lu_gp0I7Rzp-DHwR15glMTc0htN2P0H0sng_0NmgCg97bz_pf9zBOpwI7EYgsbA527ZqLIYgOplJxtJ7cF0IKAyqC6BRl4gqDWRQv7yPmSFRp-8sKHvM5020YtZeHj7zPbrjXjr9NrfDbyB7d7a5zdmS34VQ09XubHH6wE6cxIzKaP5JnLNLYkW_SgfeV6YTb2PLLkuGi_woPKaQS19e-vDkHFZDjObmd9V7XwSjO77xMhRfe5_DPXkC3Ki7Npxc8WZP8pFtLLJKUFSkQ\",\"p\":\"oWQVI63tIEQWppQq_kfQPSmn257EV69W7-cap75QFYEfVP_8NjFXDyrCZ5udZLcsKgbFDsS_W_qn2bK_DqDjsb1ZpZKWxR3MChkXY80eyEW0ufefjYjyRzspUazAQb0nTis0DPEe0QzyKp8LbO9HfHnYYlFsw2bIKAsq_j1DBM0\",\"q\":\"vXJQQ_YyEblZnfe5FCzHxX2EdLsg8C5fW_K3OfuhSv0L956qRuMZSsu5clfrl-eFjy586WsbVPYfBe8Qpi2gxBiFUdMCI_VWwUUYfnFTt_WmgnmzAKGbMJMD871nOAurwRclZMD-wEB26XGTvVEy5YCltPB9ZVzsTR5XYyitdbU\",\"qi\":\"T9Y1gWd9iqh-qE0Mbzd0dHMef_olK1gMaSFZrKjyGmvnHMdIzi5NH8y0zU7gdqSMNe8h7GgENRcobKdox-ABGYZ2P__xZ8oCFttgMba7icRGW9BK8WyrWUJmlfzD8VFZw646t8WerOsX_xP9bUV_okByRzinIMoclap3IVLiCLo\"}";

int main() {
    HelseidTokenRequest requestGenerator{"https://testing.radiotube.org", "client-id", jwk, "app://appredirect", "code", {"scope1", "scope2"}, "verifier"};
    auto request = requestGenerator.GetTokenRequest();
    JwkPemRsaKey jwkobj{};
    jwkobj.FromJwk(jwk);
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
}