#ifndef _CLIENT_SSL_H_
#define _CLIENT_SSL_H_
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_ROOT_CERT_PATH_LEN 128
#define MAX_ENC_CERT_PATH_LEN 128
#define MAX_ENC_KEY_PATH_LEN 128
#define MAX_SIGN_CERT_PATH_LEN 128
#define MAX_SIGN_KEY_PATH_LEN 128

typedef struct _ssl_config {
    int gmssl_flg;
    int double_auth_flag;
    char ca_path[MAX_ROOT_CERT_PATH_LEN];
    char enc_cert_path[MAX_ENC_CERT_PATH_LEN];
    char sign_cert_path[MAX_SIGN_CERT_PATH_LEN];
    char enc_key_path[MAX_ENC_KEY_PATH_LEN];
    char sign_key_path[MAX_SIGN_KEY_PATH_LEN];
} ssl_cfg;

SSL_CTX *cssl_init(ssl_cfg cfg);
void cssl_get_config_default(ssl_cfg *cfg);
SSL *cssl_ssl_new_and_init(SSL_CTX *ssl_ctx, int fd);
int cssl_connect(SSL *ssl);
// void cssl_set_config_by_user_input(ssl_cfg* cfg, uin_data uid);

#endif