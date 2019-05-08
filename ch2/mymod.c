
#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>

MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Zhao Lei");
MODULE_DESCRIPTION("For test");

#define SIMP_BLKDEV_DEVICEMAJOR 72
#define SIMP_BLKDEV_DISKNAME "simp_blkdev"
#define SIMP_BLKDEV_BYTES (16*1024*1024)
static struct request_queue *simp_blkdev_queue;
static struct gendisk *simp_blkdev_disk;
struct block_device_operations simp_blkdev_fops = {
	.owner = THIS_MODULE,
};

unsigned char simp_blkdev_data[SIMP_BLKDEV_BYTES];

static void simp_transfer(unsigned long sector, unsigned long nsect, char *buffer, int write)
{
    unsigned long offset = sector * 512;
    unsigned long nbytes = nsect * 512;
    if (write)
    {
        printk("write request : (offset, nbytes) = (%ld, %ld)", offset, nbytes);
        memcpy(simp_blkdev_data + offset, buffer, nbytes);
    }
    else
    {
        printk("read request");
        memcpy(buffer, simp_blkdev_data + offset, nbytes);
    }
}


static void simp_handle_bio(struct request* req, struct bio *bio)
{
    unsigned long flags;
    struct bvec_iter i;
    struct bio_vec bvec;
    sector_t sector = bio->bi_iter.bi_sector;

    bio_for_each_segment(bvec, bio, i) {
        char *buffer = bvec_kmap_irq(&bvec, &flags);
        simp_transfer(sector, bio_cur_bytes(bio)>>9 ,buffer, bio_data_dir(bio) == WRITE);
        sector += bio_cur_bytes(bio)>>9;
        bvec_kunmap_irq(buffer, &flags);

    }
}

static void simp_blkdev_do_request(struct request_queue *q)
{
    struct request *req;
    struct bio *bio;

    while ((req = blk_fetch_request(q)) != NULL) {
        /*
        if ((blk_rq_sectors(req) + blk_rq_cur_sectors(req)) << 9 > SIMP_BLKDEV_BYTES) {
            printk(KERN_ERR SIMP_BLKDEV_DISKNAME": bad request: block=%llu, count=%un",
                    (unsigned long long)blk_rq_sectors(req), blk_rq_cur_sectors(req));
            __blk_end_request_all(req, 1);
            continue;
        }
        */
        __rq_for_each_bio(bio, req){
            simp_handle_bio(req, bio);
        }
        __blk_end_request_all(req, 0);
    }
}

static int simp_blkdev_init(void)
{
	int ret;
    struct elevator_queue *eq;
    struct elevator_queue *old_e;

    simp_blkdev_queue = blk_init_queue(simp_blkdev_do_request, NULL);
    if (!simp_blkdev_queue) {
        ret = -ENOMEM;
        goto err_init_queue;
    }
    /*
    old_e = simp_blkdev_queue->elevator;
    eq = elevator_get(simp_blkdev_queue, "noop", false);
    if(!eq)
    {
       printk(KERN_WARNING "Alloc elevator failed, using defaultn");
    }
    else
    {
        simp_blkdev_queue->elevator = eq;
        elevator_exit(old_e);
    }
    */

	simp_blkdev_disk = alloc_disk(1);
	if (!simp_blkdev_disk) {
		ret = -ENOMEM;
		goto err_alloc_disk;
	}
	strcpy(simp_blkdev_disk->disk_name, SIMP_BLKDEV_DISKNAME);
	simp_blkdev_disk->major = SIMP_BLKDEV_DEVICEMAJOR;
	simp_blkdev_disk->first_minor = 0;
	simp_blkdev_disk->fops = &simp_blkdev_fops;
	simp_blkdev_disk->queue = simp_blkdev_queue;
	set_capacity(simp_blkdev_disk, SIMP_BLKDEV_BYTES>>9);
	add_disk(simp_blkdev_disk);
    ret = blk_change_elevator(simp_blkdev_queue, "noop\n");
    printk("change elevator:%d", ret);
	return 0;

err_alloc_disk:
    blk_cleanup_queue(simp_blkdev_queue);
	return -1;

err_init_queue:
	return -1;
}

static void simp_blkdev_exit(void)
{
	del_gendisk(simp_blkdev_disk);
	put_disk(simp_blkdev_disk);
    blk_cleanup_queue(simp_blkdev_queue);
}
module_init(simp_blkdev_init);
module_exit(simp_blkdev_exit);

