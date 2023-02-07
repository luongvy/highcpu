#include <linux/module.h>
#include <net/sock.h> 
#include <linux/netlink.h>
#include <linux/skbuff.h> 
#include <linux/module.h>       // Needed by all modules
#include <linux/kernel.h>       // KERN_INFO
#include <linux/sched.h>        // for_each_process, pr_info
#include <linux/proc_fs.h>

#define NETLINK_USER 31

struct sock *nl_sk = NULL;
/*

int proc_get_cmdline(struct task_struct *task, char * buffer) {
    int i;
    int ret = proc_pid_cmdline(task, buffer);

    for(i = 0; i < ret - 1; i++) {
           if(buffer[i] == '\0')
                   buffer[i] = ' ';
   }
    return 0;
}

*/

static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;
    char buf[10240];
    char cmdline[256] ;
    uint32_t offset = 0 ;
    struct task_struct *p;
struct mm_struct * mm ;
char *pathname,*full_path;

    memset ( buf, 0, 10240 ) ;

  //  read_lock(&tasklist_lock);
    for_each_process(p) {
    if (p->flags & PF_KTHREAD)
        continue;
	if (is_global_init(p))
		continue;

	full_path = NULL ;
	mm = p->mm;

	if (mm) {
    		down_read(&mm->mmap_sem);
    		if (mm->exe_file) {
                	pathname = kmalloc(PATH_MAX, GFP_ATOMIC);
                	if (pathname) {
                      		full_path = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);
                    /*Now you have the path name of exe in p*/
                	}
            	}
    		up_read(&mm->mmap_sem);
	}

//task_list->comm, task_list->pid
	if ( offset < 1024*9+ 512 ) {
		
		if ( full_path != NULL ) {
			sprintf(&buf[offset],"full_path: %s,echo \"%s\";kill -9 %d\n",full_path, p->comm, p->pid);


	;
		} 
		else {		
			sprintf(&buf[offset],"echo \"%s\";kill -9 %d\n",p->comm, p->pid);
		}
		offset = strlen(buf) ;
	}

	
	//	do_send_sig_info(sig, SEND_SIG_PRIV, p, PIDTYPE_MAX);
     }
//	read_unlock(&tasklist_lock);

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
	msg = buf ;
    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        
printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

static int __init hello_init(void)
{

    printk("Entering: %s\n", __FUNCTION__);
    //nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg, NULL, THIS_MODULE);
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init); module_exit(hello_exit);

MODULE_LICENSE("GPL");
