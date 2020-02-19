#include <apiClient.h>
#include <malloc.h>
#include <CoreV1API.h>

#define K8S_APISERVER_BASEPATH "https://9.111.254.254:6443"
#define K8S_SECRET_SAMLE_NAME "secret-sample-1"
#define K8S_SECRET_VERSION "v1"
#define K8S_SECRET_KIND "Secret"
#define K8S_SECRET_DATA_TOKEN_KEY "token"
#define K8S_NAMESPACE_SAMPLE "default"

#define K8S_TOKEN_BUF_SIZE 1024
#define K8S_AUTH_KEY "Authorization"
#define K8S_AUTH_VALUE_TEMPLATE "Bearer %s"

typedef list_t k8s_client_c_list_t;

apiClient_t *g_k8sAPIConnector;

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
setupK8sSecretSample()
{
    char fname[] = "setupK8sSecretSample";

    char *b64_cred= "Admin\"2019-11-27T01:41:56Z\"VmlxT1TNVUf2wyNG3L2kFWUPLsk+uzulkuU8d4wg2vz7UXEqqle17fG9lXOBPrMxPT2Z6nK9NtYLwga3XJ3VwA==\"laYEX07TQC+n4YEGDxELHg==";

    v1_secret_t *secretSample = calloc(1, sizeof(v1_secret_t));
    fillinSecretBody(secretSample, b64_cred);

    v1_secret_t* secr1 = CoreV1API_createCoreV1NamespacedSecret(
        g_k8sAPIConnector,
        K8S_NAMESPACE_SAMPLE,
        secretSample,
        NULL,
        NULL,
        NULL);

    if (secr1) { // remove compiler warning
        ;
    }

    printf("%s: ActivitiesV1API_createNamespacedActivity return code=%ld",
        fname, g_k8sAPIConnector->response_code);

    if (409 == g_k8sAPIConnector->response_code) { // already exists
        v1_secret_t* secr2 = CoreV1API_replaceCoreV1NamespacedSecret(
            g_k8sAPIConnector,
            K8S_SECRET_SAMLE_NAME,
            K8S_NAMESPACE_SAMPLE,
            secretSample,
            NULL,
            NULL,
            NULL);

        if (secr2) { // remove compiler warning
            ;
        }

        printf("%s: CoreV1API_replaceCoreV1NamespacedSecret return code=%ld",
            fname, g_k8sAPIConnector->response_code);
    }

    v1_secret_free(secretSample);
}

void print_usage()
{
    printf("Usage: main token(mandotory) cafile(optional)\n\
e.g. main mtpZCI6IjJZT3k1bDNK\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage();
        return 1;
    }

    list_t *apiKeys;
    apiKeys = list_create();
    
    char *keyToken = strdup(K8S_AUTH_KEY);
    char valueToken[K8S_TOKEN_BUF_SIZE];
    memset(valueToken, 0, sizeof(valueToken));
    sprintf(valueToken, K8S_AUTH_VALUE_TEMPLATE, argv[1]);

    keyValuePair_t *keyPairToken = keyValuePair_create(keyToken, valueToken);
    list_addElement(apiKeys, keyPairToken);

    // connect to API server directly in hypervisor
    g_k8sAPIConnector = apiClient_create(K8S_APISERVER_BASEPATH, apiKeys, NULL);

    // connect to "kubectl proxy" in hypervisor
    //g_k8sAPIConnector = apiClient_create("http://localhost:8001", NULL, NULL);

    setupK8sSecretSample();
}

