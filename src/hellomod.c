#include "mod-pub.h"


static JaAction request_handler(JaRequest * req)
{
    const void *data = ja_request_data(req);
    int len = ja_request_data_length(req);
    int i = 0;
    for (i = 0; i < len; i++) {
        ja_response_append(req, "l", 1);
    }
    ja_response_append(req, "\0", 1);
    return JA_ACTION_RESPONSE;
}


void init()
{
    g_message("hello world");
    ja_hook_register((void *) request_handler, JA_HOOK_TYPE_REQUEST);
}

JaModule hellomod_struct = {
    "helloall",
    init
};
