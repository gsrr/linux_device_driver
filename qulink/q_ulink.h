
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define TEMPLATE_SIZE 512
#define CELL_SIZE 16
#define TEMPLATE_RESERVE_BYTES 8

struct qulink_cell {
    unsigned short int id;
    unsigned short int flag; // 0: accumulated value, 1: new val.
    unsigned int val;
    char res[8];  // reserved, fill out zero
};

// for template 016, 201, 202
struct qulink_tmp {
    unsigned short int num;
    unsigned short int rev;
    unsigned short int cnt;
    unsigned short int len;
    char res[8];  // reserved, fill out zero
    struct qulink_cell *cells;
};

void qulink_temp_to_buffer(char *blob, void *value, int size, int *offset)
{
	int i = *offset;
	int* ivalue = (unsigned int*) value;
	if(size == 4)
	{
		blob[i] = (*ivalue >> 24) & 0xFF;
		blob[i + 1] = (*ivalue >> 16) & 0xFF;
		blob[i + 2] = (*ivalue >> 8) & 0xFF;
		blob[i + 3] = (*ivalue) & 0xFF;
	}
	else if(size == 2)
	{
		blob[i] = (*ivalue >> 8) & 0xFF;
		blob[i + 1] = (*ivalue) & 0xFF;
	}
	else if(size == 1)
	{
		blob[i] = (*ivalue) & 0xFF;
	}
	*offset += size;
}

unsigned int qulink_read_buffer(char *value, int size, int *offset)
{
	int num = 0;
	int i;
	for(i = 0 ; i < size ; i++)
	{
		num = num << 8;
		num |= (value[*offset + i] & 0xff);
	}
	*offset += size;
	return num;
}

int qulink_buffer_to_file(char *fpath, char *blob)
{
     int ret;
     int i;
     int fd = open(fpath, O_CREAT | O_WRONLY, 0644);  
     ret = write(fd, blob, TEMPLATE_SIZE);
     close(fd);
     return ret;
}

int qulink_file_to_buffer(char *file, char *blob)
{
    int ret;
    int fd = open(file, O_RDONLY, 0644); 
    ret = read(fd, blob, TEMPLATE_SIZE);
    close(fd);
    return ret;
}

void qulink_dump_cell(struct qulink_cell *cell, char *blob, int offset)
{
    int coff = offset;
    qulink_temp_to_buffer(blob, &(cell->id), 2, &coff);
    qulink_temp_to_buffer(blob, &(cell->flag), 2, &coff);
    qulink_temp_to_buffer(blob, &(cell->val), 4, &coff);
}

int qulink_dump_template(struct qulink_tmp *tmp, char *fpath)
{
    int i;
    int ret;
    int offset = 0;
    char blob[TEMPLATE_SIZE] = {0};

    qulink_temp_to_buffer(blob, &(tmp->num), 2, &offset);
    qulink_temp_to_buffer(blob, &(tmp->rev), 2, &offset);
    qulink_temp_to_buffer(blob, &(tmp->cnt), 2, &offset);
    qulink_temp_to_buffer(blob, &(tmp->len), 2, &offset);
    offset += TEMPLATE_RESERVE_BYTES;

    for(i = 0 ; i < tmp->cnt ; i++)
    {
        qulink_dump_cell((tmp->cells) + i, blob, offset);
        offset += CELL_SIZE;
    }

    ret = qulink_buffer_to_file(fpath, blob);
    if (ret < 0)
    {
        return ret;
    }
    return 0;
}

void qulink_read_cell(struct qulink_cell *cell, char* blob, int i, int offset)
{
    int coffset = offset;
    cell->id = qulink_read_buffer(blob, 2, &coffset);
    cell->flag = qulink_read_buffer(blob, 2, &coffset);
    cell->val = qulink_read_buffer(blob, 4, &coffset);
}

struct qulink_tmp* qulink_read_template(char *fpath)
{
    int i;
    int offset = 0;
    char blob[TEMPLATE_SIZE] = {0};
    struct qulink_tmp *tmp = (struct qulink_tmp*)malloc(sizeof(struct qulink_tmp));

    if(tmp == NULL)
    {
        return NULL;
    }
    if (qulink_file_to_buffer(fpath, blob) < 0)
    {
        free(tmp);
        return NULL;
    }
    tmp->num = qulink_read_buffer(blob, 2, &offset);
    tmp->rev = qulink_read_buffer(blob, 2, &offset);
    tmp->cnt = qulink_read_buffer(blob, 2, &offset);
    tmp->len = qulink_read_buffer(blob, 2, &offset);
    offset += TEMPLATE_RESERVE_BYTES; 
    if(tmp->cnt != 0)
    {
        tmp->cells = (struct qulink_cell*) malloc(sizeof(struct qulink_cell) * tmp->cnt);
        if(tmp->cells != NULL)
        {
            for(i = 0 ; i < tmp->cnt ; i++)
            {
                qulink_read_cell(&(tmp->cells[i]), blob, i, offset);
                offset += CELL_SIZE;
            }
        }
        else
        {
            // Fail to alloc memory for cells.
        }
    }
    return tmp;
}

struct qulink_tmp* qulink_init_tmp(int tnum, int rev)
{
    struct qulink_tmp *tmp = (struct qulink_tmp*)malloc(sizeof(struct qulink_tmp));
    if(tmp == NULL)
    {
        return NULL;
    }
    tmp->num = tnum;
    tmp->rev = rev;
    tmp->cnt = 0;
    tmp->len = TEMPLATE_SIZE;
    return tmp;
}   

struct qulink_tmp* qulink_init_tmp_with_cell(int tnum, int rev, int cnt)
{
    int i;
    struct qulink_tmp *tmp = (struct qulink_tmp*)malloc(sizeof(struct qulink_tmp));
    if(tmp == NULL)
    {
        return NULL;
    }
    tmp->num = tnum;
    tmp->rev = rev;
    tmp->cnt = cnt;
    tmp->len = TEMPLATE_SIZE;
    tmp->cells = (struct qulink_cell*)malloc(sizeof(struct qulink_cell) * cnt);

    if(tmp->cells != NULL)
    {
        for(i = 0 ; i < cnt ; i++)
        {
            tmp->cells[i].id = (i + 1);
            tmp->cells[i].flag = 1;
            tmp->cells[i].val = 0;
        }
    }
    return tmp;
}   

void qulink_display_tmp(struct qulink_tmp *tmp)
{
    int i;
    printf("template number : %d\n", tmp->num);
    printf("template revision : %d\n", tmp->rev);
    printf("cell count : %d\n", tmp->cnt);
    printf("template length : %d\n", tmp->len);
    if(tmp->cells)
    {
        for(i = 0 ; i < tmp->cnt ; i++)
        {
            printf("\ttmp->cells[%d].id : %d\n", (i + 1), tmp->cells[i].id);
            printf("\ttmp->cells[%d].flag : %d\n", (i + 1), tmp->cells[i].flag);
            printf("\ttmp->cells[%d].val : %d\n", (i + 1), tmp->cells[i].val);
            printf("\n");
        }
    }
}

void qulink_tmp_free(struct qulink_tmp *tmp)
{
    if(tmp)
    {
        if(tmp->cells)
        {
            free(tmp->cells);
        }
        free(tmp);
    }
}

int qulink_add_cnt(char *sn, int tnum, int id)
{
    struct qulink_tmp *tmp;
   char fpath[512] = {0};
   sprintf(fapth, "disk_data_%s_%d", sn, tmp);
   if( access( fname, F_OK ) != -1 ) 
   {
       // file exists
   } 
   else 
   {
       tmp = qulink_init_tmp_with_cell(tnum, )
   }
}

int qulink_get_disk_data(char *sn, int tmp)
{
    
}

