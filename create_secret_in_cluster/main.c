#include <apiClient.h>
#include <ActivitiesV1API.h>
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
#define K8S_CA_FILE_IN_CLUSTER "/var/run/secrets/kubernetes.io/serviceaccount/token/ca.crt"
#define K8S_TOKEN_BUF_SIZE 1024
#define K8S_AUTH_KEY "Authorization"
#define K8S_AUTH_VALUE_TEMPLATE "Bearer %s"

typedef list_t k8s_client_c_list_t;

apiClient_t *g_k8sAPIConnector;

void create_one_activity()
{
    char *namesapce = "default";

    ego_v1_activity_t * activityinfo = calloc(1, sizeof(ego_v1_activity_t));
    activityinfo->apiVersion = strdup("ego.symphony.spectrumcomputing.ibm.com/v1");
    activityinfo->kind = strdup("Activity");

    activityinfo->metadata = calloc(1, sizeof(v1_object_meta_t));
    activityinfo->metadata->name = strdup("activity-sample-ego-4");

    activityinfo->spec = calloc(1, sizeof(ego_v1_activity_spec_t));
    activityinfo->spec->host = strdup("workload-pod-2");
    activityinfo->spec->command = strdup("sleep 3601");

    ego_v1_activity_t* activ = ActivitiesV1API_createNamespacedActivity(
        g_k8sAPIConnector,
        namesapce,
        activityinfo,
        NULL);

    printf("code=%ld\n", g_k8sAPIConnector->response_code);

    ego_v1_activity_free(activityinfo);
}


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

    int len = strlen(token);
    if ('\n' == token[len - 1] ) {
        token[len -1] = '\0';
    }
    
    printf("%s\n", token);

    fclose(fp);

    return 0;
}

static int
fillinSecretBody(v1_secret_t *secretSample, const char *cred)
{
    secretSample->apiVersion = strdup(K8S_SECRET_VERSION);
    secretSample->kind = strdup(K8S_SECRET_KIND);
    secretSample->metadata = calloc(1, sizeof(v1_object_meta_t));
    secretSample->metadata->name = strdup(K8S_SECRET_SAMLE_NAME);
    secretSample->metadata->namespace = strdup(K8S_NAMESPACE_SAMPLE);

    k8s_client_c_list_t *tokenList = NULL;
    tokenList = list_create();
    keyValuePair_t *keyPairToken = keyValuePair_create(strdup(K8S_SECRET_DATA_TOKEN_KEY), strdup(cred));
    list_addElement(tokenList, keyPairToken);
    secretSample->stringData = tokenList;

    return 0;
}

int
setupK8sSecretSample()
{
    char fname[] = "setupK8sSecretSample";

    char *cred= "abc";

    v1_secret_t *secretSample = calloc(1, sizeof(v1_secret_t));
    fillinSecretBody(secretSample, cred);

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

    printf("%s: ActivitiesV1API_createNamespacedActivity return code=%ld\n",
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

        printf("%s: CoreV1API_replaceCoreV1NamespacedSecret return code=%ld\n",
            fname, g_k8sAPIConnector->response_code);
    }

    v1_secret_free(secretSample);
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

    keyValuePair_t *keyPairToken = keyValuePair_create(keyToken, strdup(valueToken));
    list_addElement(apiKeys, keyPairToken);

    g_k8sAPIConnector = apiClient_create(K8S_APISERVER_BASEPATH, apiKeys, NULL);
}

int main(int argc, char *argv[])
{
    init_k8s_connector();

    setupK8sSecretSample();
    //create_one_activity();

    apiClient_free(g_k8sAPIConnector);
}

