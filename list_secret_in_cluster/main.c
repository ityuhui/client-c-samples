#include <apiClient.h>
#include <malloc.h>
#include <CoreV1API.h>
#include <errno.h>

#define K8S_APISERVER_BASEPATH "https://kubernetes"
#define K8S_SECRET_SAMLE_NAME "secret-sample-1"
#define K8S_SECRET_VERSION "v1"
#define K8S_SECRET_KIND "Secret"
#define K8S_SECRET_DATA_TOKEN_KEY "token"
#define K8S_NAMESPACE_SAMPLE "default"

#define K8S_TOKEN_FILE_IN_CLUSTER "/var/run/secrets/kubernetes.io/serviceaccount/token"
#define K8S_TOKEN_BUF_SIZE 1024
#define K8S_AUTH_KEY "Authorization"
#define K8S_AUTH_VALUE_TEMPLATE "Bearer %s"

typedef list_t k8s_client_c_list_t;

apiClient_t *g_k8sAPIConnector;

int
loadK8sConfigInCluster(char *token, int token_buf_size)
{
    static char fname[] = "loadK8sConfigInCluster()";

    FILE *fp;
    fp = fopen(K8S_TOKEN_FILE_IN_CLUSTER, "r");

    if (fp == NULL) {
        if (errno == ENOENT) {
            printf("\
%s: The file %s does not exist.",
fname, K8S_TOKEN_FILE_IN_CLUSTER);
            return (-1);
        } else {
            printf("\
%s: Failed to open file %s (%m).",
fname, K8S_TOKEN_FILE_IN_CLUSTER);
            return (-1);
        }
    }

    while (fgets(token, token_buf_size, fp) != NULL) {
        ;
    }

    printf("%s\n", token);

    fclose(fp);

    return 0;
}

static int
fillinSecretBody(v1_secret_t *secretSample, const char *b64_cred)
{
    secretSample->apiVersion = strdup(K8S_SECRET_VERSION);
    secretSample->kind = strdup(K8S_SECRET_KIND);
    secretSample->metadata = calloc(1, sizeof(v1_object_meta_t));
    secretSample->metadata->name = strdup(K8S_SECRET_SAMLE_NAME);

    k8s_client_c_list_t *tokenList = NULL;
    tokenList = list_create();
    keyValuePair_t *keyPairToken = keyValuePair_create(strdup(K8S_SECRET_DATA_TOKEN_KEY), strdup(b64_cred));
    list_addElement(tokenList, keyPairToken);
    secretSample->stringData = tokenList;

    return 0;
}

int
listSecret()
{
    char fname[] = "listSecret";

    v1_secret_list_t *secretList = 
    CoreV1API_listCoreV1NamespacedSecret(
        g_k8sAPIConnector,
        K8S_NAMESPACE_SAMPLE,
        NULL,
        0,
        NULL,
        NULL,NULL,
        100,
        NULL,
        1000,
        0
    );

    printf("return code=%ld\n", g_k8sAPIConnector->response_code);
    if (g_k8sAPIConnector->dataReceived) {
        printf("message=%s\n",(char *) (g_k8sAPIConnector->dataReceived));
    }

    if (secretList &&
        secretList->items) {
        listEntry_t *listEntry;

        list_ForEach(listEntry, secretList->items) {
            v1_secret_t *secret1 = (v1_secret_t *)(listEntry->data);
            printf("secret name: %s\n", secret1->metadata->name);
        }


    } else {
        printf("empty\n");
    }

}

int
init_k8s_connector()
{
    list_t *apiKeys;
    apiKeys = list_create();

    char *keyToken = strdup(K8S_AUTH_KEY);
    char token[K8S_TOKEN_BUF_SIZE];
    memset(token, 0, sizeof(token));

    loadK8sConfigInCluster(token, K8S_TOKEN_BUF_SIZE);

    char valueToken[K8S_TOKEN_BUF_SIZE];
    memset(valueToken, 0, sizeof(valueToken));
    sprintf(valueToken, K8S_AUTH_VALUE_TEMPLATE, token);

    keyValuePair_t *keyPairToken = keyValuePair_create(keyToken, valueToken);
    list_addElement(apiKeys, keyPairToken);

    g_k8sAPIConnector = apiClient_create(K8S_APISERVER_BASEPATH, apiKeys, NULL);
}

int main(int argc, char *argv[])
{
    init_k8s_connector();

    listSecret();

    apiClient_free(g_k8sAPIConnector);
}

