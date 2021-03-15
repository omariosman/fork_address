#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/moduleparam.h>
#include<linux/fs.h>
#include<linux/buffer_head.h>
#include<asm/segment.h>
#include<asm/uaccess.h>
#include<linux/slab.h>

MODULE_LICENSE("GPL");

struct myfile{
	struct file*f;
	mm_segment_t fs;
	loff_t pos;
};

struct myfile *open_file_for_read(char *filename){
	struct myfile *myfile_obj;
	myfile_obj = kmalloc(sizeof(struct myfile), GFP_KERNEL);

	
	
	int err = 0;

	myfile_obj->fs = get_fs();
	set_fs(get_ds());
	myfile_obj->f = filp_open(filename, O_RDONLY,0);
	myfile_obj->pos = 0;


	set_fs(myfile_obj->fs);

	if(IS_ERR(myfile_obj->f)){
		err = PTR_ERR(myfile_obj->f);
		return NULL;
	}

	return myfile_obj;

}

void close_file(struct myfile *mf){
	filp_close(mf->f, NULL);
	kfree(mf);
}

int read_from_file_until(struct myfile *mf, char *buf, unsigned long vlen, char c){
	
	int ret = 0;
	mf->fs = get_fs();
	set_fs(get_ds());
	
	int i;
	for (i = 0; i < vlen; i++){
		ret = vfs_read(mf->f, buf + i,1, &(mf->pos));
		if ((buf[i] == c) || (buf[i]) == '\0'){
			break;
		}
	}

	i++;
	buf[i] = '\0';
	set_fs(mf->fs);
	return ret;
	
}

struct file *fp;
static int hello_init(void){

	struct myfile* mf = open_file_for_read("/proc/version");
	char *data;
	data = kmalloc(sizeof(char), GFP_KERNEL);
	read_from_file_until(mf, data, 1024, ' ');
	read_from_file_until(mf, data, 1024, ' ');
	read_from_file_until(mf, data, 1024, ' ');

	printk("Kernel Version: %s\n", data);

	struct myfile* sysmap = open_file_for_read("/boot/System.map-4.19.0-13-amd64");
	char *line;
	line = kmalloc(100*sizeof(char), GFP_KERNEL);

	char *word;
	word = kmalloc(100*sizeof(char), GFP_KERNEL);
	word = "sys_call_table";

	while(true){
		read_from_file_until(sysmap, line, 1024, '\n');
		if (strstr(line, word)){
			break;
		}
	}

	printk("syscall: %s\n", line);

	char *sys_call_addr;
	sys_call_addr = kmalloc(100*sizeof(char), GFP_KERNEL);
	strncpy(sys_call_addr, line, 16);
	sys_call_addr[16] = '\0';
	printk("sys call addr: %s\n", sys_call_addr);

	sys_call_ptr_t *sys_call_arr = (sys_call_ptr_t *) sys_call_addr;
	printk("Fork Address: %px\n", sys_call_arr[__NR_fork]);







	close_file(mf);
	kfree(data);
	return 0;
}

static void hello_exit(void){

	printk(KERN_ALERT "Bye Bye CSCE-3402 :)\n");
	
}

module_init(hello_init);
module_exit(hello_exit);
