<h1 align="center">
  Semaphores structure. 
  <img src="https://cdn-icons-png.flaticon.com/512/1189/1189462.png" width="30px"/>
</h1>

### :lotus_position_man: The semaphore structure is particularly useful for managing synchronization tasks between processes and threads.

### :nerd_face: Using System V technology.

Semaphores enable processes to query or alter status information. They are often used to monitor and control the availability of system resources such as shared memory segments. Semaphores can be operated on as individual units or as elements in a set. The semaphore set must be initialized using semget. The semaphore creator can change its ownership or permissions using semctl. Any process with permission can use semctl to do control operations.
Semaphore operations are performed by semop. This interface takes a pointer to an array of semaphore operation structures. Each structure in the array contains data about an operation to perform on a semaphore. Any process with read permission can test whether a semaphore has a zero value. Operations to increment or decrement a semaphore require write permission. 
Only one process at a time can update a semaphore. Simultaneous requests by different processes are performed in an arbitrary order. When an array of operations is given by a semop call, no updates are done until all operations on the array can finish successfully. 

### :nerd_face: Synch

The codes you will find will have to manage the synchronization between threads and processes.
You will also be able to see the management of synchronization between different processes where each of them will have an arbitrary number of threads to manage.

### Execution from Unix and Library

Use "lpthread" means link with a library called pthread.

For example, `gcc file_name.c -lpthread` and  `./a.out param1 param2 param3 param4 ... ` 
The double **argv** **character pointer** is used to retrieve the parameters.
In fact, in all codes you can assign the argc parameter to a global variable to maintain passed information.

There is a need to include the pthread library:`#include <pthread.h>`.

However, that's not the only library to include.
After each traffic light operation we must check whether the operation we have performed is blocking.
If a blocking operation is activated the system call has aborted.
Consequently, the **ERRNO** environment variable is set to **EINTR**.
And so you need to include the following library: `#include <errno.h>`.

The inclusion of the standard library signal.h allows the use of functions for handling signals between processes: `#include <signal.h>`. 

## Signal

```
void (*signal( int sig, void (*handler) (int))) (int);
```
Sets the error handler for signal sig. The signal handler can be set so that default handling will occur, signal is ignored, or a user-defined function is called. 
	
## Parameters
The parameters of this function are the following:
* sig: the signal to set the signal handler to. It can be an implementation-defined value or one of the following values: **SIGABRT** **SIGFPE** **SIGILL** **SIGINT** **SIGSEGV** **SIGTERM**.
* handler: the signal handler. This must be one of the following:

   * **SIG_DFL** macro. The signal handler is set to default signal handler.
  
   * **SIG_IGN** macro. The signal is ignored.
  
   * **Pointer to a function.** The signature of the function must be equivalent to the following: `void fun(int sig);`.
	


	





### Semaphores Array[]

![](https://www.keil.com/pack/doc/CMSIS/RTOS/html/Semaphore.png)

You have to imagine the **busy world** .

In Rome there is a large national road called **"Via Tiburtina"** (spoiler: the whole city of Rome is busy) where there is a **traffic light every two hundred metres**, and a very long queue forms, especially around 6pm. :dizzy_face:
However, this traffic must be managed.  <img src="https://cdn-icons-png.flaticon.com/512/1189/1189462.png" width="20px"/>

The road is a **shared resource** and many cars find themselves, at a certain moment of time, on this resource and take up space.

If there were no traffic lights to regulate the flow of traffic there would be accidents. :woozy_face:
So we need to be able to understand which and how many threads or processes (or both) to pass through to correctly manage the flow.

**A Linux semaphore cannot have a negative value**, so when all threads have taken their token, the semaphore remains set to zero.
The concept of the semaphore array is important, because thanks to it we are able to atomically perform a specific operation assigned to just one (!) thread.

The thread/process accesses the shared resource by taking a token and returns the token to whoever wants: it can return it to the same semaphore structure to which it belongs, or another structure not necessarily an array.
Typically if the main thread has to handle simple operations the traffic light configuration is not an array, but a single token dispenser.

Each process/thread will have an element[s] available to perform withdrawal or supply operations.
**About what? Of the token.** :vertical_traffic_light:
Please note that this operation must be correctly managed by the programmer.

1. Select the `<> Code` button.

2. Copy the SSH link.

3. Open your terminal.

4. Etc.

Writing the above using bullet points means the user doesn't have to follow the order. When using bullet points, stick to a reasonable number (3-4).

You can lose the reader's attention by creating a long list of bullet points.

### Insert links and images.

Markdown allows you to insert links and images into your documentation. Use square brackets to create a link or image description, followed by the URL or file path in parentheses.

For example, `[GitHub](`[`https://github.com)`](https://github.com)`) creates a link to GitHub while` ![Image](https://techwithmaddy.com/path/to/image.jpg)` inserts an image.

## Creating a README File for Project Overview

A README file is a crucial component of any GitHub repository.

It serves as the entry point for your project, providing an overview of its purpose, features, installation instructions, and more.

A README file is especially [powerful when job-searching](https://techwithmaddy.com/5-actionable-steps-to-land-your-first-software-engineering-role#heading-a-project-that-includes-a-descriptive-readme-file) because:

- A non-technical recruiter can understand the purpose of the project.

- It shows that you possess an additional skill (which is writing).

Creating a comprehensive README file allows you to effectively communicate your project's value and encourage collaboration with other developers.

### **1. Start with a project description**

Begin your README file with a brief project description. Clearly state the purpose, goals, and intended audience of your project. This helps readers quickly understand your project and whether it aligns with their needs.

### **2. Include installation instructions**

Provide detailed instructions on how to install and set up your project.

Include any dependencies, system requirements, and step-by-step instructions. This helps other developers get started with your project quickly and reduces potential issues related to installation.

In this step, use numbered lists instead of bullet points.

### **3. Highlight key features**

Showcase the key features and functionalities of your project. This lets readers quickly grasp what your project offers and its potential benefits.

Use bullet points or concise descriptions to highlight the main features.

### **4. Provide usage examples**

Include usage examples to help readers understand how to use your project effectively.

This can include code snippets, command-line examples, or screenshots demonstrating the project in action.

Real-world examples make it easier for readers to relate to your project and envision how it can solve their problems.

### **5. Document API references**

If your project exposes an API, [document the API endpoints](https://techwithmaddy.com/api-testing-using-postman-for-beginners), request/response formats, and authentication requirements.

This helps developers integrate your project with their applications and ensures a smooth integration process.

## Key Takeaways

After reading this article, you know how to create the perfect documentation for your GitHub project.

If you enjoyed this article, I share more content like this in my [FREE weekly newsletter](https://techwithmaddy.com/newsletter).

I hope to see you there!
