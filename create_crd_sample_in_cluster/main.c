#include <apiClient.h>
#include <ActivitiesV1API.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>

#define K8S_APISERVER_BASEPATH "https://kubernetes"
#define K8S_TOKEN_FILE_IN_CLUSTER "/var/run/secrets/kubernetes.io/serviceaccount/token"
#define K8S_TOKEN_BUF_SIZE 1024
#define K8S_AUTH_KEY "Authorization"
#define K8S_AUTH_VALUE_TEMPLATE "Bearer %s"

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

    create_one_activity();
}

