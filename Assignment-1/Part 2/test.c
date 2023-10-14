#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

// Module descriptions

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sourabh Soumyakanta Das and Shiladitya De");
MODULE_DESCRIPTION("LKM for a deque");
MODULE_VERSION("1.1");

#define LKM_NAME "partb_1_20CS30051_20CS30061"
#define PROCFS_MAX_SIZE 1024

// Structure define each element of the linked list of the deque
struct elem {
	int val;
	struct elem *next;
};

// linked list for implementing deque
struct deque {
	struct elem *head;
	struct elem *tail;
	int size;
	int max_capacity;
};

// Deque helper functions

/*
	Function name: deque_init
	Arguments: capacity of the deque
	Returns: pointer to the deque created and on error it returns NULL
	Function: It initialises the deque with the required capacity
*/
static struct deque *deque_init(int capacity)
{
    struct deque *dq = kmalloc(sizeof(struct deque), GFP_KERNEL);
    if(dq == NULL)
	{
        printk(KERN_ALERT "ERR: Memory could not be allocated for deque\n");
        return NULL;
    }
    dq->head = NULL;
	dq->tail = NULL;
	dq->size = 0;
	dq->max_capacity = capacity;
    return dq;
}

/*
	Function name: push_back
	Arguments: pointer to a deque, value to be inserted
	Returns: 0 if successful and -EACCES on error
	Function: It pushes to the back of the deque (from the right side)
*/

static int push_back(struct deque *dq, int val)
{
	if(dq->size == dq->max_capacity)
	{
        printk(KERN_ALERT "ERR: Deque is full\n");
        return -EACCES;
    }
	dq->size++;
	struct elem* element = kmalloc(sizeof(struct elem), GFP_KERNEL);
	element->val = val;
	element->next = NULL;
	if(dq->tail != NULL) dq->tail->next = element;
	dq->tail = element;
	if(dq->head == NULL) dq->head = element;
	return 0;
}

/*
	Function name: push_front
	Arguments: pointer to a deque, value to be inserted
	Returns: 0 if successful and -EACCES on error
	Function: It pushes the value to the front of the deque (from left side)
*/
static int push_front(struct deque *dq, int val)
{
	if(dq->size == dq->max_capacity)
	{
        printk(KERN_ALERT "ERR: Deque is full\n");
        return -EACCES;
    }
	dq->size++;
	struct elem* element = kmalloc(sizeof(struct elem), GFP_KERNEL);
	element->val = val;
	if(dq->head == NULL) element->next = NULL;
	else element->next = dq->head;
	dq->head = element;
	if(dq->tail == NULL) dq->tail = element;
	return 0;
}

/*
	Function name: pop_front
	Arguments: pointer to a deque (deque *dq)
	Returns: The value of the front node of the deque
	Function: It pops the element from the front of the deque and deletes the front (from left side) node, followed by returning the value of the front
*/

static int pop_front(struct deque *dq)
{
	if(dq->size == 0)
	{
        printk(KERN_ALERT "ERR: Deque is empty\n");
        return -EACCES;
    }
	int value = dq->head->val;
	dq->size--;
	if(dq->size == 0)
	{
		kfree(dq->head);
		dq->head = NULL;
		dq->tail = NULL;
	}
	else
	{
		struct elem *target = dq->head;
		dq->head = dq->head->next;
		kfree(target);
	}
	return value;
}

/*
	Function name: dq_delete
	Arguments: pointer to a deque (deque *dq)
	Function: It deletes the entire deque
*/
static void dq_delete(struct deque *dq)
{
    if(dq != NULL)
	{
        while(dq->size != 0) pop_front(dq);
		kfree(dq);
		dq = NULL;
    }
}

/*
	enum representing two process states:
	1) PROC_FILE_OPEN: This state represents the process states when the process has just created the proc file and did not do any operation on the deque (deque is not initialised)
	2) PROC_MODIFY_DEQUE: This state is reached after doing atleast one operation on the deque (the deque is created)
*/ 

enum proc_state {
    PROC_FILE_OPEN,
    PROC_MODIFY_DEQUE,
};

// Linked list of processes
struct proc_node {
    pid_t pid;
    enum proc_state state;
    struct deque *proc_deque;
    struct proc_node *next;
};

// Global variables
static struct proc_dir_entry *proc_file;
static struct proc_node *process_list = NULL; // list of processes which has the proc file open

DEFINE_MUTEX(mutex); // mutex lock


/*
	Function name: get_process_by_pid
	Arguments: pid (process id of the process)
	Returns: The process with the given pid and on error returns NULL
*/
static struct proc_node *get_process_by_pid(pid_t pid)
{
	struct proc_node *plist = process_list;

	while(plist != NULL)
	{
		if(plist->pid == pid) return plist;
		plist = plist->next;
	}

	return NULL;
}

/*
	Function name: insert_process
	Arguments: pid of the process to be inserted
	Returns: The pointer to the node inserted
	Function: It inserts the node of the new process to the list of processes
*/

static struct proc_node *insert_process(pid_t pid)
{
	struct proc_node *new_node = kmalloc(sizeof(struct proc_node), GFP_KERNEL);

	if(new_node != NULL)
	{
		new_node->pid = pid;
		new_node->state = PROC_FILE_OPEN;
		new_node->proc_deque = NULL;
		new_node->next = process_list; 
	}
	else return NULL;

	process_list = new_node;

	return new_node;
}

/*
	Function name: delete_proc_node
	Arguments: pointer to the node in the list of process which is to be deleted
	Returns: void
	Function: deletes the process node (by first deleting the deque)
*/

static void delete_proc_node(struct proc_node *node)
{
	if(node != NULL)
	{
		dq_delete(node->proc_deque);
		kfree(node);
	}
}

/*
	Function name: delete_process
	Arguments: pid of the process to be deleted from the list of processes
	Returns: 0 on successful deletion and -EACCES on error
	Function: deletes the process node from the list of processes (by calling the delete_proc_node as a sub-routine)
*/

static int delete_process(pid_t pid)
{
	struct proc_node *ptr1 = NULL;
	struct proc_node *ptr2 = process_list;

	if(ptr2->pid == pid)
	{
		process_list = ptr2->next;

		delete_proc_node(ptr2);
		return 0;
	}

	struct proc_node *gp = get_process_by_pid(pid);
	if(gp == NULL) return -EACCES;

	while(ptr2 != NULL)
	{
		if(ptr2->pid == pid)
		{
			ptr1->next = ptr2->next;
			delete_proc_node(ptr2);
			return 0;
		}

		ptr1 = ptr2;
		ptr2 = ptr2->next;
	}

	return 0;
}

/*
	Function name: delete_all_process
	Arguments: void
	Returns: void
	Function: Deletes all the processes from the process list
*/

static void delete_all_process(void)
{
	struct proc_node *all_proc = process_list;

	while(all_proc != NULL)
	{
		struct proc_node *temp = all_proc;
		all_proc = all_proc->next;
		delete_proc_node(temp);
	}
}

/*
	Function name: proc_file_create
	Arguments: pointer to a inode and a file pointer
	Returns: 0 if successful and -EACCES on error
	Function: Creates the proc_file for a process (after checking if the process already has one or not)
*/

static int proc_file_create(struct inode *ind, struct file *fp)
{
	pid_t pid; 
	struct proc_node *pnode;

	mutex_lock(&mutex);

	pid = current->pid;
	pnode = get_process_by_pid(pid);

	printk(KERN_INFO "proc_file_create() initiated by process[%d]\n", pid);

	if(pnode != NULL)
	{
		printk(KERN_ALERT "ERR: The process[%d] has its procfile already open\n", pid);
		mutex_unlock(&mutex);
		return -EACCES;
	}
	else
	{
		pnode = insert_process(pid);

		if(pnode == NULL)
		{
			printk(KERN_ALERT "ERR: The process[%d] node could not be allocated memory\n", pid);
			mutex_unlock(&mutex);
			return ENOMEM;
		}
		else
		{
			printk(KERN_INFO "SUCCESS: The process[%d] node has been created\n", pid);
			mutex_unlock(&mutex);
			return 0;
		}
	}

	return 0;
}

/*
	Function name: proc_file_create
	Arguments: pointer to a inode and a file pointer
	Returns: 0 if successful and 0 on error
	Function: Deletes the process from the list of processes
*/

static int proc_file_destroy(struct inode *ind, struct file* fp)
{
	pid_t pid;
	struct proc_node *pnode;
	
	mutex_lock(&mutex);
	pid = current->pid;
	printk(KERN_INFO "proc_file_destroy() initiated by process[%d]\n", pid);	
	pnode = get_process_by_pid(pid);

	if(pnode != NULL)
	{
		delete_process(pid);
		printk(KERN_INFO "SUCCESS: The process[%d] has been removed from the list\n", pid);
		mutex_unlock(&mutex);
		return 0;
	}
	else
	{
		printk(KERN_ALERT "ERR: The procfile of the process[%d] is not in open state\n", pid);
		mutex_unlock(&mutex);
		return -EACCES;
	}
	return 0;
}

/*
	Function name: read_dq
	Arguments: pointer to the process_node, character array, buffer size
	Returns: 0 if successful and -EACCES on error
	Function: reads the value from the front (using pop_front) and copies the value into the buf array
*/

static int read_dq(struct proc_node *pnode, char *buf, int *buf_size)
{
	if(pnode->state == PROC_FILE_OPEN)
	{
		printk(KERN_ALERT "The process[%d] has not written anything to the procfile\n", pnode->pid);
		return -EACCES;
	}
	if(pnode->proc_deque->size == 0)
	{
		printk(KERN_ALERT "ERR: The process[%d] has its deque empty\n", pnode->pid);
		return -EACCES;
	}

	int front = pop_front(pnode->proc_deque);
	memcpy(buf, &front, sizeof(int));
    	buf[sizeof(int)] = '\0';
	(*buf_size) = (int)sizeof(int);
	return sizeof(int);
}

/*
	Function name: proc_file_read
	Arguments: file pointer, array of charcters (in user space), length, offset
	Returns: 0 on success and -EACCES on error
	Function: reads the value from the front of the deque (using read_dq) and copies the value into the user space 
*/

static ssize_t proc_file_read(struct file *fp, char __user *buffer, size_t length, loff_t *offset)
{
	pid_t pid;
	struct proc_node *pnode;
	int ret_val = 0;
	mutex_lock(&mutex);

	pid = current->pid;
	printk(KERN_INFO "Process[%d] has initiated the proc_file_read() function\n", pid);
	pnode = get_process_by_pid(pid);

	if(pnode != NULL)
	{
		int buf_size = min(length, (int)PROCFS_MAX_SIZE);
		char *buf = kmalloc(sizeof(char)*(buf_size+1), GFP_KERNEL);	
		ret_val = read_dq(pnode, buf, &buf_size);
		if(ret_val >= 0)
		{
			int cp  = copy_to_user(buffer, buf, buf_size);
			kfree(buf);
			if(cp != 0)
			{
				printk(KERN_ALERT "ERR: The value could not be copied to the user space\n");
				mutex_unlock(&mutex);
				return -EACCES;
			}
			mutex_unlock(&mutex);
			return ret_val;
		}
		kfree(buf);
	}
	else
	{
		printk(KERN_ALERT "ERR: Process[%d] does not have the proc_file open", pid);
		mutex_unlock(&mutex);
		return -EACCES;
	}
	mutex_unlock(&mutex);
	return -EACCES;
}

/*
	Function name: write_dq
	Arguments: pointer to the process node, buffer, buffer size
	Returns: 0 on success and -EACCES on error or -EINVAL error
	Function: inserts the value on buffer to the deque after checking its parity
*/

static int write_dq(struct proc_node *pnode, char *buf, int *buf_size)
{
	if(pnode->state == PROC_FILE_OPEN)
	{
		if(*buf_size > 1)
		{
            printk(KERN_ALERT "ERR: Buffer size for maximum capacity must be 1 byte\n");
            return -EINVAL;
        }
        int max_capacity = (size_t)buf[0];
        if(max_capacity < 1 || max_capacity > 100)
		{
            printk(KERN_ALERT "ERR: Maximum capacity of the deque must be between 1 and 100\n");
            return -EINVAL;
        }
        pnode->proc_deque = deque_init(max_capacity);
        if(pnode->proc_deque == NULL)
		{
            printk(KERN_ALERT "ERR: Deque initialization failed\n");
            return -ENOMEM;
        }
        printk(KERN_INFO "SUCCESS: Deque with capacity %d has been initialized for process[%d]\n", max_capacity, pnode->pid);
        pnode->state = PROC_MODIFY_DEQUE;
	}
	else if(pnode->state == PROC_MODIFY_DEQUE)
	{
        if(*buf_size != 4)
		{
            printk(KERN_ALERT "ERR: Buffer size for value must be 4 bytes\n");
            return -EINVAL;
        }
        if(pnode->proc_deque->size == pnode->proc_deque->max_capacity)
		{
			printk(KERN_ALERT "ERR: The process[%d] has its deque full\n", pnode->pid);
            return -EACCES;
        }
        int value = *((int *)buf);
	printk(KERN_ALERT "The write value : %d\n", value);
        int ret_val;
        if(value % 2 == 0) ret_val = push_back(pnode->proc_deque, value);
        else ret_val = push_front(pnode->proc_deque, value);
        if(ret_val < 0)
		{
            printk(KERN_ALERT "ERR: Value insertion into deque failed\n");
            return -EACCES;
        }
        printk(KERN_INFO "Value %d has been inserted into the deque for process %d\n", value, pnode->pid);
    }

    return *buf_size;
}

/*
	Function name: proc_file_write
	Arguments: file pointer, user space character array, length, offset
	Returns: 0 on success and -EACCES on error
	Function: writes the value to be written on the deque (using write_dq)
*/

static ssize_t proc_file_write(struct file *fp, const char __user *buffer, size_t length, loff_t *offset)
{
    pid_t pid;
	struct proc_node *pnode;
	int ret_val = 0;
    mutex_lock(&mutex);

    pid = current->pid;
    printk(KERN_INFO "Process[%d] has initiated the proc_file_write() function\n", pid);
    pnode = get_process_by_pid(pid);

	if(pnode != NULL)
	{
        if(buffer == NULL || length == 0)
		{
            printk(KERN_ALERT "ERR: Nothing to write / Invalid write buffer\n");
            ret_val = -EINVAL;
        }
		else
		{
			int buf_size = min(length, (int)PROCFS_MAX_SIZE);
			char *buf = kmalloc(sizeof(char)*(buf_size+1), GFP_KERNEL);
            int cp  = copy_from_user(buf, buffer, buf_size);
			// printk("%c\n", *buffer);
            if(cp != 0)
			{
                printk(KERN_ALERT "ERR: The value could not be copied from the user space\n");
                ret_val = -EFAULT;
            }
			else ret_val = write_dq(pnode, buf, &buf_size);
			kfree(buf);
        }
    }
    else
	{
        printk(KERN_ALERT "ERR: Process[%d] does not have the proc file open\n", pid);
        ret_val = -EACCES;
    }
    mutex_unlock(&mutex);
    return ret_val;
}

/*
	structure consisting of the declarations of the endpoints of the functions
*/

static const struct proc_ops proc_fops = {
    .proc_open = proc_file_create,
    .proc_read = proc_file_read,
    .proc_write = proc_file_write,
    .proc_release = proc_file_destroy,
};


/*
	Function name: lkm_init
	Arguments: void
	Returns: 0 on success and -ENOENT on error
	Function: Initialises the LKM by creating the proc file
*/
static int __init lkm_init(void)
{
    printk(KERN_INFO "LKM initialised\n");
    proc_file = proc_create(LKM_NAME, 0666, NULL, &proc_fops);
    if(proc_file == NULL)
	{
        printk(KERN_ALERT "ERR: proc file could not be created\n");
        return -ENOENT;
    }
    printk(KERN_INFO "/proc/%s created\n", LKM_NAME);
    return 0;
}

/*
	Function name: lkm_exit
	Arguments: void
	Returns: void
	Function: Deletes all processes and removes the proc_file
*/
static void __exit lkm_exit(void)
{
    delete_all_process();
    remove_proc_entry(LKM_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", LKM_NAME);
    printk(KERN_INFO "LKM unloaded\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
