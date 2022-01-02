## gitignore
`bin` matches any files or directories named 'bin'.

`bin/` matches any directories named 'bin', which in effect means all of its contents since Git doesn't track directories alone.

`bin/*` matches all files and directories directly in any bin/. This prevents Git automatically finding any files in its subdirectories, but if, say a bin/foo subdirectory is created, this rule will not match foo's contents.

`bin/**` matches all files and directories in any bin/ directory and all of its subdirectories.


not ignore 

```.gitignore
# Ignore everything
*

# But not these files...
!.gitignore
!script.pl
!template.latex
# etc...

# ...even if they are in subdirectories
!*/

# if the files to be tracked are in subdirectories
!*/a/b/file1.txt
!*/a/b/c/*

```