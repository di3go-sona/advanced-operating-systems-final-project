

#### Introduction
An interprocess communication module for Linux Kernel developed as Final project for the Advanced Operating Systems and Virtualisation course @ La Sapienza

This inter-process messaging and synchronization subsystem is implemented as a kernel loadable module that manages two kind of devices: group root and group.

In order to compile and run the module it is sufficient to : 

- `$ make` to compile the module
- `$ make install` to install `udev` rules
- `$ make insert` to insert module in the kernel 
- `$ make test` to compile userspace library

Symmetrically there exist

- `$ make uninstall ` to delete udev rules
- `$ make remove` to remove module from kernel 

#### User Space Library

##### Raw Usage

The group root device is created at module insertion time and is mapped to `/dev/synch/group`  using a `udev` rule. It implements the open, ioctl and release operations.

```c
struct file_operations ipc_group_root_ops = {
	.open = ipc_group_root_open,
	.unlocked_ioctl = ipc_group_root_ioctl,
	.release = ipc_group_root_release
};
```

The `ioctl` accepts two ioctl numbers:

- `IPC_GROUP_UNINSTALL` to create a new group
- `IPC_GROUP_UNINSTALL` to destroy an existing group

Additionally a second parameter, an integer value that will be  casted to `group_t`,  should be provided, and it identifies the group number to be created/destroyed.

Group devices are dinamically created by calling `ioctl` on the aforementioned device, each group is mapped to `/dev/synch/group<i>` where` i ` is the identifier of the group and should be in the range 1 to `IPC_MAX_GROUPS` included.

Each group device implements the following operations:

```c
struct file_operations ipc_group_ops = {
	.open = ipc_group_open,
	.write = ipc_group_write,
	.read = ipc_group_read,
	.release = ipc_group_release,
	.unlocked_ioctl = ipc_group_ioctl,
	.flush = ipc_group_flush
};
```

- `write` allows to publish a message
- `read` allows to retrieve a published message
- `flush` Makes all the delayed messages available 
- `ioctl` accepts 5 different values as ioctl number:
  - `SET_SEND_DELAY` sets the delay between the moment a message is published and the moment it is made available, a second parameter should be provided that is interpreted as milliseconds.
  - `REVOKE_DELAYED_MESSAGES` Will destroy all messages that have been published with a delay value greater than 0 and that are not been made available yet
  - `FLUSH_DELAYED_MESSAGES` alias for flush
  - `SLEEP_ON_BARRIER` The process invoking this function will sleep until another process issues an awake command on this group
  - `AWAKE_BARRIER` Awakes all process sleeping on this group 

##### Wrapper

Additionally it is possible to execute basic primitives by using the wrappers provided inside `ipc_lib`

```c
void print_code(int err);

int install_group(group_t groupno, char* group_path, int lrn );
int uninstall_group(group_t groupno);
int open_group(group_t groupno);
int close_group(group_t groupno);
int send_msg(group_t groupno, char* payload, ssize_t payload_len);
int recv_msg(group_t groupno, char* payload, ssize_t payload_len);
int set_send_delay(group_t groupno, int delay);
int flush_delayed_messages(group_t groupno);
int revoke_delayed_messages(group_t groupno);
int sleep_on_barrier(group_t groupno);
int awake_barrier(group_t groupno);
```

This library simply incapsulate the device installation/uninstallation procedures and keeps track about file opening/closing procedures by  mantaining 2 arrays: one for group devices paths and one for group device files descriptor.

#### Kernel-Level Data Structures

This module revolves around three main data structures: 

###### Groups root device

This device is installed on module insertion and removed upon module deletion. It's only purpose is to allow the creation and deletion of regular messaging/synchronization group devices. It has a custom `ipc_group_root_dev` struct to mantain some required fields:

```c
typedef struct ipc_group_root_dev_t {
	spinlock_t lock;
	bool closing;
	struct cdev cdev;
} ipc_group_root_dev;
```

This structure wraps `cdev` ,in this way it can be retrieved from the file operations context using the `container_of` macro.

Additionally it stores only two meaningful values:

- A `spinlock_t lock` used to serialize group installation/removal procedures 
- A `bool closing` flag, it is set when the module removal is triggered, forbidding to open this device to other processes

###### Group device

The core of the module, this is the device to which messages are posted and synching requests are issued.

```c
typedef struct ipc_group_dev_t {
	struct list_head msg_list;
	struct list_head delayed_msg_list;
	int msg_count;
	int delayed_msg_count;
  spinlock_t lock;
	spinlock_t delayed_lock;
	int threads_count;
	ktime_t delay;
	struct wait_queue_head wait_queue;
	int waiting_count;
	int awaking_count;
	bool closing;
	struct cdev cdev;
} ipc_group_dev;
```

It contains the 3 sets of variables, 

- The general purpose ones:
  - A `int threads_count` to keep track of the number of threads that currently opened this device 
  - A `bool closing` flag that, similarly to the root group, inhibitis the open of this device once it is set to true
  - The `struct cdev cdev` to allow the retrieval of this structure using the `container_of` macro

- Those used by the messaging subsystem (there are 2 version for each of these, one for the available messages and one for the delayed)
  - The head of a doubly linked list  `struct list_head msg_list` (`delayed_msg_list`) to store available (delayed) messages 
  - A `int msg_count` ( `delayed_msg_count` ) to count available (delayed) messages 
  - A `spinlock_t lock` ( `delayed_lock` ) to synchronize list operations

- And those used for the synchronization operations:
  - `struct wait_queue_head wait_queue` to store waiting processes
  - `int awaking_count` and `int waiting_count` to keep count of processes in one or the other state 

###### IPC Message

The last structure is used to represent internal messages exchanged between processes 

```c
typedef struct ipc_message_t {
	struct list_head next;
	char* payload;
	ssize_t payload_len;
	ipc_group_dev* group_dev;
	struct hrtimer timer;
} ipc_message;
```

- `struct list_head next` used to insert messages inside the linked lists
- `char* payload` and `ssize_t payload_len` for the message payload 
- `ipc_group_dev* group_dev` is the group in which this message has been published ( although not strictly required it's handy to have this reference during some operations )
- `struct hrtimer timer` is the timer used to make delayed messages available

###### Other

Also, there are some global variables for the message and storage size and to keep track of installed devices.

```c
int max_message_size = IPC_DEFAULT_MSG_SIZE;
int max_storage_size = IPC_DEFAULT_STORAGE_SIZE;
int curr_storage_size = 0;

ipc_group_root_dev* 	group_root_dev ;
ipc_group_dev* 			group_devs[IPC_MAX_GROUPS+1] = {0};
```

Additionally `max_message_size` and `max_storage_size` are module parameters, allowing to customize them at runtime

```c
module_param(max_message_size, int, 0660);
module_param(max_storage_size, int, 0660);
```

#### Kernel-Level Subsystem Implementation

The susbsystem is basically composed of three kinds of actions: module management, message passing and barrier synchronization.

##### Module management

###### Module insertion and removal 

The `module_init()` and `module_exit()` operation will relatively perform the group root device creation and removal by calling `ipc_group_root_install()` and `ipc_group_root_uninstall()`

###### Group root installation

The group root installation  basically consist of sequentially calling the functions below

```c
alloc_chrdev_region(&devno, 0, IPC_MAX_GROUPS+1, IPC_ROOT_DEV_NAME);
class_create(THIS_MODULE, IPC_CLASS_NAME)
kmalloc(sizeof(ipc_group_root_dev), GFP_USER)
// fields initialization to default value
cdev_init(&(group_root_dev -> cdev) , &ipc_group_root_ops)
cdev_add(&(group_root_dev -> cdev) ,devno , 1)
device_create(group_dev_class, NULL, devno, NULL, IPC_ROOT_DEV_NAME)
```

Basically what they do is to allocate space for the device, initialize boh the custom fields a and the `cdev` ones, add it to sysfs using `cdev_add` and then create the kobject with `device_create` that will make this device appear under `/dev`

As these operation may fail, we should be checking the result of each one in order to assess if they were executed correctly like:

```c
group_root_device = device_create(group_dev_class, NULL, devno, NULL, IPC_ROOT_DEV_NAME);
	if (group_root_device < 0) {
		GR_ERROR( "Failed creating device\n");
    res = group_root_device;
		goto DEVICE_CREATE_FAIL;
	} else {
		GR_DEBUG( "Device creation success");
	}
```

If everything is executed correctly `0` is returned, otherwise we may leave the system in an inconsistent state, for this reason errors are handled in the following way:

- `int res ` is set to the return value ( or to a custom error )

- the `goto` construct is invoked to revert the steps already done

```
DEVICE_CREATE_FAIL:
	cdev_del(&(group_root_dev -> cdev));

CDEV_ADD_FAIL:
	kfree(group_root_dev);

CDEV_ALLOC_FAIL:
	class_destroy(group_dev_class);
	
CLASS_CREATE_FAIL:
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);

ALLOC_CHRDEV_REGION_FAIL:
	return res;
```

###### Group root removal

The removal of the group root consist of 3 phases:

- First the `closing` flag is set to true, ensuring that no other process can open this device (and subsequently forbidding to create a new group device once this routine has been fired)

  ```c
  spin_lock(&(group_root_dev -> lock));
  group_root_dev -> closing = true;
  spin_unlock(&(group_root_dev -> lock));
  ```

  Note that taking the lock is required in order to ensure that we wait the completion of concurrent group createion/deletion operations

- Then all group devices are uninstalled 

  ```c
  for (i=1; i<= IPC_MAX_GROUPS; i++){
    ipc_group_uninstall((group_t)i);
  }
  ```

- Finally the classical routines to remove the device from the system and deallocate the used memory are issued

  ```
  device_destroy(group_dev_class, devno);
  cdev_del(&(group_root_dev -> cdev));
  kfree(group_root_dev);
  class_destroy(group_dev_class);
  unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);
  return SUCCESS;
  ```

###### Group installation

Group  installation is triggered by issuing `ioctl` with the `IPC_GROUP_INSTALL` number as first parameter and `group_t groupno` as second one.

Once ioctl is called two things happen:

- The group root  lock is taken with `spin_lock(&(group_root_dev -> lock));` to ensure that no other group creation and deletion operation are happening concurrently 
- `int ipc_group_install(group_t groupno)` is invoked, and, it will perform the same steps of group_root creation, with a few additional checks.
  - Check if `groupno` is  valid ( i.e in the range 1 to `IPC_MAX_GROUPS` included )
  - Check if the group is not already installed
  - Allocate the required space for the device with `kmalloc`
  - Initialize the fields of `ipc_group_dev` used by this module
  - Initialize cdev and add it to sysfs with `cdev_init` and `cdev_alloc`
  - Create the kobject for the device with `device_create`
  - Store a pointer to this device into the `ipc_group_dev* group_devs[]` array 	

```c
kmalloc(sizeof(ipc_group_dev), GFP_USER);
cdev_init(&(group_dev->cdev), &ipc_group_ops);
// fields initialization to default value
cdev_add(&(group_dev->cdev), group_devno, 1);
device_create(group_dev_class, NULL, group_devno, NULL, devname);
```

###### Group removal

The group removal basically reverts the group installation operation, with a few preliminary steps required to ensure that no other process is using this group, basically what happens is that:

- Removal is triggered by issuing `ioctl` with the `IPC_GROUP_UNINSTALL` parameter.

- The group root  lock is taken with `spin_lock(&(group_root_dev -> lock));`

- `ipc_group_uninstall(group_t groupno)` is invoked and, now it happens that:
  - A validity check on `group_t groupno` is performed, similarly to the installation routine

  - Group closing flag is set to true, inhibiting other processes to open this device from now on

    ```c
    group_dev -> closing = true;
    ```

  - The  process enters a busy waiting loop waiting until it is the only one using the device

    ```c
    while (group_dev -> threads_count > 0 ){};
    ```

  - All delayed and available messages are deleted

    ```c
    _revoke_delayed_messages( group_dev );
    _delete_messages( group_dev );
    ```

  - The device itself is removed from the system and the memory it used is deallocated

    ```c
    device_destroy(group_dev_class, group_devno);
    cdev_del(&(group_dev->cdev));
    kfree(group_dev);
    ```

  - The reference to this device in the `ipc_group_dev* group_devs[]` array is set to `NULL`

##### Messaging system

###### Publish a message

Message publishing is triggered by issuing a write on the group device. First it will perform a few checks on the device in order to ensure that `message_size` is less than the maximum allowed 

```
if (lrn > max_message_size) return -EFBIG;
```

Then it will check if there is still space available on the device, this is implemented by means of a local read and compare and swap, in order to make this operation robust towards concurrency 

```c
do{
  old_storage_size = curr_storage_size;
  new_storage_size = old_storage_size + lrn + sizeof(ipc_message);
  if (new_storage_size > max_storage_size) {
    return -ENOSPC;		
  }
} while (__sync_bool_compare_and_swap(&curr_storage_size, 
                                      old_storage_size, 
                                      new_storage_size) == false );
```

Basically what it does is to 

- Read the current space usage
- Increase it locally if space is available
- Update the global variable if it didn't change in the meanwhile 

Then it allocates space for the message and initialize the internal fields

```c
msg = kmalloc(sizeof(ipc_message), GFP_USER);
if (msg == NULL) {
  res = -MEM_ALLOCATION_FAILED;
  goto MSG_ALLOC_FAIL;
};

payload_buf = kmalloc(lrn, GFP_USER);
if (payload_buf == NULL) {
  res = -MEM_ALLOCATION_FAILED;
  goto MSG_PAYLOAD_ALLOC_FAIL;
};
```

Then copies the buffer from user space

```c
copied = 0; 
while( copied < lrn){
  res = copy_from_user(payload_buf, buf, lrn-copied);
  if (res >0) copied += res;
  else if (copied ==0) break;
}
```

And finally, it adds the message in the available or delayed message queues by means of `_enqueue_message(ipc_message* msg, ipc_group_dev* group_dev)` or `_enqueue_delayed_message(ipc_message* msg, ipc_group_dev* group_dev)` depending on the current delay value for the group.

If the delay is 0, the message is instantly added by taking a lock and inserting it to the list 

```c
spin_lock( &(group_dev ->lock));
(group_dev -> msg_count )++ ;
list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
spin_unlock( &(group_dev ->lock));
```

If delay is greater than 0 it means that the message should not be available at publish time but we need to wait for an interval.

In order to implement this mechanism we'll rely on timers, so in addition to the normal list insertion we'll initialize a timer before taking the lock

```
	hrtimer_init(timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer -> function = _publish_delayed_message_handler;

	spin_lock( &(group_dev ->delayed_lock));
	(group_dev -> delayed_msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> delayed_msg_list));
	spin_unlock( &(group_dev ->delayed_lock));

	hrtimer_start(timer,group_dev -> delay,HRTIMER_MODE_REL);
```

And we'll start it right before releasing it, the publish `_publish_delayed_message_handler` will simply invoke `_publish_delayed_message`, that in turn will move the message from the delayed list to the available message list 

```c
	spin_lock( &(group_dev ->delayed_lock));
	(group_dev -> delayed_msg_count )-- ;
	__list_del_entry(&(msg->next));
	spin_unlock( &(group_dev ->delayed_lock));

	spin_lock( &(group_dev ->lock));
	(group_dev -> msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	spin_unlock( &(group_dev ->lock));
```

###### Retrieve a message

Retrieving a message is executed by calling read on the group file device, regardless of how many bytes are read one and only  one message is consumed everytime.

In order to read one message we will take the available messages list lock, check if there is at least one message, if there is one we store a reference to it and release the lock

```c
	spin_lock( &(group_dev ->lock));
	
	if (group_dev -> msg_count == 0){
		spin_unlock( &(group_dev ->lock));
		DEBUG("no msg found");

		return -NO_MESSAGES;
	}

	
	msg = list_first_entry(&(group_dev -> msg_list), ipc_message, next);
	(group_dev -> msg_count )-- ;

	__list_del_entry(&(msg->next));
	spin_unlock( &(group_dev ->lock));
```

Then we copy the content of the message to userspace 

```c
to_copy = lrn > msg -> payload_len ? msg -> payload_len : lrn;

while( copied < to_copy){
  res = copy_to_user(buf, msg->payload, to_copy-copied);
  if (res >0) copied += res;
  else if (copied ==0) break;
}
```

Free the message and update the space usage counter

```c
payload_len = msg -> payload_len;
kfree(msg->payload);
kfree(msg);
__sync_sub_and_fetch( &curr_storage_size, payload_len + sizeof(ipc_message));
```

###### Set message send delay

Setting the message send delay is implemented inside the `ioctl` of the `ipc_group`, and basically consist in changing a single variable 

```c
group_dev -> delay = ktime_set((int)ioctl_param,0);
```

###### Flush messages 

Flushing  messages requires to make all delayed messages available, in order to accomplish this, we may simply move them from one list to the other, but we also need to take care of timer expiration or concurrent execution. Finally we do not want to take the lock for a long time.

In order to accomplish this we'll create a temporary head for the list `tmp_list`

```
INIT_LIST_HEAD(&tmp_list);
```

Then we'll take the lock, move there all the messages from the `delayed_msg_list` and release the lock

```c
spin_lock( &(group_dev ->delayed_lock));

if (group_dev -> delayed_msg_count == 0 ) {
  return SUCCESS;
} else {
  list_splice_init(  &(group_dev -> delayed_msg_list), &tmp_list );
};

spin_unlock( &(group_dev ->delayed_lock));
```

Now we can safely operate on the temporary list, but there may still be timer in execution or close to expiration, for this reason we'll need to iterate over the list, stop all timers and count how many of them did we actually cancel.

```c
canceled = 0;
list_for_each_entry_safe(tmp_msg, _tmp_msg, &tmp_list, next){
  canceled += 1 - hrtimer_cancel(&(tmp_msg->timer));
}
__sync_sub_and_fetch(&(group_dev -> delayed_msg_count), canceled);
```

Finally we will take the lock and move all the remaining messages to the available messages list`msg_list`

```
spin_lock( &(group_dev ->lock));
list_splice( &tmp_list,  &(group_dev -> msg_list) );
group_dev -> msg_count += canceled ;
spin_unlock( &(group_dev ->lock));
```

###### Revoke messages 

Revoking basically works in the same way of flushing messages, with the only exception that, once we have our local list of messages to be revoked, instead of adding them to available list, we'll deallocate them

```c
list_for_each_entry_safe(tmp_msg, _tmp_msg, &tmp_list, next){
  __list_del_entry(&(tmp_msg -> next) );
  payload_len = tmp_msg -> payload_len;
  kfree(tmp_msg -> payload);
  kfree(tmp_msg);
  __sync_sub_and_fetch( &curr_storage_size, payload_len + sizeof(ipc_message));
}
```

##### Synchronization system

The synchronization subsystem is implemented by relying on the waitqueue mechanism. Basically each groups stores two counters:

- `int waiting_count` for the sleeping processes
- `int awaking_count` for the sleeping process that are waiting to be awoken

###### Sleeping on barrier 

```c
int pos;
int attempts=0;

pos = __sync_add_and_fetch( &(group_dev -> waiting_count), 1);
wait_event_interruptible(group_dev -> wait_queue, 
                         (attempts++ >0) && (group_dev -> awaking_count > 0));
__sync_sub_and_fetch( &(group_dev -> awaking_count), 1);
```

When a process invokes the `_sleep_on_barrier(ipc_group_dev* group_dev)` routine, it will simply 

- Increase the `waiting_count` 
- Invoke `wait_event_interruptible` on the condition `(attempts++ >0) && (group_dev -> awaking_count > 0)` where `(attempts++ >0)` is used to ensure that this process will enter the sleeping phase at least once, preventing the case in which a process tries to sleep in the moment in which orther processing are awaking.
- Decrease `awaking_count`

###### Awaking barrier

The awaking procedure is implemented by `_awake_barrier`

```c
int to_awake;

to_awake = __sync_fetch_and_and( &(group_dev -> waiting_count), 0);
__sync_add_and_fetch( &(group_dev -> awaking_count), to_awake);
wake_up_nr(&(group_dev -> wait_queue), to_awake);
```

And it basically consist of 

- Fetch the number of sleeping processes and set it to 0
- Increase the counter of processes to be awoken ` 

- wake up the same amount of processes

#### Testcase and Benchmark

The testing has been written using a test program in c that basically installs and opens a target group and, while keeping it open ( in order to avoid triggering flushes ) spawns a thread that perform one of the ` sleep | awake | send | recv | flush | revoke | set_delay  ` operations in loop.

It also accepts 3 optional arguments:

- `--repeat` that sets the number of loop iterations

- `--delay`  that sets the delay between successive loop iterations
- `--group ` that sets the group where operations are performed

This allows to combine the operation in various ways, for example I created a bash script spawning 4 sender processess and 4 receiver processes with a parametric value for `--repeat`

```bash
#! /bin/bash
REPEATS=${REPEATS:-100}
echo "Testing with 4 sender/receiver threads and $REPEATS repeats"

./test send "--repeat=$REPEATS" --delay=0 --group=2  &
./test send "--repeat=$REPEATS" --delay=0 --group=2  &
./test send "--repeat=$REPEATS" --delay=0 --group=2  &
./test send "--repeat=$REPEATS" --delay=0 --group=2  &

./test recv "--repeat=$REPEATS" --delay=0 --group=2  &
./test recv "--repeat=$REPEATS" --delay=0 --group=2  &
./test recv "--repeat=$REPEATS" --delay=0 --group=2  &
./test recv "--repeat=$REPEATS" --delay=0 --group=2  &


for job in `jobs -p`
do
    wait $job 
done
```

###### Benchmark

I tested the above program in the following settings:

- Ubuntu 20.04  under Virtualbox with 4 cores assigned 
- Physical machine is a MacBook Pro mounting an I7-7700HQ 
  ( 2.80 GHz base speed with 4 cores / 8 threads )
- 4 different values for `REPEATS` variable: 10, 1000, 10k and 50k

I obtained the following result

```
Testing with 4 sender/receiver threads and 10 repeats each
0.01 user 
0.00 system 
0.01 elapsed

Testing with 4 sender/receiver threads and 1000 repeats each
0.04 user 
0.40 system 
0.33 elapsed

Testing with 4 sender/receiver threads and 10000 repeats each
0.47 user 
2.83 system 
3.00 elapsed

Testing with 4 sender/receiver threads and 10000 repeats each
1.89 user 
18.30 system 
18.68 elapsed
```

