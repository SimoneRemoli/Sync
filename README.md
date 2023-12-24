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

The inclusion of the standard library signal.h allows the use of functions for handling signals between processes: `#include <signal.h>`.


                <a id="top"></a>
                <div id="mw-js-message" style="display:none;"></div>
                                <!-- firstHeading -->
<script async="" type="text/javascript" src="//cdn.carbonads.com/carbon.js?serve=CKYITK7M&amp;placement=cppreferencecom" id="_carbonads_js"></script><div id="carbonads"><span>
<style>#carbonads{max-width:130px;left:-150px}#carbonads .carbon-img img{width:100% !important}</style>
<span class="carbon-wrap">
	<a href="https://srv.carbonads.net/ads/click/x/GTND427ECKBDVKQ7CKYLYKQUCT7ILKJYCY7DLZ3JCAYD65QUFTYD627KCWAD42QUF6BI4KQ7CKSD42JLF6AI65QKC6SITK7WC6BIKK3EHJNCLSIZ" class="carbon-img" target="_blank" rel="noopener sponsored">
		<img src="https://srv.carbonads.net/static/30242/1e53d1c47237794c6203cbbe39552e7fa8d4a116" alt="ads via Carbon" border="0" height="100" width="130" data-no-statview="no" style="max-width: 130px;">
	</a>
	<a href="https://srv.carbonads.net/ads/click/x/GTND427ECKBDVKQ7CKYLYKQUCT7ILKJYCY7DLZ3JCAYD65QUFTYD627KCWAD42QUF6BI4KQ7CKSD42JLF6AI65QKC6SITK7WC6BIKK3EHJNCLSIZ" class="carbon-text" target="_blank" rel="noopener sponsored">
		Build AI-powered web apps faster with Vercel AI SDK: a serverless, edge-ready, open-source library.
	</a>
</span>
<a href="http://carbonads.net/?utm_source=cppreferencecom&amp;utm_medium=ad_via_link&amp;utm_campaign=in_unit&amp;utm_term=carbon" class="carbon-poweredby" target="_blank" rel="noopener sponsored">ads via Carbon</a>
</span></div>
                <h1 id="firstHeading" class="firstHeading">signal</h1>
                <!-- /firstHeading -->
                <!-- bodyContent -->
                <div id="bodyContent">
                                        <!-- tagline -->
                    <div id="siteSub">From cppreference.com</div>
                    <!-- /tagline -->
                                        <!-- subtitle -->
                    <div id="contentSub"><span class="subpages">&lt; <a href="/w/c" title="c">c</a>‎ | <a href="/w/c/program" title="c/program">program</a></span></div>
                    <!-- /subtitle -->
                                                            <!-- bodycontent -->
                    <div id="mw-content-text" lang="en" dir="ltr" class="mw-content-ltr"><div class="t-navbar" style=""><div class="t-navbar-sep">&nbsp;</div><div class="t-navbar-head"><a href="/w/c" title="c"> C</a><div class="t-navbar-menu"><div><div><table class="t-nv-begin" cellpadding="0" style="line-height:1.1em;">
<tbody><tr class="t-nv"><td colspan="5"> <a href="/w/c/language" title="c/language"> Language</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/header" title="c/header"> Headers</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/types" title="c/types"> Type support</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/program" title="c/program"> Program utilities</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/variadic" title="c/variadic"> Variadic function support</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/error" title="c/error"> Error handling</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/memory" title="c/memory"> Dynamic memory management</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/string" title="c/string"> Strings library</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/algorithm" title="c/algorithm"> Algorithms</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/numeric" title="c/numeric"> Numerics</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/chrono" title="c/chrono"> Date and time utilities</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/io" title="c/io"> Input/output support</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/locale" title="c/locale"> Localization support</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/thread" title="c/thread"> Concurrency support</a> <span class="t-mark-rev t-since-c11">(C11)</span> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/experimental" title="c/experimental"> Technical Specifications</a> </td></tr>
<tr class="t-nv"><td colspan="5"> <a href="/w/c/symbol_index" title="c/symbol index"> Symbol index</a> </td></tr>
</tbody></table></div><div><span class="editsection noprint plainlinks" title="Edit this template"><a rel="nofollow" class="external text" href="https://en.cppreference.com/mwiki/index.php?title=Template:c/navbar_content&amp;action=edit">[edit]</a></span></div></div></div></div><div class="t-navbar-sep">&nbsp;</div><div class="t-navbar-head"><a href="/w/c/program" title="c/program"> Program support utilities</a><div class="t-navbar-menu"><div><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv-h1"><td colspan="5">Program termination</td></tr>
<tr class="t-nv-col-table"><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/abort" title="c/program/abort"><span class="t-lines"><span>abort</span></span></a></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/exit" title="c/program/exit"><span class="t-lines"><span>exit</span></span></a></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/quick_exit" title="c/program/quick exit"><span class="t-lines"><span>quick_exit</span></span></a></div><div><span class="t-lines"><span><span class="t-mark-rev t-since-c11">(C11)</span></span></span></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/_Exit" title="c/program/ Exit"><span class="t-lines"><span>_Exit</span></span></a></div><div><span class="t-lines"><span><span class="t-mark-rev t-since-c99">(C99)</span></span></span></div></div></td></tr>
</tbody></table></div></td><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/atexit" title="c/program/atexit"><span class="t-lines"><span>atexit</span></span></a></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/at_quick_exit" title="c/program/at quick exit"><span class="t-lines"><span>at_quick_exit</span></span></a></div><div><span class="t-lines"><span><span class="t-mark-rev t-since-c11">(C11)</span></span></span></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/EXIT_status" title="c/program/EXIT status"><span class="t-lines"><span>EXIT_SUCCESS</span><span>EXIT_FAILURE</span></span></a></div></div></td></tr>
</tbody></table></div></td></tr>
<tr class="t-nv-h1"><td colspan="5">Unreachable control flow</td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/unreachable" title="c/program/unreachable"><span class="t-lines"><span>unreachable</span></span></a></div><div><span class="t-lines"><span><span class="t-mark-rev t-since-c23">(C23)</span></span></span></div></div></td></tr>
<tr class="t-nv-h1"><td colspan="5">Communicating with the environment</td></tr>
<tr class="t-nv-col-table"><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/system" title="c/program/system"><span class="t-lines"><span>system</span></span></a></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><br>
</td></tr>
</tbody></table></div></td><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/getenv" title="c/program/getenv"><span class="t-lines"><span>getenv</span><span>getenv_s</span></span></a></div><div><span class="t-lines"><span></span><span><span class="t-mark-rev t-since-c11">(C11)</span></span></span></div></div></td></tr>
</tbody></table></div></td></tr>
<tr class="t-nv-h1"><td colspan="5">Signals</td></tr>
<tr class="t-nv-col-table"><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><strong class="selflink"><span class="t-lines"><span>signal</span></span></strong></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/raise" title="c/program/raise"><span class="t-lines"><span>raise</span></span></a></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/sig_atomic_t" title="c/program/sig atomic t"><span class="t-lines"><span>sig_atomic_t</span></span></a></div></div></td></tr>
</tbody></table></div></td><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/SIG_strategies" title="c/program/SIG strategies"><span class="t-lines"><span>SIG_DFL</span><span>SIG_IGN</span></span></a></div></div></td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/SIG_ERR" title="c/program/SIG ERR"><span class="t-lines"><span>SIG_ERR</span></span></a></div></div></td></tr>
</tbody></table></div></td></tr>
<tr class="t-nv-h2"><td colspan="5">Signal types</td></tr>
<tr class="t-nv-col-table"><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/SIG_types" title="c/program/SIG types"><span class="t-lines"><span>SIGABRT</span><span>SIGFPE</span><span>SIGILL</span></span></a></div></div></td></tr>
</tbody></table></div></td><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/SIG_types" title="c/program/SIG types"><span class="t-lines"><span>SIGINT</span><span>SIGSEGV</span><span>SIGTERM</span></span></a></div></div></td></tr>
</tbody></table></div></td></tr>
<tr class="t-nv-h1"><td colspan="5">Non-local jumps</td></tr>
<tr class="t-nv-col-table"><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/setjmp" title="c/program/setjmp"><span class="t-lines"><span>setjmp</span></span></a></div></div></td></tr>
</tbody></table></div></td><td><div><table class="t-nv-begin" cellpadding="0" style="">
<tbody><tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/longjmp" title="c/program/longjmp"><span class="t-lines"><span>longjmp</span></span></a></div></div></td></tr>
</tbody></table></div></td></tr>
<tr class="t-nv-h2"><td colspan="5">Types</td></tr>
<tr class="t-nv"><td colspan="5"><div class="t-nv-ln-table"><div><a href="/w/c/program/jmp_buf" title="c/program/jmp buf"><span class="t-lines"><span>jmp_buf</span></span></a></div></div></td></tr>
</tbody></table></div><div><span class="editsection noprint plainlinks" title="Edit this template"><a rel="nofollow" class="external text" href="https://en.cppreference.com/mwiki/index.php?title=Template:c/program/navbar_content&amp;action=edit">[edit]</a></span></div></div></div></div><div class="t-navbar-sep">&nbsp;</div></div>
<table class="t-dcl-begin"><tbody>
<tr class="t-dsc-header">
<td> <div>Defined in header <code>&lt;signal.h&gt;</code>
 </div></td>
<td></td>
<td></td>
</tr>
<tr class="t-dcl">
<td class="t-dcl-nopad"> <div><span class="mw-geshi c source-c"><span class="kw4">void</span> <span class="br0">(</span><span class="sy2">*</span>signal<span class="br0">(</span> <span class="kw4">int</span> sig, <span class="kw4">void</span> <span class="br0">(</span><span class="sy2">*</span>handler<span class="br0">)</span> <span class="br0">(</span><span class="kw4">int</span><span class="br0">)</span><span class="br0">)</span><span class="br0">)</span> <span class="br0">(</span><span class="kw4">int</span><span class="br0">)</span><span class="sy4">;</span></span></div></td>
<td class="t-dcl-nopad">  </td>
<td class="t-dcl-nopad">  </td>
</tr>
<tr class="t-dcl-sep"><td></td><td></td><td></td></tr>
</tbody></table>
<p>Sets the error handler for signal <code>sig</code>. The signal handler can be set so that default handling will occur, signal is ignored, or a user-defined function is called.
</p><p>When signal handler is set to a function and a signal occurs, it is implementation defined whether <span class="t-c"><span class="mw-geshi c source-c">signal<span class="br0">(</span>sig, <a href="http://en.cppreference.com/w/c/program/SIG_strategies"><span class="kw499">SIG_DFL</span></a><span class="br0">)</span></span></span> will be executed immediately before the start of signal handler. Also, the implementation can prevent some implementation-defined set of signals from occurring while the signal handler runs.
</p>
<table id="toc" class="toc"><tbody><tr><td><div id="toctitle"><h2>Contents</h2><span class="toctoggle">&nbsp;[<a href="#" class="internal" id="togglelink">hide</a>]&nbsp;</span></div>
<ul>
<li class="toclevel-1 tocsection-1"><a href="#Parameters"><span class="tocnumber">1</span> <span class="toctext">Parameters</span></a></li>
<li class="toclevel-1 tocsection-2"><a href="#Return_value"><span class="tocnumber">2</span> <span class="toctext">Return value</span></a></li>
<li class="toclevel-1 tocsection-3"><a href="#Signal_handler"><span class="tocnumber">3</span> <span class="toctext">Signal handler</span></a></li>
<li class="toclevel-1 tocsection-4"><a href="#Notes"><span class="tocnumber">4</span> <span class="toctext">Notes</span></a></li>
<li class="toclevel-1 tocsection-5"><a href="#Example"><span class="tocnumber">5</span> <span class="toctext">Example</span></a></li>
<li class="toclevel-1 tocsection-6"><a href="#References"><span class="tocnumber">6</span> <span class="toctext">References</span></a></li>
<li class="toclevel-1 tocsection-7"><a href="#See_also"><span class="tocnumber">7</span> <span class="toctext">See also</span></a></li>
</ul>
</td></tr></tbody></table>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=1" title="Edit section: Parameters">edit</a>]</span> <span class="mw-headline" id="Parameters">Parameters</span></h3>
<table class="t-par-begin">


<tbody><tr class="t-par">
<td>  sig
</td>
<td> -
</td>
<td>  the signal to set the signal handler to. It can be an implementation-defined value or one of the following values:
<table class="t-dsc-begin">

<tbody><tr class="t-dsc">
<td>  <div class="t-dsc-member-div"><div><a href="/w/c/program/SIG_types" title="c/program/SIG types"> <span class="t-lines"><span>SIGABRT</span><span>SIGFPE</span><span>SIGILL</span><span>SIGINT</span><span>SIGSEGV</span><span>SIGTERM</span></span></a></div></div>
</td>
<td>   defines signal types <br> <span class="t-mark">(macro constant)</span> <span class="editsection noprint plainlinks" title="Edit this template"><a rel="nofollow" class="external text" href="https://en.cppreference.com/mwiki/index.php?title=Template:c/program/dsc_SIG_types&amp;action=edit">[edit]</a></span>
</td></tr>
</tbody></table>
</td></tr>
<tr class="t-par">
<td>  handler
</td>
<td> -
</td>
<td>  the signal handler. This must be one of the following:
<ul><li><span class="t-lc"><a href="/w/c/program/SIG_strategies" title="c/program/SIG strategies">SIG_DFL</a></span> macro. The signal handler is set to default signal handler.
</li><li><span class="t-lc"><a href="/w/c/program/SIG_strategies" title="c/program/SIG strategies">SIG_IGN</a></span> macro. The signal is ignored.
</li><li>pointer to a function. The signature of the function must be equivalent to the following:
</li></ul>
<table class="t-dcl-begin"><tbody>

<tr class="t-dcl">
<td class="t-dcl-nopad"> <div><span class="mw-geshi c source-c"><span class="kw4">void</span> fun<span class="br0">(</span><span class="kw4">int</span> sig<span class="br0">)</span><span class="sy4">;</span></span></div></td>
<td class="t-dcl-nopad">  </td>
<td class="t-dcl-nopad">  </td>
</tr>
<tr class="t-dcl-sep"><td></td><td></td><td></td></tr>
</tbody></table>
</td></tr></tbody></table>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=2" title="Edit section: Return value">edit</a>]</span> <span class="mw-headline" id="Return_value">Return value</span></h3>
<p>Previous signal handler on success or <span class="t-lc"><a href="/w/c/program/SIG_ERR" title="c/program/SIG ERR">SIG_ERR</a></span> on failure (setting a signal handler can be disabled on some implementations).
</p>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=3" title="Edit section: Signal handler">edit</a>]</span> <span class="mw-headline" id="Signal_handler">Signal handler</span></h3>
<p>The following limitations are imposed on the user-defined function that is installed as a signal handler.
</p><p>If the user defined function returns when handling <span class="t-lc"><a href="/w/c/program/SIG_types" title="c/program/SIG types">SIGFPE</a></span>, <span class="t-lc"><a href="/w/c/program/SIG_types" title="c/program/SIG types">SIGILL</a></span> or <span class="t-lc"><a href="/w/c/program/SIG_types" title="c/program/SIG types">SIGSEGV</a></span>, the behavior is undefined.
</p><p>If the signal handler is called as a result of <span class="t-lc"><a href="/w/c/program/abort" title="c/program/abort">abort</a></span> or <span class="t-lc"><a href="/w/c/program/raise" title="c/program/raise">raise</a></span>, the behavior is undefined if the signal handler calls <span class="t-lc"><a href="/w/c/program/raise" title="c/program/raise">raise</a></span>.
</p><p>If the signal handler is called NOT as a result of <span class="t-lc"><a href="/w/c/program/abort" title="c/program/abort">abort</a></span> or <span class="t-lc"><a href="/w/c/program/raise" title="c/program/raise">raise</a></span> (in other words, the signal handler is <i>asynchronous</i>), the behavior is undefined if
</p>
<ul><li> the signal handler calls any function within the standard library, except
</li></ul>
<dl><dd><ul><li> <span class="t-lc"><a href="/w/c/program/abort" title="c/program/abort">abort</a></span>
</li><li> <span class="t-lc"><a href="/w/c/program/_Exit" title="c/program/ Exit">_Exit</a></span>
</li><li> <span class="t-lc"><a href="/w/c/program/quick_exit" title="c/program/quick exit">quick_exit</a></span>
</li><li> <code>signal</code> with the first argument being the number of the signal currently handled (async handler can re-register itself, but not other signals). 
</li><li> atomic functions from <a href="/w/c/thread#Atomic_operations" title="c/thread"><code>&lt;stdatomic.h&gt;</code></a> if the atomic arguments are lock-free
</li><li> <span class="t-lc"><a href="/w/c/atomic/atomic_is_lock_free" title="c/atomic/atomic is lock free">atomic_is_lock_free</a></span> (with any kind of atomic arguments)
</li></ul>
</dd></dl>
<ul><li> the signal handler refers to any object with static <span class="t-rev-inl t-since-c11"><span>or thread-local</span><span><span class="t-mark-rev t-since-c11">(since C11)</span></span></span> <a href="/w/c/language/storage_duration" title="c/language/storage duration">storage duration</a> <span class="t-rev-inl t-since-c11"><span>that is not a lock-free <a href="/w/c/language/atomic" title="c/language/atomic">atomic</a></span><span><span class="t-mark-rev t-since-c11">(since C11)</span></span></span> other than by assigning to a static <span class="t-c"><span class="mw-geshi c source-c"><span class="kw4">volatile</span> <a href="http://en.cppreference.com/w/c/program/sig_atomic_t"><span class="kw498">sig_atomic_t</span></a></span></span>.
</li></ul>
<p>On entry to the signal handler, the state of the floating-point environment and the values of all objects is unspecified, except for
</p>
<ul><li> objects of type <span class="t-c"><span class="mw-geshi c source-c"><span class="kw4">volatile</span> <a href="http://en.cppreference.com/w/c/program/sig_atomic_t"><span class="kw498">sig_atomic_t</span></a></span></span>
</li><li> objects of lock-free atomic types <span class="t-mark-rev t-since-c11">(since C11)</span>
</li><li> side effects made visible through <span class="t-lc"><a href="/w/c/atomic/atomic_signal_fence" title="c/atomic/atomic signal fence">atomic_signal_fence</a></span> <span class="t-mark-rev t-since-c11">(since C11)</span>
</li></ul>
<p>On return from a signal handler, the value of any object modified by the signal handler that is not <span class="t-c"><span class="mw-geshi c source-c"><span class="kw4">volatile</span> <a href="http://en.cppreference.com/w/c/program/sig_atomic_t"><span class="kw498">sig_atomic_t</span></a></span></span> or lock-free atomic<span class="t-mark-rev t-since-c11">(since C11)</span> is undefined.
</p><p>The behavior is undefined if <span class="t-c"><span class="mw-geshi c source-c">signal</span></span> is used in a multithreaded program. It is not required to be thread-safe.
</p>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=4" title="Edit section: Notes">edit</a>]</span> <span class="mw-headline" id="Notes">Notes</span></h3>
<p>POSIX requires that <code>signal</code> is thread-safe, and <a rel="nofollow" class="external text" href="http://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_04">specifies a list of async-signal-safe library functions</a> that may be called from any signal handler.
</p><p>Besides <code>abort</code> and <code>raise</code>, POSIX specifies that <code>kill</code>, <code>pthread_kill</code>, and <code>sigqueue</code> generate synchronous signals.
</p><p>POSIX recommends <a rel="nofollow" class="external text" href="http://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html"><code>sigaction</code></a> instead of <code>signal</code>, due to its underspecified behavior and significant implementation variations, regarding signal delivery while a signal handler is executed.
</p>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=5" title="Edit section: Example">edit</a>]</span> <span class="mw-headline" id="Example">Example</span></h3>
<div class="t-example"><div class="t-example-live-link"><div class="coliru-btn coliru-btn-run-init">Run this code</div></div>
<div dir="ltr" class="mw-geshi t-example-code" style="text-align: left;"><div class="c source-c"><pre class="de1"><span class="co2">#include &lt;signal.h&gt;</span>
<span class="co2">#include &lt;stdio.h&gt;</span>
&nbsp;
<span class="kw4">volatile</span> <a href="http://en.cppreference.com/w/c/program/sig_atomic_t"><span class="kw498">sig_atomic_t</span></a> gSignalStatus<span class="sy4">;</span>
&nbsp;
<span class="kw4">void</span> signal_handler<span class="br0">(</span><span class="kw4">int</span> signal<span class="br0">)</span>
<span class="br0">{</span>
  gSignalStatus <span class="sy1">=</span> signal<span class="sy4">;</span>
<span class="br0">}</span>
&nbsp;
<span class="kw4">int</span> main<span class="br0">(</span><span class="kw4">void</span><span class="br0">)</span>
<span class="br0">{</span>
  signal<span class="br0">(</span><a href="http://en.cppreference.com/w/c/program/SIG_types"><span class="kw505">SIGINT</span></a>, signal_handler<span class="br0">)</span><span class="sy4">;</span>
&nbsp;
  <a href="http://en.cppreference.com/w/c/io/fprintf"><span class="kw850">printf</span></a><span class="br0">(</span><span class="st0">"SignalValue:&nbsp;%d<span class="es1">\n</span>"</span>, gSignalStatus<span class="br0">)</span><span class="sy4">;</span>
  <a href="http://en.cppreference.com/w/c/io/fprintf"><span class="kw850">printf</span></a><span class="br0">(</span><span class="st0">"Sending signal:&nbsp;%d<span class="es1">\n</span>"</span>, <a href="http://en.cppreference.com/w/c/program/SIG_types"><span class="kw505">SIGINT</span></a><span class="br0">)</span><span class="sy4">;</span>
  <a href="http://en.cppreference.com/w/c/program/raise"><span class="kw497">raise</span></a><span class="br0">(</span><a href="http://en.cppreference.com/w/c/program/SIG_types"><span class="kw505">SIGINT</span></a><span class="br0">)</span><span class="sy4">;</span>
  <a href="http://en.cppreference.com/w/c/io/fprintf"><span class="kw850">printf</span></a><span class="br0">(</span><span class="st0">"SignalValue:&nbsp;%d<span class="es1">\n</span>"</span>, gSignalStatus<span class="br0">)</span><span class="sy4">;</span>
<span class="br0">}</span></pre></div></div>
<p>Output:
</p>
<div dir="ltr" class="mw-geshi" style="text-align: left;"><div class="text source-text"><pre class="de1">SignalValue: 0
Sending signal: 2
SignalValue: 2</pre></div></div> 
</div>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=6" title="Edit section: References">edit</a>]</span> <span class="mw-headline" id="References">References</span></h3>
<div class="t-ref-std-17">
<ul><li> C17 standard (ISO/IEC 9899:2018): 
</li></ul>
<dl><dd><ul><li> 7.14.1.1 The signal function (p: 193-194)
</li></ul>
</dd></dl>
</div>
<div class="t-ref-std-11">
<ul><li> C11 standard (ISO/IEC 9899:2011): 
</li></ul>
<dl><dd><ul><li> 7.14.1.1 The signal function (p: 266-267)
</li></ul>
</dd></dl>
</div>
<div class="t-ref-std-c99">
<ul><li> C99 standard (ISO/IEC 9899:1999): 
</li></ul>
<dl><dd><ul><li> 7.14.1.1 The signal function (p: 247-248)
</li></ul>
</dd></dl>
</div>
<div class="t-ref-std-c89">
<ul><li> C89/C90 standard (ISO/IEC 9899:1990): 
</li></ul>
<dl><dd><ul><li> 4.7.1.1 The signal function 
</li></ul>
</dd></dl>
</div>
<h3><span class="editsection">[<a href="/mwiki/index.php?title=c/program/signal&amp;action=edit&amp;section=7" title="Edit section: See also">edit</a>]</span> <span class="mw-headline" id="See_also">See also</span></h3>
<table class="t-dsc-begin">

<tbody><tr class="t-dsc">
<td>  <div class="t-dsc-member-div"><div><a href="/w/c/program/raise" title="c/program/raise"> <span class="t-lines"><span>raise</span></span></a></div></div>
</td>
<td>   runs the signal handler for particular signal  <br> <span class="t-mark">(function)</span> <span class="editsection noprint plainlinks" title="Edit this template"><a rel="nofollow" class="external text" href="https://en.cppreference.com/mwiki/index.php?title=Template:c/program/dsc_raise&amp;action=edit">[edit]</a></span>
</td></tr>

<tr class="t-dsc">
<td colspan="2"> <div class="t-dsc-see"><span><a href="/w/cpp/utility/program/signal" title="cpp/utility/program/signal">C++ documentation</a></span> for <span class="t-dsc-see-tt"><span>signal</span></span></div>
</td></tr>

</tbody></table>

<!-- 
NewPP limit report
Preprocessor visited node count: 3062/1000000
Preprocessor generated node count: 8617/1000000
Post‐expand include size: 69521/2097152 bytes
Template argument size: 11312/2097152 bytes
Highest expansion depth: 21/40
Expensive parser function count: 0/100
-->

<!-- Saved in parser cache with key mwiki1-mwiki_en_:pcache:idhash:6183-0!*!0!!en!*!* and timestamp 20231123112246 -->
</div>                    <!-- /bodycontent -->
                                        <!-- printfooter -->
                    <div class="printfooter">
                    Retrieved from "<a href="https://en.cppreference.com/mwiki/index.php?title=c/program/signal&amp;oldid=140330">https://en.cppreference.com/mwiki/index.php?title=c/program/signal&amp;oldid=140330</a>"                    </div>
                    <!-- /printfooter -->
                                                            <!-- catlinks -->
                    <div id="catlinks" class="catlinks catlinks-allhidden"></div>                    <!-- /catlinks -->
                                                            <div class="visualClear"></div>
                    <!-- debughtml -->
                                        <!-- /debughtml -->
                </div>
                <!-- /bodyContent -->
            

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
