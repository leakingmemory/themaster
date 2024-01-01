//
// Created by sigsegv on 12/30/23.
//

#include <string>
#include "JwkPemRsaKey.h"
#include <iostream>

const std::string jwk = "{\"d\":\"QVI8PFk_lRYGHDz5arVimx5ldPQJU1Lp1Nujc9SKz_OvON9_HSj8LYh8Fj3T7pedeLPA6E_54cM-of2r8yVu6IkMHvgDSu2WI8lx0IF3uYOU3k3HCxMeGzpKtS6lm1NbnbxT7XhyYUrXTqL04idNQz30wepyiG_xTnQ-zuqIVAP7LlGU1AyHjUsxICBIsiUrEq9g61y86TWx7v43egG13ui4KaBVdgMCTeORTyLlUD_gQTMzP0MoNKZq7pGzroKRkcrbPsIOcHSrPtydn_itsUpo7bWRPJWD6-yp4TNje-8iR2avSLhsCgyiZKdynL2NKRlQJmTXiTZL1YCbmRCi\",\"dp\":\"oQeH3Si7GZEfgJ-PInaFHedDqPa9WKSj_O_oXzFZ3nmmYIfzszjniIXcViVc393MsrwxUoQkqfW3KPQ4FHk6BrSydpmi-8OUbwnOHVCLXcYLR9ZOUqY2UUQY95abzcnIxCkEwrMS-5j26KRMdW5-hQon0FqflAVx1zOQjTup-g\",\"dq\":\"mUW8COAL3gvSRNV0LT-6IQWBhjqLnzpf4dOwWhzCJ9zxlsi8jJVHmGMBYamWDjM6iXox7SGXKdvg0zBlyfvjAnYHiyWuLRPNwz6bWAwiOYADl354gGokAgQLm4ne2EE1yv1EIxvblbpwUr3AAid9ed2ZuAnV3DG2fWLksdz5cik\",\"e\":\"AQAB\",\"kty\":\"RSA\",\"n\":\"3fzqAi5HV3icKebq0ncgVKTQEk6tq8yVrOpp67SK7-XzeWDoKGunwm8Ip0QQ2_HStGup6Lu_gp0I7Rzp-DHwR15glMTc0htN2P0H0sng_0NmgCg97bz_pf9zBOpwI7EYgsbA527ZqLIYgOplJxtJ7cF0IKAyqC6BRl4gqDWRQv7yPmSFRp-8sKHvM5020YtZeHj7zPbrjXjr9NrfDbyB7d7a5zdmS34VQ09XubHH6wE6cxIzKaP5JnLNLYkW_SgfeV6YTb2PLLkuGi_woPKaQS19e-vDkHFZDjObmd9V7XwSjO77xMhRfe5_DPXkC3Ki7Npxc8WZP8pFtLLJKUFSkQ\",\"p\":\"oWQVI63tIEQWppQq_kfQPSmn257EV69W7-cap75QFYEfVP_8NjFXDyrCZ5udZLcsKgbFDsS_W_qn2bK_DqDjsb1ZpZKWxR3MChkXY80eyEW0ufefjYjyRzspUazAQb0nTis0DPEe0QzyKp8LbO9HfHnYYlFsw2bIKAsq_j1DBM0\",\"q\":\"vXJQQ_YyEblZnfe5FCzHxX2EdLsg8C5fW_K3OfuhSv0L956qRuMZSsu5clfrl-eFjy586WsbVPYfBe8Qpi2gxBiFUdMCI_VWwUUYfnFTt_WmgnmzAKGbMJMD871nOAurwRclZMD-wEB26XGTvVEy5YCltPB9ZVzsTR5XYyitdbU\",\"qi\":\"T9Y1gWd9iqh-qE0Mbzd0dHMef_olK1gMaSFZrKjyGmvnHMdIzi5NH8y0zU7gdqSMNe8h7GgENRcobKdox-ABGYZ2P__xZ8oCFttgMba7icRGW9BK8WyrWUJmlfzD8VFZw646t8WerOsX_xP9bUV_okByRzinIMoclap3IVLiCLo\"}";
const std::string pem = "-----BEGIN RSA PRIVATE KEY-----\n"
                        "MIIEoQIBAAKCAQEA3fzqAi5HV3icKebq0ncgVKTQEk6tq8yVrOpp67SK7+XzeWDo\n"
                        "KGunwm8Ip0QQ2/HStGup6Lu/gp0I7Rzp+DHwR15glMTc0htN2P0H0sng/0NmgCg9\n"
                        "7bz/pf9zBOpwI7EYgsbA527ZqLIYgOplJxtJ7cF0IKAyqC6BRl4gqDWRQv7yPmSF\n"
                        "Rp+8sKHvM5020YtZeHj7zPbrjXjr9NrfDbyB7d7a5zdmS34VQ09XubHH6wE6cxIz\n"
                        "KaP5JnLNLYkW/SgfeV6YTb2PLLkuGi/woPKaQS19e+vDkHFZDjObmd9V7XwSjO77\n"
                        "xMhRfe5/DPXkC3Ki7Npxc8WZP8pFtLLJKUFSkQIDAQABAoH/QVI8PFk/lRYGHDz5\n"
                        "arVimx5ldPQJU1Lp1Nujc9SKz/OvON9/HSj8LYh8Fj3T7pedeLPA6E/54cM+of2r\n"
                        "8yVu6IkMHvgDSu2WI8lx0IF3uYOU3k3HCxMeGzpKtS6lm1NbnbxT7XhyYUrXTqL0\n"
                        "4idNQz30wepyiG/xTnQ+zuqIVAP7LlGU1AyHjUsxICBIsiUrEq9g61y86TWx7v43\n"
                        "egG13ui4KaBVdgMCTeORTyLlUD/gQTMzP0MoNKZq7pGzroKRkcrbPsIOcHSrPtyd\n"
                        "n/itsUpo7bWRPJWD6+yp4TNje+8iR2avSLhsCgyiZKdynL2NKRlQJmTXiTZL1YCb\n"
                        "mRCiAoGBAKFkFSOt7SBEFqaUKv5H0D0pp9uexFevVu/nGqe+UBWBH1T//DYxVw8q\n"
                        "wmebnWS3LCoGxQ7Ev1v6p9myvw6g47G9WaWSlsUdzAoZF2PNHshFtLn3n42I8kc7\n"
                        "KVGswEG9J04rNAzxHtEM8iqfC2zvR3x52GJRbMNmyCgLKv49QwTNAoGBAL1yUEP2\n"
                        "MhG5WZ33uRQsx8V9hHS7IPAuX1vytzn7oUr9C/eeqkbjGUrLuXJX65fnhY8ufOlr\n"
                        "G1T2HwXvEKYtoMQYhVHTAiP1VsFFGH5xU7f1poJ5swChmzCTA/O9ZzgLq8EXJWTA\n"
                        "/sBAdulxk71RMuWApbTwfWVc7E0eV2MorXW1AoGAAKEHh90ouxmRH4CfjyJ2hR3n\n"
                        "Q6j2vViko/zv6F8xWd55pmCH87M454iF3FYlXN/dzLK8MVKEJKn1tyj0OBR5Oga0\n"
                        "snaZovvDlG8Jzh1Qi13GC0fWTlKmNlFEGPeWm83JyMQpBMKzEvuY9uikTHVufoUK\n"
                        "J9Ban5QFcdczkI07qfoCgYEAmUW8COAL3gvSRNV0LT+6IQWBhjqLnzpf4dOwWhzC\n"
                        "J9zxlsi8jJVHmGMBYamWDjM6iXox7SGXKdvg0zBlyfvjAnYHiyWuLRPNwz6bWAwi\n"
                        "OYADl354gGokAgQLm4ne2EE1yv1EIxvblbpwUr3AAid9ed2ZuAnV3DG2fWLksdz5\n"
                        "cikCgYBP1jWBZ32KqH6oTQxvN3R0cx5/+iUrWAxpIVmsqPIaa+ccx0jOLk0fzLTN\n"
                        "TuB2pIw17yHsaAQ1Fyhsp2jH4AEZhnY///FnygIW22AxtruJxEZb0ErxbKtZQmaV\n"
                        "/MPxUVnDrjq3xZ6s6xf/E/1tRX+iQHJHOKcgyhyVqnchUuIIug==\n"
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