#include "q_ulink.h"

int main()
{
    struct qulink_tmp *tmp = qulink_init_tmp_with_cell(16, 100, 10);
    qulink_dump_template(tmp, "./test.file");
    struct qulink_tmp *tmp2 = qulink_read_template("./test.file");
    qulink_display_tmp(tmp2);

printf("\n");
    tmp = qulink_init_tmp(201, 100);
    qulink_dump_template(tmp, "./test.file");
    tmp2 = qulink_read_template("./test.file");
    qulink_display_tmp(tmp2);

    printf("\n");
    tmp = qulink_init_tmp(202, 100);
    qulink_dump_template(tmp, "./test.file");
    tmp2 = qulink_read_template("./test.file");
    qulink_display_tmp(tmp2);
    return 0;
}

