#include "jconf/jconf.h"

static void print_group(JGroup * g)
{
    g_printf("<%s %s>\n", j_group_get_name(g), j_group_get_value(g));
    GList *ptr = j_group_get_nodes(g);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_directive(n)) {
            JDirective *d = j_node_get_directive(n);
            g_printf("%s %s\n", j_directive_get_name(d),
                     j_directive_get_value(d));
        } else {
            JGroup *g = j_node_get_group(n);
            print_group(g);
        }
        ptr = g_list_next(ptr);
    }
    g_printf("</%s>\n", j_group_get_name(g));
}


int main(int argc, char *argv[])
{
    GError *error = NULL;
    JParser *p = j_conf_parse("./conf", &error);
    if (error != NULL) {
        g_printf("%s\n", error->message);
        g_error_free(error);
        return -1;
    }

    GList *ptr = j_parser_get_root(p);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_directive(n)) {
            JDirective *d = j_node_get_directive(n);
            g_printf("%s:%s\n", j_directive_get_name(d),
                     j_directive_get_value(d));
        } else {
            JGroup *g = j_node_get_group(n);
            print_group(g);
        }
        ptr = g_list_next(ptr);
    }
    j_parser_free(p);
    return 0;
}
