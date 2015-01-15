#include "mod-pub.h"


static JaModule module = { "hello world", NULL };

void JA_MODULE_INIT()
{
    g_message("hello world");
    ja_module_register(&module);
}
