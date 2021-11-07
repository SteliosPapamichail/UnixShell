## Unix shell built by Stelios Papamichail as part of an assignment for the course of CS-345 Operating Systems at the University Of Crete.

-=-=- Assignment Report -=-=-

1) Command parsing logic:

Whenever a line is entered in the shell, the parseInput() function is called. This function will check the type of command(s) entered
(i.e. single command, multiple commands, pipes, redirection) and will then call the appropriate
functions for their execution. In every case, the parseCommandWords() function is called
which simply breaks down the given command into words/args. In cases where we have multiple
commands (i.e. pipes, multi-command input,etc.), the input is first broken down into commands
and then parseCommandWords() is called.

2) Single command execution:

When a single command is given as input to the shell, I first check if it's one of the
commands that can be executed without the need for system calls via the handleCustomCommands()
function. These commands include the cd and exit commands. The cd is executed using chdir()
while exit is executed using... well exit().

3) Multi-command execution:

Once multiple commands have been identified (through ';'), the initial input is first
broken down into the individual commands, and then each command is broken down into its
command words. Once all of the above are ready, execSystemCommand() is called for each
one of those commands. This function uses fork() to create a child process that will execute the given command while the
parent will wait() for it to finish.

4) Piped commands:

In this case, the input is once again split into commands and then into command words just
like above. Once finished, execPipedCommands() is called which first counts the number of
pipes given, and then creates the appropriate pipes using pipe() in order to get the required
file descriptors for each one so that interprocess communication is established. Once finished,
fork() is called for each command and then depending on whether or not the current command
is the first or last one, dup2() is used to copy the parent's file descriptors and redirect I/O,
then the inherited file descriptors are closed and the command is executed using execvp().
When the parent regains control again, it closes its own file descriptors and calls wait() for
each child process.

5) Redirection:

When a redirection char is identified (<,> or >>), the parseInput() function first retrieves
the entered delimeter and then splits the input based on that delimeter using splitAtRedirectionDelim().
This function uses strtok() to split the given input based on the delimeter since strsep() 
accepts only char delimeters but in this case, we could have a string (">>") delimeter.
Once the two parts of the redirection command have been split, execRedirectionCommands() is 
called. This function checks the value of the delimeter first, if it contains the '>', then
we know that the command to the left of the '>' will need to redirect its output to the input
of the command on the right of the '>' delimeter. So, it splits the left hand-side command into
its parts/words, then depending on whether the delimeter equals '>' or ">>", open() is used
to get the required file descriptor. In the first case, since we want to overwrite the previous
contents of the file we are writing to, we add the extra O_TRUNC flag. In the latter case,
we use the O_APPEND in order to append to it. Once the file descriptor is created, fork() is
used to create the child process that will execute the command using execvp(). Before executing,
I use dup2() to redirect the output to STD_IN and then close the initial file descriptor.

The logic is similar for the '<' case, but in that case, the file on the right of the delimeter
will be passed as input to the command on the left. This means that the function opens the
file on the right for reading, uses dup2() to redirect the output, closes the initial file descriptor
and then executes the command.

6) Final thoughts, observations & potential improvements:

The code could definitely use some cleaning up (didn't have spare time) and it definitely could
be split up into smaller .c source files to improve clarity and maintainability. Support for
multiple redirection commands could also be added, as well as a command history using a simple buffer.

I would also like to point out, that running the executable using valgrind, upon exiting you
can see that the only open file descriptors are the three standard ones (input,output and error)
which means that all fds are properly closed. There are also no memory leaks (to my knowledge after testing)
apart from the argv array that is passed to all execvp() commands since when it's successful,
the process is replaced and there is no point in freeing that memory before hand (from why I read).
If it fails, I've made sure to free that memory :).

-=-=- End of Assignment Report -=-=-
