#include "client_ssl.h"
#include "debug_msg.h"
#include "common.h"


SSL_CTX *cssl_init(ssl_cfg cfg)
{
    int ret;
    SSL_CTX *ctx;

#if 0
#if (OPENSSL_VERSION_NUMBER < 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER))
    SSL_library_init();
#ifndef ENABLE_SMALL
    SSL_load_error_strings();
#endif
    OpenSSL_add_all_algorithms();
#endif
#endif

    if(cfg.gmssl_flg) {
        ctx = SSL_CTX_new(GMTLS_client_method());
    } else {
        ctx = SSL_CTX_new(TLS_client_method());
    }

    if( !ctx ) {
        DB_ERR("ssl_ctx is NULL!");
        return NULL;
    }

    if(cfg.gmssl_flg)
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

    if( cfg.ca_path[0] ) {
        ret = SSL_CTX_load_verify_locations(ctx, cfg.ca_path, NULL);
        if( ret <= 0 ) {
            ret = ERR_get_error();
            DB_ERR("SSL_CTX_use_PrivateKey_file error:%s", ERR_reason_error_string(ret));
            return NULL;
        }
    }

    if( cfg.sign_cert_path[0] ) {
        ret = SSL_CTX_use_certificate_file(ctx, cfg.sign_cert_path, SSL_FILETYPE_PEM);
        if( ret <= 0 ) {
            ret = ERR_get_error();
            DB_ERR("SSL_CTX_use_certificate_file error:%s", ERR_reason_error_string(ret));
            return NULL;
        }
    }

    if( cfg.sign_key_path[0] ) {
        ret = SSL_CTX_use_PrivateKey_file(ctx, cfg.sign_key_path, SSL_FILETYPE_PEM);
        if( ret <= 0 ) {
            ret = ERR_get_error();
            DB_ERR("SSL_CTX_use_PrivateKey_file error:%s", ERR_reason_error_string(ret));
            return NULL;
        }
    }

    if( cfg.enc_cert_path[0] ) {
        ret = SSL_CTX_use_certificate_file(ctx, cfg.enc_cert_path, SSL_FILETYPE_PEM);
        if( ret <= 0 ) {
            ret = ERR_get_error();
            DB_ERR("SSL_CTX_use_certificate_file error:%s", ERR_reason_error_string(ret));
            return NULL;
        }
    }

    if( cfg.enc_key_path[0] ) {
        ret = SSL_CTX_use_PrivateKey_file(ctx, cfg.enc_key_path, SSL_FILETYPE_PEM);
        if( ret <= 0 ) {
            ret = ERR_get_error();
            DB_ERR("SSL_CTX_use_PrivateKey_file error:%s", ERR_reason_error_string(ret));
            return NULL;
        }
    }

    if ( (cfg.enc_key_path[0] || cfg.sign_key_path[0]) && !SSL_CTX_check_private_key(ctx) ) {
        ret = ERR_get_error();
        DB_ERR("SSL_CTX_check_private_key error:%s", ERR_reason_error_string(ret));
        return NULL;
    }

    return ctx;
}

// void cssl_set_config_by_user_input(ssl_cfg* cfg, uin_data uid)
// {
//     if( uid.ssl_opt & SSL_OPT_GMSSL_MASK )
//         cfg->gmssl_flg = 1;
// 	if(uid.ssl_opt & SSL_OPT_DOUBLE_AUTH_MASK)
// 		cfg->double_auth_flag = 1;
// }

void cssl_get_config_default(ssl_cfg *cfg)
{
    strcpy(cfg->ca_path, "conf/certs/1/rootcert.pem");
    if(cfg->double_auth_flag == 1) {
        strcpy(cfg->enc_cert_path, "conf/certs/1/enccert.pem");
        strcpy(cfg->sign_cert_path, "conf/certs/1/signcert.pem");
        strcpy(cfg->enc_key_path, "conf/certs/enc.key");
        strcpy(cfg->sign_key_path, "conf/certs/sign.key");
    }
    return;
}

SSL *cssl_ssl_new_and_init(SSL_CTX *ssl_ctx, int fd)
{
    SSL *ssl;
    int ret;

    ssl = SSL_new(ssl_ctx);
    if( ssl == NULL ) {
        DB_ERR("SSL_new failed!");
        return NULL;
    }

    ret = SSL_set_fd(ssl, fd);
    if( ret <= 0 ) {
        DB_ERR("SSL_set_fd failed!");
        SAFE_FREE(ssl, SSL_free);
        return NULL;
    }

    SSL_set_connect_state(ssl);

    return ssl;
}

int cssl_connect(SSL *ssl)
{
    int ret;

    ret = SSL_do_handshake(ssl);
    if( ret != 1 ) {
        return SSL_get_error(ssl, ret);
    }

    return 0;
}
