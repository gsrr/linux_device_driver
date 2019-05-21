#include "q_ulink.h"

int main()
{
    struct qulink_tmp *tmp = qulink_init_tmp_with_cell(16);
    qulink_dump_template(tmp, "./test.file");
    struct qulink_tmp *tmp2 = qulink_read_template("./test.file");
    qulink_display_tmp(tmp2);

printf("\n");
    tmp = qulink_init_tmp_with_cell(201);
    qulink_dump_template(tmp, "./test.file");
    tmp2 = qulink_read_template("./test.file");
    qulink_display_tmp(tmp2);

    printf("\n");
    tmp = qulink_init_tmp_with_cell(202);
    qulink_dump_template(tmp, "./test.file");
    tmp2 = qulink_read_template("./test.file");
    qulink_display_tmp(tmp2);

    printf("test-4\n");
    qulink_cell_add_cnt("1234", 16, 1);
    tmp2 = qulink_read_template("/root/disk_data_1234_16");
    qulink_display_tmp(tmp2);
    
    printf("test-5\n");
    int ret = qulink_check_disk_data("1234", 16);
    printf("check disk data:%d\n", ret);

    printf("test-6\n");
    ret = qulink_check_disk_data("1234", 201);
    printf("check disk data:%d\n", ret);
    return 0;
}

