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

### Execution from Unix

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

The inclusion of the standard library signal.h allows the use of functions for handling signals between processes.

<script src="https://en.cppreference.com/mwiki/load.php?debug=false&amp;lang=en&amp;modules=startup&amp;only=scripts&amp;skin=cppreference2&amp;*"></script>
<script>if(window.mw){
mw.config.set({"wgCanonicalNamespace":"","wgCanonicalSpecialPageName":false,"wgNamespaceNumber":0,"wgPageName":"c/program/signal","wgTitle":"c/program/signal","wgCurRevisionId":140330,"wgArticleId":6183,"wgIsArticle":true,"wgAction":"view","wgUserName":null,"wgUserGroups":["*"],"wgCategories":[],"wgBreakFrames":false,"wgPageContentLanguage":"en","wgSeparatorTransformTable":["",""],"wgDigitTransformTable":["",""],"wgDefaultDateFormat":"dmy","wgMonthNames":["","January","February","March","April","May","June","July","August","September","October","November","December"],"wgMonthNamesShort":["","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"],"wgRelevantPageName":"c/program/signal","wgRestrictionEdit":[],"wgRestrictionMove":[]});
}</script><script>if(window.mw){
mw.loader.implement("user.options",function(){mw.user.options.set({"ccmeonemails":0,"cols":80,"date":"default","diffonly":0,"disablemail":0,"disablesuggest":0,"editfont":"default","editondblclick":0,"editsection":0,"editsectiononrightclick":0,"enotifminoredits":0,"enotifrevealaddr":0,"enotifusertalkpages":1,"enotifwatchlistpages":0,"extendwatchlist":0,"externaldiff":0,"externaleditor":0,"fancysig":0,"forceeditsummary":0,"gender":"unknown","hideminor":0,"hidepatrolled":0,"imagesize":2,"justify":0,"math":1,"minordefault":0,"newpageshidepatrolled":0,"nocache":0,"noconvertlink":0,"norollbackdiff":0,"numberheadings":0,"previewonfirst":0,"previewontop":1,"quickbar":5,"rcdays":7,"rclimit":50,"rememberpassword":0,"rows":25,"searchlimit":20,"showhiddencats":0,"showjumplinks":1,"shownumberswatching":1,"showtoc":0,"showtoolbar":1,"skin":"cppreference2","stubthreshold":0,"thumbsize":2,"underline":2,"uselivepreview":0,"usenewrc":0,"watchcreations":0,"watchdefault":0,"watchdeletion":0,
"watchlistdays":3,"watchlisthideanons":0,"watchlisthidebots":0,"watchlisthideliu":0,"watchlisthideminor":0,"watchlisthideown":0,"watchlisthidepatrolled":0,"watchmoves":0,"wllimit":250,"variant":"en","language":"en","searchNs0":true,"searchNs1":false,"searchNs2":false,"searchNs3":false,"searchNs4":false,"searchNs5":false,"searchNs6":false,"searchNs7":false,"searchNs8":false,"searchNs9":false,"searchNs10":false,"searchNs11":false,"searchNs12":false,"searchNs13":false,"searchNs14":false,"searchNs15":false,"gadget-ColiruCompiler":1,"gadget-MathJax":1});;},{},{});mw.loader.implement("user.tokens",function(){mw.user.tokens.set({"editToken":"+\\","patrolToken":false,"watchToken":false});;},{},{});
/* cache key: mwiki1-mwiki_en_:resourceloader:filter:minify-js:7:9f05c6caceb9bb1a482b6cebd4c5a330 */
}</script>
<script>if(window.mw){
mw.loader.load(["mediawiki.page.startup","mediawiki.legacy.wikibits","mediawiki.legacy.ajax"]);
}</script>
<style type="text/css">/*<![CDATA[*/
.source-c {line-height: normal;}
.source-c li, .source-c pre {
	line-height: normal; border: 0px none white;
}
/**
 * GeSHi Dynamically Generated Stylesheet
 * --------------------------------------
 * Dynamically generated stylesheet for c
 * CSS class: source-c, CSS id: 
 * GeSHi (C) 2004 - 2007 Nigel McNie, 2007 - 2008 Benny Baumann
 * (http://qbnz.com/highlighter/ and http://geshi.org/)
 * --------------------------------------
 */
.c.source-c .de1, .c.source-c .de2 {font: normal normal 1em/1.2em monospace; margin:0; padding:0; background:none; vertical-align:top;}
.c.source-c  {font-family:monospace;}
.c.source-c .imp {font-weight: bold; color: red;}
.c.source-c li, .c.source-c .li1 {font-weight: normal; vertical-align:top;}
.c.source-c .ln {width:1px;text-align:right;margin:0;padding:0 2px;vertical-align:top;}
.c.source-c .li2 {font-weight: bold; vertical-align:top;}
.c.source-c .kw1 {color: #0000dd;}
.c.source-c .kw2 {color: #0000ff;}
.c.source-c .kw3 {color: #0000dd;}
.c.source-c .kw4 {color: #0000ff;}
.c.source-c .co1 {color: #909090;}
.c.source-c .co2 {color: #339900;}
.c.source-c .coMULTI {color: #ff0000; font-style: italic;}
.c.source-c .es0 {color: #008000; font-weight: bold;}
.c.source-c .es1 {color: #008000; font-weight: bold;}
.c.source-c .es2 {color: #008000; font-weight: bold;}
.c.source-c .es3 {color: #008000; font-weight: bold;}
.c.source-c .es4 {color: #008000; font-weight: bold;}
.c.source-c .es5 {color: #008000; font-weight: bold;}
.c.source-c .br0 {color: #008000;}
.c.source-c .sy0 {color: #008000;}
.c.source-c .sy1 {color: #000080;}
.c.source-c .sy2 {color: #000040;}
.c.source-c .sy3 {color: #000040;}
.c.source-c .sy4 {color: #008080;}
.c.source-c .st0 {color: #008000;}
.c.source-c .nu0 {color: #000080;}
.c.source-c .nu6 {color:#000080;}
.c.source-c .nu8 {color:#000080;}
.c.source-c .nu12 {color:#000080;}
.c.source-c .nu16 {color:#000080;}
.c.source-c .nu17 {color:#000080;}
.c.source-c .nu18 {color:#000080;}
.c.source-c .nu19 {color:#000080;}
.c.source-c .ln-xtra, .c.source-c li.ln-xtra, .c.source-c div.ln-xtra {background-color: #ffc;}
.c.source-c span.xtra { display:block; }

/*]]>*/
</style><style type="text/css">/*<![CDATA[*/
.source-text {line-height: normal;}
.source-text li, .source-text pre {
	line-height: normal; border: 0px none white;
}
/**
 * GeSHi Dynamically Generated Stylesheet
 * --------------------------------------
 * Dynamically generated stylesheet for text
 * CSS class: source-text, CSS id: 
 * GeSHi (C) 2004 - 2007 Nigel McNie, 2007 - 2008 Benny Baumann
 * (http://qbnz.com/highlighter/ and http://geshi.org/)
 * --------------------------------------
 */
.text.source-text .de1, .text.source-text .de2 {font: normal normal 1em/1.2em monospace; margin:0; padding:0; background:none; vertical-align:top;}
.text.source-text  {font-family:monospace;}
.text.source-text .imp {font-weight: bold; color: red;}
.text.source-text li, .text.source-text .li1 {font-weight: normal; vertical-align:top;}
.text.source-text .ln {width:1px;text-align:right;margin:0;padding:0 2px;vertical-align:top;}
.text.source-text .li2 {font-weight: bold; vertical-align:top;}
.text.source-text .ln-xtra, .text.source-text li.ln-xtra, .text.source-text div.ln-xtra {background-color: #ffc;}
.text.source-text span.xtra { display:block; }




### Create lists

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1688905849306/14d3331f-1f07-4f70-8269-b19c9469956f.jpeg)

Use Markdown's list syntax to create ordered or unordered lists.

Note the difference between ordered and unordered lists: use numbered lists when the order matters. Use bullet points when the order is not important.

Often people mistake the two, and it adds confusion to the documentation.

Use numbered lists to explain procedural concepts. For example, to explain how to clone a GitHub repository to a local machine, you'll write something like this:

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
