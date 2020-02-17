# C-shell
A C based shell for Linux. Written as part of EECS 3221 (Operating Systems) coursework.

## Functionalities 

* **cd** - Change directory

* **pwd** - Current working directory

* **mkdir** - Make a directory

* **rmdir** - Remove a directory

* **ls** - List contents of directory

* **!!** - Repeat last command

* Supports background execution such as `estudio &`

* Input from and output to files; `ls > out.txt` or `sort < in.txt`

* Support piping up to 1 level; `sort | ls`

* **exit** - Exit the shell

## Execution
* `make` - Creates ./run file that will run automatically

* `make clean` - Removes the ./run file
